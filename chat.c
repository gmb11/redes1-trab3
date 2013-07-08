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
}

void *chat(void *p)
{
	char buffer[SIZE_BUFFER], *ptr, *ptr2;
	struct tailq_entry *item;

	while (1) {
		item = malloc(sizeof(*item));
		if (!item)
			erro("erro no malloc");
		fgets(buffer, sizeof(buffer), stdin);
		ptr = buffer;
		ptr2 = item->destino;
		if (ptr[0] == '/') {
			ptr++;
			while ((*ptr2++ = *ptr++) != ' ');
			*--ptr2 = '\0';
		} else {
			strcpy(item->destino, "todos");
		}
		ptr2 = strchr(ptr, '\n');
		if (ptr2) 
			*ptr2 = '\0';
		strcpy(item->mensagem, ptr);
		pthread_mutex_lock(&mutex);
		//printf("enfilando mensagem <%s> pra <%s>\n", item->mensagem, item->destino);
		TAILQ_INSERT_TAIL(&my_tailq_head, item, entries);
		pthread_mutex_unlock(&mutex);
	}
}

void listar_contatos(void)
{
	int i;

	printf("contatos:\n");
	for (i = 0; i < N_CONTATOS; i++) {
		printf(">> %s\n", contatos[i]);
	}
}
