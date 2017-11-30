#define COMMAND_PORT 21
#define DATA_PORT 10000

#define MAX_CONNECTION_QUEUE 5

#define MAX_CWD_SIZE 256
#define BUFFER_SIZE 1024
#define MAX_COMMAND_SIZE 5
#define MAX_REQUEST_ARGS_SIZE 128

#include "ftp_server.h"
#include "lwip/tcp.h"
#include "lwip.h"
#include <ctype.h>
#include "memory_access.h"
#include "lwip/ip_addr.h"
#include "lwip.h"

extern struct netif gnetif;

struct tcp_pcb *command_socket;
struct tcp_pcb *data_socket;

Request *init_Request(uint8_t cmd_len, uint16_t args_len) {
	Request *req = malloc(sizeof(Request));
	req->command = malloc(sizeof(char)*(cmd_len + 1));
	req->args = malloc(sizeof(char)*(args_len + 1));
	return req;
}

void free_Request(Request *req) {
	free(req->command);
	if(req->args != NULL)
		free(req->args);
	free(req);
}

Request *get_request(char *msg) {
	uint8_t i = 0;
	for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
	uint8_t command_start = i;

	for(; !isspace(msg[i]) && msg[i] != '\0'; i++);
	uint8_t command_len = i - command_start + 1;
	if(msg[i] != '\0') {
		msg[i++] = '\0';
	}

	for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
	uint8_t args_start = i;

	for(; msg[i] != '\r' && msg[i] != '\n' && msg[i] != '\0'; i++);
	uint8_t args_len = i - args_start + 1;
	msg[i] = '\0';

	Request *req = init_Request(command_len, args_len);
	strcpy(req->command, &msg[command_start]);
	strcpy(req->args, &msg[args_start]);

	return req;
}

struct tcp_pcb *open_socket(uint16_t port) {
	struct tcp_pcb *pcb = tcp_new();
	if(pcb == NULL) {
		printf("tcp_new error\r\n");
		return NULL;
	}

	if(tcp_bind(pcb, &gnetif.ip_addr, port) != ERR_OK) {
		printf("tcp_bind error\r\n");
		return NULL;
	}

	pcb = tcp_listen(pcb);
	if(pcb == NULL) {
		printf("tcp_listen error");
		return NULL;
	}
	return pcb;
}

void accept_command_conn(struct tcp_pcb *pcb) {
	tcp_accept(pcb, serve_client);
	tcp_accepted(pcb);
}

void accept_data_conn(struct tcp_pcb *pcb, err_t (* callback)(void * arg, struct tcp_pcb * newpcb, err_t err)) {
	tcp_accept(pcb, callback);
	tcp_accepted(pcb);
}

void send_response(uint16_t status, char *text, struct tcp_pcb *pcb) {
	uint16_t len = 3 + 1 + strlen(text) + 3;
	char *buf = malloc(sizeof(char)*len);
	sprintf(buf, "%d %s\r\n", status, text);

	while(tcp_write(pcb, buf, len, TCP_WRITE_FLAG_COPY) == ERR_MEM) {
		printf("tcp_write error, will try again");
		osDelay(100);
	}
	tcp_output(pcb);
	free(buf);
}

err_t serve_client(void *arg, struct tcp_pcb *pcb, err_t err) {
	send_response(220, "Welcome.", pcb);
	printf("client connected\r\n");

	while(1) {
		tcp_recv(pcb, serve_request);
	}

	return ERR_OK;
}

err_t *serve_request(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	Request *req = get_request(p->payload);
	tcp_recved(pcb, p->len);
	pbuf_free(p);

	respond(req, pcb);
	return ERR_OK;
}

void pasv_response(struct tcp_pcb *pcb) {
	char *buf = malloc(sizeof(char)*50);

	sprintf(buf, "Entering Passive Mode (%u,%u,%u,%u,%d,%d)\r\n",
			ip4_addr1(&gnetif.ip_addr),
			ip4_addr2(&gnetif.ip_addr),
			ip4_addr3(&gnetif.ip_addr),
			ip4_addr4(&gnetif.ip_addr),
			DATA_PORT/256, DATA_PORT % 256);

	send_response(227, buf, pcb);
}

err_t send_list_data(void *arg, struct tcp_pcb *pcb, err_t err) {
	char *ls = list_directory();
	while(tcp_write(pcb, ls, strlen(ls), TCP_WRITE_FLAG_COPY) == ERR_MEM) {
		printf("tcp_write error, list directory, will try again");
		osDelay(100);
	}
	free(ls);
	tcp_output(pcb);
	return ERR_OK;
}

void list_response(struct tcp_pcb *pcb) {
	send_response(150, "Directory listing.", pcb);
	accept_data_conn(data_socket, send_list_data);
	send_response(226, "Directory send OK.", pcb);
}

void respond(Request *req, struct tcp_pcb *pcb) {
	char *command = req->command;

	if(strcmp(command, "USER") == 0) {
		send_response(230, "Success.", pcb);
	}
	else if(strcmp(command, "SYST") == 0) {
		send_response(215, "EMBOS", pcb);
	}
	else if(strcmp(command, "PWD") == 0) {
		send_response(257, get_current_directory(), pcb);
	}
	else if((strcmp(command, "TYPE") == 0))	{
		if(strcmp(req->args, "I") == 0)
			send_response(200, "Binary mode accepted.", pcb);
		else
			send_response(502, "Command not implemented.", pcb);
	}
	else if(strcmp(command, "PASV") == 0) {
		pasv_response(pcb);
	}
	else if(strcmp(command, "LIST") == 0) {
		list_response(pcb);
	}
	else if(strcmp(command, "CWD") == 0) {
		change_directory(req->args);
		send_response(250, "OK.", pcb);
	}
	else {
		send_response(502, "Command not implemented.", pcb);
	}
}

void start_server() {
	printf("IP: %u.%u.%u.%u\r\nPORT: %d\r\n",
				ip4_addr1(&gnetif.ip_addr),
				ip4_addr2(&gnetif.ip_addr),
				ip4_addr3(&gnetif.ip_addr),
				ip4_addr4(&gnetif.ip_addr),
				DATA_PORT);

	command_socket = open_socket(COMMAND_PORT);
	data_socket = open_socket(DATA_PORT);

	accept_command_conn(command_socket);
}

