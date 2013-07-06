#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>



#define PORTA 3432

struct sockaddr_in end_servidor, end_cliente;
int socket_servidor, socket_cliente;

void abre_sockets(void);
static int abre_socket_servidor(void);
static int abre_socket_cliente(void);
static unsigned char paridade(struct s_pacote pacote);
