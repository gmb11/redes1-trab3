#include "chat.h"

void abrir_chat(void)
{
	pthread_t t_chat, t_ring;

	pthread_create(&t_chat, NULL, chat, NULL);
	pthread_create(&t_ring, NULL, ring, NULL);
	pthread_join(t_chat, NULL);
	pthread_join(t_ring, NULL);
}

void *ring(void *p)
{
	abre_sockets();
	return (void *)0x0;
}

void *chat(void *p)
{
	char buffer[SIZE_BUFFER], destino[SIZE_MSG], *ptr, *ptr2;
	struct tailq_entry *item;

	item = NULL;
	while (1) {
		item = item ? item : malloc(sizeof(struct tailq_entry));
		if (!item)
			erro("erro no malloc");
		memset(item, 0, sizeof(struct tailq_entry));
		fgets(buffer, SIZE_BUFFER, stdin);
		ptr = buffer;
		ptr2 = destino;
		if (ptr[0] == '/') {
			ptr++;
			if (!strncmp(ptr, "listar", 6)) {
				listar_contatos();
				continue;
			}
			while (*ptr != '\n' && (*ptr2++ = *ptr++) != ' ') ;
			if (*ptr == '\n')
				continue;
			*--ptr2 = '\0';
			item->destino = (char)contato_id(destino);
			if (item->destino == ihost)
				continue;
			if (item->destino < 0) {
				printf("contato inexistente...\n");
				continue;
			}
		} else {
			item->destino = TODOS;
		}
		ptr2 = strchr(ptr, '\n');
		if (ptr2)
			*ptr2 = '\0';
		strcpy(item->mensagem, ptr);
		pthread_mutex_lock(&mutex);
		TAILQ_INSERT_TAIL(&my_tailq_head, item, entries);
		pthread_mutex_unlock(&mutex);
		item = NULL;
	}
	return (void *)0x0;
}

void listar_contatos(void)
{
	int i;

	printf("contatos:\n");
	for (i = 0; i < N_CONTATOS; i++) {
		printf(">> %s\n", contatos[i]);
	}
}

int contato_id(char *nome)
{
	int i;

	if (!strcmp(nome, "todos"))
		return TODOS;
	for (i = 0; i < N_CONTATOS; i++) {
		if (!strcmp(nome, contatos[i])) {
			return i;
		}
	}

	return -1;
}
