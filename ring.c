#include "ring.h"

void abre_sockets(void)
{
	FILE *file;
	int i;
	//struct hostent hostent_local;

	file = fopen(CONTATOS, "r");
	if (!file) {
		perror("erro ao abrir CONTATOS\n");
		exit(1);
	}
	gethostname(hostname, 15);
	i = 0;
	while (fscanf(file, "%s", contatos[i]) != EOF) {
		if (!strcmp(contatos[i], hostname)) {
			ihost = i;
		}
		i++;
	}
	if (i > 4) {
		perror("erro ao ler CONTATOS\n");
		exit(1);
	}
	inext = (ihost + 1) % N_CONTATOS;
	printf("host: %s\nnext: %s\n", contatos[ihost], contatos[inext]);
	socket_servidor = abre_socket_servidor(&end_servidor);
	socket_cliente = abre_socket_cliente(&end_cliente, contatos[inext]);
	if (ihost == 0) {
		comeca_com_bastao();	
	} else {
		comeca_sem_bastao();
	}
}

static int abre_socket_servidor(struct sockaddr_in *end_servidor)
{
	int sok;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0) {
		perror("erro ao abrir socket servidor\n");
		exit(1);
	}
	memset(end_servidor, 0, sizeof(struct sockaddr_in));
	end_servidor->sin_family = AF_INET;
	end_servidor->sin_port = htons(PORTA);
	end_servidor->sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sok, (struct sockaddr *)end_servidor, sizeof(struct sockaddr_in))
	    < 0) {
		perror("erro no bind\n");
		exit(1);
	}

	return sok;
}

static int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino)
{
	int sok;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0) {
		perror("erro ao abrir socket cliente\n");
		exit(1);
	}
	memset(end_cliente, 0, sizeof(struct sockaddr_in));
	end_cliente->sin_family = AF_INET;
	end_cliente->sin_port = htons(PORTA);
	end_cliente->sin_addr.s_addr = htonl(INADDR_ANY);
	struct hostent *hostp = gethostbyname(destino);
	memcpy(&(end_cliente->sin_addr), hostp->h_addr_list[0],
	       sizeof(end_cliente->sin_addr));

	return sok;
}

static void comeca_com_bastao(void)
{
	struct s_pacote pacote;
	char resposta;

	tenho_bastao = TRUE;
	printf("começando com o bastão!\n");
	mandar("todos", NULL, PEDE_BASTAO);
}

static void comeca_sem_bastao(void)
{
	struct s_pacote pacote;
	char resposta;

	tenho_bastao = FALSE;
	printf("esperando pelo bastão...\n");
	do {
		resposta = receber(&pacote);
		passar(&pacote);
	} while (resposta != BASTAO);
	printf("bastão recebido!\n");
	tenho_bastao = TRUE;
	recebe_bastao();
}

int mandar(char *destino, char *mensagem, char tipo)
{
	int enviados;
	struct s_pacote pacote, pacote_resposta;
	char resposta, pronto;
	fd_set readfds;
	struct timeval tval;

	tval.tv_sec = TIMEOUT_RESPOSTA;
	tval.tv_usec = 0;
	memset(&pacote, 0, sizeof(pacote));
	pacote.tipo = tipo;
	strcpy(pacote.destino, destino);
	strcpy(pacote.origem, hostname);
	if (mensagem)
		strcpy(pacote.mensagem, mensagem);
	resposta = 0;
	while (!resposta) {
		do {
			enviados = sendto(socket_cliente, &pacote, sizeof(struct s_pacote), 0, (struct sockaddr*)&end_cliente, sizeof(end_cliente));
		} while (enviados <= 0);
		printf("pacote enviado\n");
		FD_ZERO(&readfds);
		FD_SET(socket_servidor, &readfds);
		pronto = select(socket_servidor + 1, &readfds, NULL, NULL, &tval);
		if (pronto) {
			resposta = receber(&pacote_resposta);
			printf("resposta recebida!\n");
		} else {
			printf("time out de resposta...\n");
		}
	}
}

char receber(struct s_pacote *pacote)
{
	int recebidos, cliente_tam;
	char tipo;

	cliente_tam = sizeof(struct s_pacote);
	do {
		memset(pacote, 0, cliente_tam);
		recebidos = recvfrom(socket_servidor, pacote, cliente_tam, 0, (struct sockaddr*)&end_servidor, (socklen_t*)&cliente_tam);
	} while (!recebidos);
	printf("pacote recebido\n");

	return pacote->tipo;
}

void passar(struct s_pacote *pacote)
{
	int enviados;

	pacote->lido |= 1 << ihost;
	do {
		enviados = sendto(socket_cliente, pacote, sizeof(struct s_pacote), 0, (struct sockaddr*)&end_cliente, sizeof(end_cliente));
	} while (enviados <= 0);
	printf("pacote passado pra frente\n");
}

void recebe_bastao(void)
{
}
