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

typedef struct pa {
	char pokenestAsignado;
	char simbolo;
	char accion;
	int numeroLlegada;
	int numeroCliente;
	int flagEstaEnLista;
	int posx;
	int posy;
	int posPokex;
	int posPokey;
	int flagLeAsignaronPokenest;
} entrenador;

/* Función loca para testear rececpción de mensajes a través de un socket. */
void imprimirGiladas(void *unCliente);

/* Función para imprimir por pantalla el contenido de un archivo de texto */
void imprimir_archivo(char *rutaDelArchivo);

/* Función para volcar el contenido de un archivo .txt dentro de un string */
char *txtAString(char *rutaDelArchivo);

/* Función par enviar un código de operación (header) via sockets */
void enviarHeader(int unSocket, int unHeader);

/* Funciones para notificar a los clientes conectados de un cierre inesperado */
void enviarAvisoDeCierre();
void notificarCaida();

/* Función para atender una conexión en particular */
void atenderConexion(void *numeroCliente);

#endif /* LIBRERIAPOKEDEXSERVIDOR_H_ */
