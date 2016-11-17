/*
 * libreriaMapa.h
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#ifndef LIBRERIAPOKEDEXSERVIDOR_H_
#define LIBRERIAPOKEDEXSERVIDOR_H_
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "libSockets.h"
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <ctype.h>
#include <pthread.h>
#include <poll.h>
#include <semaphore.h>
#include <signal.h>

typedef struct {
	int cliente;
	int socket;
} t_infoCliente;

typedef struct{
	metaDataPokemon* pokePeleador;
	char pokenestAsignado;
	t_queue* colaAccion;
	t_list* asignados;
	t_list* solicitud;
	t_list* pokemones;
	char simbolo;
	int entroBloqueados;
	int numeroLlegada;
	int numeroCliente;
	int flagEstaEnLista;
	int posx;
	int posy;
	int posPokex;
	int posPokey;
	int flagLeAsignaronPokenest;
	int estaMarcado;
	int fallecio;
} entrenador;


/* Función loca para testear rececpción de mensajes a través de un socket. */
void imprimirGiladas(void *unCliente);

/* Función para imprimir por pantalla el contenido de un archivo de texto */
void imprimir_archivo(char *rutaDelArchivo);

/* Función para volcar el contenido de un archivo .txt dentro de un string */
char *txtAString(char *rutaDelArchivo);

// Funciones para ordenar colas en srdf
int calcularDistancia(entrenador* ent); //calcula la distancia de un entrenador a una pokenest
bool esMasCerca(entrenador* cerca, entrenador* lejos); //compara dos distancias de dos entrenadores a sus respectivas pokenests

/* Función par enviar un código de operación (header) via sockets */
void enviarHeader(int unSocket, int unHeader);

/* Funciones para notificar a los clientes conectados de un cierre inesperado */
void enviarAvisoDeCierre();
void notificarCaida();

/* Función para atender una conexión en particular */
void atenderConexion(void *numeroCliente);

//mata un entrenador
void matar(entrenador* en);

#endif /* LIBRERIAPOKEDEXSERVIDOR_H_ */
