/*
 ============================================================================
 Name        : PokedexServidor.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "libSockets.h"

/* para testear sockets */
#define PACKAGESIZE 1024
#define PUERTO "7777"

int main() {

int socketEscucha;
socketEscucha = setup_listen("localhost", PUERTO);

listen(socketEscucha, 1024);

struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);
int socketCliente = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);


/* Esto de ahora es gilada para el primer checkpoint, eliminar después */

char paquete[PACKAGESIZE];
int status = 1;
printf("PokeCliente conectado! Esperando mensajes...\n");

	while (status != 0){
		status = recv(socketCliente, (void*) paquete, PACKAGESIZE, 0);
		if (status != 0) printf("%s", paquete);

	}

/* Fin de la gilada para el primer checkpoint */

close(socketCliente);
close(socketEscucha);

return 0;

}
