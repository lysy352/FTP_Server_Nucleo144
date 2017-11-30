#ifndef FTP_SERVER_H_
#define FTP_SERVER_H_

#include "stdint.h"
#include "lwip.h"

typedef struct {
	char *command;
	char *args;
} Request;


Request *init_Request(uint8_t cmd_len, uint16_t args_len);

void free_Request(Request *req);

void start_server();

Request *get_request(char *msg);

struct tcp_pcb *open_socket(uint16_t port);

void accept_command_conn(struct tcp_pcb *socket);

void accept_data_conn(struct tcp_pcb *socket, err_t (* callback)(void * arg, struct tcp_pcb * newpcb, err_t err));

void send_response(uint16_t status, char *text, struct tcp_pcb *pcb);

err_t serve_client(void *arg, struct tcp_pcb *client_pcb, err_t err);

err_t *serve_request(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

void pasv_response(struct tcp_pcb *pcb);

err_t send_list_data(void *arg, struct tcp_pcb *client_pcb, err_t err);

void list_response(struct tcp_pcb *pcb);

void respond(Request *req, struct tcp_pcb *pcb);



#endif /* FTP_SERVER_H_ */
