#include "ring.h"

void abre_sockets(void)
{
	FILE *file;
	int i;
	//struct hostent hostent_local;

	file = fopen(CONTATOS, "r");
	if (!file)
		erro("erro ao abrir CONTATOS");
	gethostname(hostname, 15);
	i = 0;
	while (fscanf(file, "%s", contatos[i]) != EOF) {
		if (!strcmp(contatos[i], hostname)) {
			ihost = i;
		}
		i++;
	}
	if (i > 4)
		erro("erro ao ler CONTATOS");
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
	if (sok < 0)
		erro("erro ao abrir socket servidor");
	memset(end_servidor, 0, sizeof(struct sockaddr_in));
	end_servidor->sin_family = AF_INET;
	end_servidor->sin_port = htons(PORTA);
	end_servidor->sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sok, (struct sockaddr *)end_servidor, sizeof(struct sockaddr_in))
	    < 0)
		erro("erro no bind");

	return sok;
}

static int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino)
{
	int sok;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0)
		erro("erro ao abrir socket cliente");
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
	//mandar("todos", NULL, PEDE_BASTAO);
	mandar("todos", "pedindo bastao\n", PEDE_BASTAO);
	recebe_bastao();
}

static void comeca_sem_bastao(void)
{
	struct s_pacote pacote;
	char resposta;

	tenho_bastao = FALSE;
	printf("esperando pelo bastão...\n");
	while ((resposta = receber(&pacote)) != BASTAO) {
		if (!strcmp(pacote.destino, hostname) || !strcmp(pacote.destino, "todos")) {
			printf("%s", pacote.mensagem);
		}
		passar(&pacote);
	}
	tenho_bastao = TRUE;
	recebe_bastao();
}

int mandar(char *destino, char *mensagem, char tipo)
{
	int enviados;
	struct s_pacote pacote, pacote_resposta;
	char resposta, pronto, ok;
	fd_set readfds;
	struct timeval tval;

	memset(&pacote, 0, sizeof(pacote));
	pacote.tipo = tipo;
	strcpy(pacote.destino, destino);
	strcpy(pacote.origem, hostname);
	if (mensagem)
		strncpy(pacote.mensagem, mensagem, sizeof(pacote.mensagem));
	resposta = 0;
	ok = FALSE;
	while (!ok) {
		resposta = 0;
		tval.tv_sec = TIMEOUT_RESPOSTA;
		tval.tv_usec = 0;
		do {
			enviados = sendto(socket_cliente, &pacote, sizeof(struct s_pacote), 0, (struct sockaddr*)&end_cliente, sizeof(end_cliente));
		} while (enviados <= 0);
		printf("pacote enviado\n");
		if (tipo == BASTAO)
			return;
		FD_ZERO(&readfds);
		FD_SET(socket_servidor, &readfds);
		pronto = select(socket_servidor + 1, &readfds, NULL, NULL, &tval);
		if (pronto) {
			resposta = receber(&pacote_resposta);
			printf("resposta recebida!\n");
			//conferir pacote_resposta.lido
			ok = TRUE;
		} else {
			printf("time out de resposta...\n");
		}
	}
}

char receber(struct s_pacote *pacote)
{
	int recebidos, cliente_tam;
	char tipo;

	cliente_tam = sizeof(end_servidor);
	do {
		memset(pacote, 0, cliente_tam);
		recebidos = recvfrom(socket_servidor, pacote, sizeof(pacote), 0, (struct sockaddr*)&end_servidor, (socklen_t*)&cliente_tam);
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
	time_t start, stop;
	int diff;
	struct s_pacote pacote;
	
	start = time(NULL);
	printf("bastão recebido!\n");
	while ((diff = (int) difftime(time(NULL), start)) < TEMPO_BASTAO) {
		//leitura não pode ser bloqueante, melhor usar outra thread
		fgets(pacote.mensagem, sizeof(pacote.mensagem), stdin);
		mandar("todos", pacote.mensagem, PRINT);
	}
	printf("tempo do bastão esgotado...\n");
	mandar(contatos[inext], "passando o bastão", BASTAO);
	comeca_sem_bastao();
}

void erro(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}
