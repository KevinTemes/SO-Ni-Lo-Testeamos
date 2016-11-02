/*
 * libreriaMapa.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaMapa.h"
#include <tad_items.h>

#define MAX_LEN 128

pthread_mutex_t mutexPaqueton = PTHREAD_MUTEX_INITIALIZER;

extern char paqueton[10];

int numEntrenador;

t_infoCliente clientesActivos[1024];

extern sem_t sem_Listos;
extern sem_t sem_quantum;

extern char* nombreMapa;

extern t_queue* colaAccion;

extern t_queue* colaListos;

extern t_list* listaDeColasAccion;

extern t_list* items;

/////////////////////////////////////////////////////////////////////////////
void* recibirDatos(int conexion, int tamanio) {
	void* mensaje = (void*) malloc(tamanio);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios = string_new();
		string_append(&adios, "0\0");
		return adios;
	}
	return mensaje;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimirGiladas(void *unCliente) {

	t_infoCliente *infoCliente = (t_infoCliente *) unCliente;
	char paquete[1024];
	int status = 1;

	void enviarAvisoDeCierre() {
		printf("\n");
		enviarHeader(infoCliente->socket, 9);
		printf(
				"AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
		exit(0);
	}

	printf("Entrenador #%d conectado! esperando mensajes... \n",
			infoCliente->cliente);

	signal(SIGINT, enviarAvisoDeCierre);

	while (status != 0) {
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el Entrenador #%d dijo: \n %s", infoCliente->cliente,
					paquete);
			enviarHeader(infoCliente->socket, 1);
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void enviarHeader(int unSocket, int unHeader) {
	char *recepcion = malloc(sizeof(int));
	memcpy(recepcion, &unHeader, sizeof(int));
	send(unSocket, recepcion, sizeof(int), 0);
	free(recepcion);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimir_archivo(char *rutaDelArchivo) {
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
void notificarCaida() {
	int i;
	for (i = 0; i <= 1024; i++) {
		enviarHeader(clientesActivos[i].socket, 9);
	}
	printf("\n");
	printf(
			"AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente) {
	int unCliente = *((int *) numeroCliente);
	char* paquete = malloc(sizeof(char) * 2);
	int status = 1;

	entrenador* ent1 = malloc(sizeof(entrenador));
	ent1->flagEstaEnLista = 0; //al ser nuevo no esta registrado en la lista


	t_log* logs;
	remove("Mapi.log");

	logs = log_create("Mapi.log", "Mapa", false, log_level_from_string("INFO"));

	//printf("Entrenador #%d conectado! esperando mensajes... \n",
	//clientesActivos[unCliente].cliente);
	while (status != 0) {

		// paquete = recibirDatos(clientesActivos[unCliente].socket,2);
		status = recv(clientesActivos[unCliente].socket, (void*) paquete, 2, 0);
		if (status != 0) {
			//printf("el Entrenador #%d dijo: \n %s", clientesActivos[unCliente].cliente, paquete);
			//status = recv(clientesActivos[unCliente].socket, paquete, 2, 0);
			//log_info(logs,"paquete: %c%c",paquete[0],paquete[1]);
			char* cambio = strdup(paquete);
			pthread_mutex_lock(&mutexPaqueton);

			numEntrenador = unCliente;
			log_info(logs, "paquete global:%c %c", cambio[0], cambio[1]);
			log_info(logs, "unCliente:%d clientesactivos[uncliente].cliente:%d",unCliente,clientesActivos[unCliente].cliente);
			//printf("%c",paqueton[0]);
			pthread_mutex_unlock(&mutexPaqueton);

			//enviarHeader(clientesActivos[unCliente].socket, 1);

			log_info(logs, "Este seria un caracter del entrenador: %c",cambio[0]);
			log_info(logs, "Este seria su accionar: %d", cambio[1]);

			//mientras no haya recibido nada



				//paso variable global al entrenador
				ent1->simbolo = cambio[0]; //almaceno simbolo en entrenador (paqueton [0] es variable global
				ent1->accion = cambio[1]; //almaceno que hacer en entrenador paqueton global
				ent1->numeroCliente = numEntrenador; //numero de cliente para envio de informacion

				//si el entrenador no esta registrado
				if (!ent1->flagEstaEnLista) {
					 ent1->numeroLlegada = (clientesActivos[unCliente].cliente-1); //numero del entrenador
					 ent1->flagEstaEnLista = 1; //ahora este entrenador nuevo esta en la lista
					 ent1->posx = 0; //posicion en x inicializada en 0
					 ent1->posy = 0; // idem en y

					queue_push(colaAccion, ent1->accion); // meto la accion del entrenador en la cola

					//list_replace(listaDeColasAccion, ent1->numeroLlegada, colaAccion); // agrego en la lista que contiene su cola de accion

                    list_add(listaDeColasAccion,colaAccion);


					log_info(logs, "llego aca");

					CrearPersonaje(items, ent1->simbolo, ent1->posx, ent1->posy); //mete al pj en el mapa

					nivel_gui_dibujar(items, nombreMapa);

					queue_push(colaListos, (void*) ent1); //llego un entrenador entonces lo meto en la cola de listos

					log_info(logs, "entrenador %c a listos", ent1->simbolo); //informo por archivo de log la llegada del entrenador

					//	aux = '\0';

					sem_post(&sem_Listos); //produce un ent en colaListos
				}

				//si el entrenador se encontraba registrado
				else {
					t_queue* cola;

					cola =  list_get(listaDeColasAccion, ent1->numeroLlegada); //saco la cola y se la meto a la auxiliar
					log_info(logs,"llego a paso el list get joya");

					queue_push(cola, ent1->accion); //pusheo nuevo accionar a la cola auxiliar
					/*int ac;
					ac = queue_pop(cola);
					log_info(logs,"acto es %d", ac); */

					list_replace(listaDeColasAccion, ent1->numeroLlegada, cola); //reemplaza la cola de la lista por la auxiliar

					log_info(logs,"paso el replace");

					sem_post(&sem_quantum);
					//aux = '\0';
				}

				//aux = '\0';
			}

		}

	free(ent1);

free( paquete);

}
