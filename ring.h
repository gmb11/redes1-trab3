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
#define N_TENTATIVAS 5
#define SIZE_BUFFER 256
#define SIZE_MSG 30
#define TODOS 99

#define TIMEOUT_RESPOSTA 500000
#define TEMPO_BASTAO 1
#define TIMEOUT_BASTAO TEMPO_BASTAO * N_CONTATOS + 1
#define TEMPO_MINIMO 100000

/*tipos de mensagem*/
#define BASTAO 1
#define PEDE_BASTAO 2
#define PRINT 3

struct s_pacote {
	char tipo;
	char origem;
	char destino;
	char mensagem[SIZE_MSG];
	char seq;
	char par;
	char lido;
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
unsigned char seq[4];

void abre_sockets(void);
int abre_socket_servidor(struct sockaddr_in *end_servidor);
int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino);
void comeca_com_bastao(void);
void comeca_sem_bastao(void);
void receber(struct s_pacote *pacote);
int receber_timeout(struct s_pacote *pacote, int timeout);
void passar(struct s_pacote *pacote);
void recebe_bastao(void);
void erro(char *msg);
int mandar(char destino, char *mensagem, char tipo);
int mandar_str(char destino, char *mensagem);
int foi_lido(char destino, char lido);
char paridade(struct s_pacote *pacote);
