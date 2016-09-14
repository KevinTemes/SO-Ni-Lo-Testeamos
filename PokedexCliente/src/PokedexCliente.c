/*
 ============================================================================
 Name        : PokedexCliente.c
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

/* defines para testear sockets */
#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024

int main() {

int servidor;
servidor = conectarCliente(IP, PUERTO);

/* gilada para el primer checkpoint */
int enviar = 1;
char message[PACKAGESIZE];
char *resultado = malloc(sizeof(int));
int resultadoEnvio = 0;
printf("Conectado al servidor. Bienvenido a la enciclopedia global Pokemon! ingrese el mensaje que desee enviar, o cerrar para salir\n");

while(enviar != 0){
	fgets(message, PACKAGESIZE, stdin);
	if (!strcmp(message,"cerrar\n")) enviar = 0;
	send(servidor, message, strlen(message) + 1, 0);
	recv(servidor, (void *)resultado, sizeof(int), 0);
	resultadoEnvio = *((int *)resultado);

	if(resultadoEnvio == 1) {
		printf("el servidor recibió el mensaje!: %d\n", resultadoEnvio);
		}
	else if(resultadoEnvio == 9){
		printf("Servidor caído! imposible reconectar. Cerrando...\n");
		exit(0);
		}
	}




/* fin gilada para el primer checkpoint */

close(servidor);

return 0;


}
