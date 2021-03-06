#include "ring.h"

void abre_sockets(void)
{
	FILE *file;
	int i;

	TAILQ_INIT(&my_tailq_head);
	pthread_mutex_init(&mutex, NULL);
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
	socket_servidor = abre_socket_servidor(&end_servidor);
	socket_cliente = abre_socket_cliente(&end_cliente, contatos[inext]);
	for (i = 0; i < N_CONTATOS; i++)
		seq[i] = 0;
	if (ihost == 0) {
		comeca_com_bastao();
	} else {
		comeca_sem_bastao();
	}
}

int abre_socket_servidor(struct sockaddr_in *end_servidor)
{
	int sok;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0)
		erro("erro ao abrir socket servidor");
	memset(end_servidor, 0, sizeof(struct sockaddr_in));
	end_servidor->sin_family = AF_INET;
	end_servidor->sin_port = htons(PORTA);
	end_servidor->sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind
	    (sok, (struct sockaddr *)end_servidor, sizeof(struct sockaddr_in))
	    < 0)
		erro("erro no bind");

	return sok;
}

int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino)
{
	int sok;
	struct hostent *hostp;

	sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0)
		erro("erro ao abrir socket cliente");
	memset(end_cliente, 0, sizeof(struct sockaddr_in));
	end_cliente->sin_family = AF_INET;
	end_cliente->sin_port = htons(PORTA);
	end_cliente->sin_addr.s_addr = htonl(INADDR_ANY);
	hostp = gethostbyname(destino);
	memcpy(&(end_cliente->sin_addr), hostp->h_addr_list[0],
	       sizeof(end_cliente->sin_addr));

	return sok;
}

void comeca_com_bastao(void)
{
	if (mandar(TODOS, "pedindo bastao", PEDE_BASTAO)) {
		printf("\t\tanel fechado\n");
		recebe_bastao();
	} else
		comeca_sem_bastao();
}

void comeca_sem_bastao(void)
{
	struct s_pacote pacote;
	char resposta, *privado;
	int timeout;

	timeout = TIMEOUT_BASTAO;
	memset(&pacote, 0, sizeof(struct s_pacote));
	/*ASSIM O TIMEOUT DE ESPERA PELO BASTÃO É EVITADO COM O RECEBIMENTO DE 
	  QUALQUER PACOTE, NÃO SÓ DO PACOTE PASSANDO O BASTÃO.
	  O CERTO SERIA USAR RECEBER_TIMEOUT DENTRO DUM LOOP CONTANDO O TEMPO PASSADO 
	  E A CADA NOVA ITERAÇÃO DIMINUIR O TEMPO RESTANTE DE TIMEOUT
	  ATÉ QUE O BASTÃO SEJA RECEBIDO*/
	while (receber_timeout(&pacote, timeout)) {
		resposta = pacote.tipo;
		if (resposta == BASTAO)
			break;
		if (resposta == PRINT && pacote.seq != seq[(int)pacote.origem] - 1) {
			if (pacote.destino == ihost || pacote.destino == TODOS) {
				privado = pacote.destino == ihost ? "(privado)" : "\0";
				if (strstr(pacote.mensagem, "wait"))
					if (pacote.destino == ihost || strstr(pacote.mensagem, hostname))
						sleep(2);
				printf("<%s>%s: <%s>\n",
				       contatos[(int)pacote.origem], privado,
				       pacote.mensagem);
			}
			seq[(int)pacote.origem]++;
		}
		passar(&pacote);
		memset(&pacote, 0, sizeof(struct s_pacote));
	}
	if (resposta == BASTAO)
		recebe_bastao();
	else {
		printf("\t\ttimeout de espera do bastão\n");
		comeca_com_bastao();
	}
}

int mandar(char destino, char *mensagem, char tipo)
{
	int enviados, tentativas;
	struct s_pacote pacote, pacote_resposta;
	char resposta, ok, par;

	memset(&pacote, 0, sizeof(struct s_pacote));
	pacote.tipo = tipo;
	pacote.destino = destino;
	pacote.origem = ihost;
	if (mensagem)
		strncpy(pacote.mensagem, mensagem, sizeof(pacote.mensagem));
	if (tipo == PRINT) {
		pacote.seq = seq[ihost];
	}
	par = paridade(&pacote);
	pacote.par = par;
	tentativas = 0;
	ok = FALSE;
	while (!ok) {
		tentativas++;
		resposta = 0;
		do {
			enviados =
			    sendto(socket_cliente, &pacote, sizeof(struct s_pacote), 0,
				(struct sockaddr *)&end_cliente, sizeof(struct sockaddr_in));
		} while (enviados <= 0);
		if (tipo == BASTAO)
			return TRUE;
		if (receber_timeout(&pacote_resposta, TIMEOUT_RESPOSTA)) {
			resposta = pacote_resposta.tipo;
			if (resposta == BASTAO)
				return TRUE;
			if (resposta == PEDE_BASTAO && pacote_resposta.origem != ihost)
				return FALSE;
			if (pacote_resposta.origem == ihost && pacote_resposta.seq != seq[ihost] - 1) {
				ok = TRUE;
				if (resposta != tipo || !foi_lido(destino, pacote_resposta.lido) || pacote_resposta.par != par) {
					ok = FALSE;
				}
				if (tipo == PRINT && tentativas >= N_TENTATIVAS) {
					printf("\t\ttentativas > %d, desistindo\n", N_TENTATIVAS);
					ok = TRUE; /*pra evitar possível loop*/
				}
				if (ok && tipo == PRINT)
					seq[ihost]++;
			}
		} else {
			if (tipo != PEDE_BASTAO)
				printf("\t\ttime out de resposta...\n");
		}
	}
	if (tipo == PRINT)
		return tentativas < 5;
	else 
		return TRUE;
}

