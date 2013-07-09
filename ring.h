#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/queue.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0
#define PORTA 3432
#define CONTATOS "contatos.txt"
#define N_CONTATOS 4
#define SIZE_BUFFER 256
#define SIZE_MSG 30
#define TODOS 99

#define TIMEOUT_RESPOSTA 1
#define TEMPO_BASTAO 2

/*tipos de mensagem*/
#define BASTAO 1
#define PEDE_BASTAO 2
#define PRINT 3
#define SLEEP 4 //pra forçar timeout

struct s_pacote {
	char tipo;
	char lido;
	char origem;
	char destino;
	char mensagem[SIZE_MSG];
	char par;
	char seq;
};

struct tailq_entry {
	char destino, mensagem[SIZE_BUFFER];
	TAILQ_ENTRY(tailq_entry) entries;
};
TAILQ_HEAD(, tailq_entry) my_tailq_head;
pthread_mutex_t mutex;

char contatos[4][15], hostname[15];
struct sockaddr_in end_servidor, end_cliente;
int socket_servidor, socket_cliente, inext, ihost;

void abre_sockets(void);
int abre_socket_servidor(struct sockaddr_in *end_servidor);
int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino);
char paridade(struct s_pacote pacote);
void comeca_com_bastao(void);
void comeca_sem_bastao(void);
char receber(struct s_pacote *pacote);
void passar(struct s_pacote *pacote);
void recebe_bastao(void);
void erro(char *msg);
void mandar(char destino, char *mensagem, char tipo);
void mandar_str(char destino, char *mensagem);
int foi_lido(char destino, char lido);
