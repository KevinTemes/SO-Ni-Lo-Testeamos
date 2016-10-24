/*
 * libreriaPokedexServidor.h
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#ifndef LIBRERIAPOKEDEXSERVIDOR_H_
#define LIBRERIAPOKEDEXSERVIDOR_H_
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netdb.h>
#include <unistd.h>
#include "libSockets.h"
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <poll.h>
#include <semaphore.h>
#include <signal.h>
#include "osada.h"
#include "TestServidor.h"

typedef struct{
	int cliente;
	int socket;
} t_infoCliente;

typedef struct{
	char *nombre;
	char *padre;
	char *abuelo;
	int largoNombre;
	int largoPadre;
	int largoAbuelo;
}t_infoDirectorio;

typedef char bloque[64];

t_infoCliente clientesActivos[1024];

osada_header mainHeader;
osada_file tablaDeArchivos[2048];
int *discoMapeado;

disco_osada miDisco;

/* Función para obtener el nombre de un archivo desde una tabla de archivos, en formato char * */
char *getFileName(unsigned char *nombreArchivo);

// Función para obtener el nombre de un directorio, dada su ruta
char *getNombreDirectorio(char *ruta);

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

/* Serialización de strings */
void *serializarString(char *unString);

/* Búsqueda de la posicion de un un archivo/directorio en la tabla de archivos */
int buscarArchivo(char *unaRuta);

/* copiado de un bloque de datos en un buffer */
void copiarBloque(void *buffer, int bloque, int offset);


/* implementaciones de operaciones del filesystema osada */
int osada_getattr(char *unaRuta);

char *osada_readdir(char *unDirectorio);

void *osada_read(char *unaRuta);

int osada_crearArchivo(char *ruta);

int osada_editaArchivo(char *ruta, void *nuevoContenido);

int osada_unlink(char *ruta);

int osada_mkdir(char *ruta, char nombreDir);

int osada_rmdir(char *ruta);

int osada_rename(char *ruta, char *nuevoNombre);

#endif /* LIBRERIAPOKEDEXSERVIDOR_H_ */
