#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define TRUE 1
#define FALSE 0
#define PORTA 3432
#define CONTATOS "contatos.txt"
#define N_CONTATOS 4
#define TIMEOUT_RESPOSTA 3

/*tipos de mensagem*/
#define BASTAO 1
#define PEDE_BASTAO 2

struct s_pacote {
	char tipo;
	char lido;
	char origem[15];
	char destino[15];
	char mensagem[30];
	char par;
	char seq;
};

char contatos[4][15], hostname[15], tenho_bastao;
struct sockaddr_in end_servidor, end_cliente;
int socket_servidor, socket_cliente, inext, ihost;

void abre_sockets(void);
static int abre_socket_servidor(struct sockaddr_in *end_servidor);
static int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino);
static char paridade(struct s_pacote pacote);
static void comeca_com_bastao(void);
static void comeca_sem_bastao(void);
int mandar(char *destino, char *mensagem, char tipo);
char receber(struct s_pacote *pacote);
void passar(struct s_pacote *pacote);
void recebe_bastao(void);