int foi_lido(char destino, char lido)
{
	if (destino == TODOS)
		return lido == (char)(1 << (N_CONTATOS)) - 1 - (1 << ihost);
	else
		return lido == (char)1 << destino;
}

void receber(struct s_pacote *pacote)
{
	int recebidos, cliente_tam;

	cliente_tam = sizeof(struct sockaddr_in);
	do {
		memset(pacote, 0, sizeof(struct s_pacote));
		recebidos =
		    recvfrom(socket_servidor, pacote, sizeof(struct s_pacote),
			0, (struct sockaddr *)&end_servidor, (socklen_t *) & cliente_tam);
	} while (!recebidos);
}

int receber_timeout(struct s_pacote *pacote, int timeout)
{
	fd_set readfds;
	struct timeval tval;
	int pronto;

	if (timeout < 100) {
		tval.tv_sec = timeout;
		tval.tv_usec = 0;
	} else {
		tval.tv_sec = 0;
		tval.tv_usec = timeout;
	}
	FD_ZERO(&readfds);
	FD_SET(socket_servidor, &readfds);
	pronto = select(socket_servidor + 1, &readfds, NULL, NULL, &tval);
	if (pronto)
		receber(pacote);

	return pronto;
}

void passar(struct s_pacote *pacote)
{
	int enviados;

	if (pacote->destino == ihost || pacote->destino == TODOS)
		pacote->lido |= 1 << ihost;
	do {
		enviados =
		    sendto(socket_cliente, pacote, sizeof(struct s_pacote), 0,
			(struct sockaddr *)&end_cliente, sizeof(struct sockaddr_in));
	} while (enviados <= 0);
}

void recebe_bastao(void)
{
	time_t start;
	int diff;
	struct tailq_entry *item;

	start = time(NULL);
	while ((diff = (int)difftime(time(NULL), start)) < TEMPO_BASTAO) {
		usleep(TEMPO_MINIMO); /*pra não ficar passando o bastão direto e sobrecarregar a rede*/
		pthread_mutex_lock(&mutex);
		if (TAILQ_EMPTY(&my_tailq_head)) {
			pthread_mutex_unlock(&mutex);
			break;	/*com esse break, se a fila está vazia o bastão é passado imediatamente */
		} else {
			item = TAILQ_FIRST(&my_tailq_head);
			TAILQ_REMOVE(&my_tailq_head, item, entries);
			pthread_mutex_unlock(&mutex);
			if (mandar_str(item->destino, item->mensagem)) {
				free(item);
			} else { /*se N_TENTATIVAS não mandaram a mensagem, desiste e a coloca no começo da fila*/
				pthread_mutex_lock(&mutex);
				TAILQ_INSERT_HEAD(&my_tailq_head, item,
						  entries);
				pthread_mutex_unlock(&mutex);
				break;
			}
		}
	}
	mandar(inext, "passando o bastão", BASTAO);
	comeca_sem_bastao();
}

void erro(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

int mandar_str(char destino, char *mensagem)
{
	char buffer[SIZE_MSG], *ptr;
	int i, ok;

	ptr = mensagem;
	ok = TRUE;
	while (*ptr != '\0' && ok) {
		memset(buffer, 0, sizeof(buffer));
		for (i = 0; i < SIZE_MSG - 1; i++) {
			buffer[i] = *ptr++;
			if (*ptr == '\0')
				break;
		}
		if (i < SIZE_MSG - 1)
			i++;
		buffer[i] = '\0';
		ok = mandar(destino, buffer, PRINT);
	}

	return ok;
}

char paridade(struct s_pacote *pacote)
{
	char p, *ptr;
	int size, i;

	p = '\0';
	size = sizeof(struct s_pacote) - 2;	/*menos campos de paridade e lido */
	ptr = (char *)pacote;
	for (i = 0; i < size; i++) {
		p ^= ptr[i];
	}

	return p;
}
