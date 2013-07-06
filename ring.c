#include "ring.h"

static int abre_socket_servidor(struct sockaddr_in *end_servidor)
{
	int sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0) {
		perror("erro ao abrir socket servidor\n");
		exit(1);
	}
	memset(end_servidor, 0, sizeof(struct sockaddr_in));
	end_servidor->sin_family = AF_INET;
	end_servidor->sin_port = htons(PORTA);
	end_servidor->sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sok, (struct sockaddr *)serveraddr, sizeof(struct sockaddr_in)) < 0) {
		perror("erro no bind\n");
		exit(1);
	}

	return sok;
}

static int abre_socket_cliente(struct sockaddr_in *end_cliente, char *destino)
{
	int sok = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok < 0) {
		perror("erro ao abrir socket cliente\n");
		exit(1);
	}
	memset(end_cliente, 0, sizeof(struct sockaddr_in));
	end_cliente->sin_family = AF_INET;
	end_cliente->sin_port = htons(PORT);
	end_cliente->sin_addr.s_addr = htonl(INADDR_ANY);
	struct hostent *hostp = gethostbyname(destino);
	memcpy(end_cliente->sin_addr, hostp->h_addr_list[0],
	       sizeof(end_cliente->sin_addr));

	return sok;
}

