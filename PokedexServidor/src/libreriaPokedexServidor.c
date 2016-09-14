/*
 * libreriaPokedexServidor.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaPokedexServidor.h"

#define MAX_LEN 128


////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimirGiladas(void *unCliente){

	t_infoCliente *infoCliente = (t_infoCliente *) unCliente;
	char paquete[1024];
	int status = 1;

	void enviarAvisoDeCierre(){
		printf("\n");
		enviarHeader(infoCliente->socket, 9);
		printf("AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
		exit(0);
	}

	printf("PokeCliente #%d conectado! esperando mensajes... \n", infoCliente->cliente);

	signal(SIGINT, enviarAvisoDeCierre);

	while(status !=0){
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el PokeCliente #%d dijo: \n %s", infoCliente->cliente, paquete);
			enviarHeader(infoCliente->socket, 1);
			}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void enviarHeader(int unSocket, int unHeader){
	char *recepcion = malloc(sizeof(int));
	memcpy(recepcion, &unHeader, sizeof(int));
	send(unSocket, recepcion, sizeof(int), 0);
	free(recepcion);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimir_archivo(char *rutaDelArchivo){
	int c;
	FILE *file;
	file = fopen(rutaDelArchivo, "r");
	if (file) {
	    while ((c = getc(file)) != EOF)
	        putchar(c);
	    fclose(file);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *txtAString(char *rutaDelArchivo) {
	char * buffer = 0;
	long length;
	FILE * f = fopen(rutaDelArchivo, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}
	return buffer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
 int unCliente = *((int *) numeroCliente);
		char paquete[1024];
		int status = 1;

		void enviarAvisoDeCierre(){
			printf("\n");
			enviarHeader(clientesActivos[unCliente].socket, 9);
			printf("AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
			exit(0);
		}

		printf("PokeCliente #%d conectado! esperando mensajes... \n",
				clientesActivos[unCliente].cliente);

		signal(SIGINT, enviarAvisoDeCierre);

		while(status !=0){
			status = recv(clientesActivos[unCliente].socket, (void*) paquete, 1024, 0);
			if (status != 0) {
				printf("el PokeCliente #%d dijo: \n %s", clientesActivos[unCliente].cliente, paquete);
				enviarHeader(clientesActivos[unCliente].socket, 1);
				}

		}

}
