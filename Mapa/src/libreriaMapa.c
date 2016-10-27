/*
 * libreriaPokedexServidor.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaMapa.h"

#define MAX_LEN 128

pthread_mutex_t mutexPaqueton=PTHREAD_MUTEX_INITIALIZER;

extern char paqueton[10];

int numEntrenador;

t_infoCliente clientesActivos[1024];

extern sem_t sem_Nuevos;


/////////////////////////////////////////////////////////////////////////////
void* recibirDatos(int conexion, int tamanio){
	void* mensaje=(void*)malloc(tamanio);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios=string_new();
		string_append(&adios,"0\0");
		return adios;}
	return mensaje;
}


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

	printf("Entrenador #%d conectado! esperando mensajes... \n", infoCliente->cliente);

	signal(SIGINT, enviarAvisoDeCierre);

	while(status !=0){
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el Entrenador #%d dijo: \n %s", infoCliente->cliente, paquete);
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
void notificarCaida(){
	int i;
	for( i = 0; i <= 1024; i++){
		enviarHeader(clientesActivos[i].socket, 9);
	}
	printf("\n");
	printf("AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
	int unCliente = *((int *) numeroCliente);
	char* paquete = malloc(sizeof(char) * 2);
	int status = 1;


	t_log* logs;
	remove("Mapi.log");

		logs = log_create("Mapi.log", "Mapa", false, log_level_from_string("INFO"));




	//printf("Entrenador #%d conectado! esperando mensajes... \n",
				//clientesActivos[unCliente].cliente);
	while(status !=0){

		// paquete = recibirDatos(clientesActivos[unCliente].socket,2);
		status = recv(clientesActivos[unCliente].socket, (void*) paquete, 2, 0);
		if (status != 0) {
			//printf("el Entrenador #%d dijo: \n %s", clientesActivos[unCliente].cliente, paquete);
			//status = recv(clientesActivos[unCliente].socket, paquete, 2, 0);
			//log_info(logs,"paquete: %c%c",paquete[0],paquete[1]);
			char* cambio = strdup(paquete);
			pthread_mutex_lock(&mutexPaqueton);
			paqueton[0] = cambio[0];
			paqueton[1] = cambio[1];
			numEntrenador = unCliente;
			log_info(logs,"paquete global:%c%c",paqueton[0],paqueton[1]);
			//printf("%c",paqueton[0]);
			pthread_mutex_unlock(&mutexPaqueton);
            sem_post(&sem_Nuevos);

			//enviarHeader(clientesActivos[unCliente].socket, 1);
			}
	}
	free (paquete);
}
