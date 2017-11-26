#define SERVER_COMMAND_PORT 21
#define SERVER_DATA_PORT 10000
#define MAX_CONNECTION_QUEUE 5


#define MAX_CWD_SIZE 256
#define BUFFER_SIZE 1024
#define MAX_COMMAND_SIZE 5
#define MAX_REQUEST_DATA_SIZE 128

#include "lwip/tcp.h"


void start_server() {
	struct tcp_pcb *pcb = tcp_new();

	if(tcp_bind(pcb, IP_ADDR_ANY, SERVER_COMMAND_PORT) == NULL) {

	}






}

void new_connection() {
	
}
