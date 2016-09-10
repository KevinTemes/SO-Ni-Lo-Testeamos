/*
 * libSockets.h
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#ifndef LIBSOCKETS_H_
#define LIBSOCKETS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/log.h>



typedef struct{
	int tiempoChequeoDeadlock;
	bool batalla; // 0= desactivado 1=activado
	char* algoritmo;
	int quantum;
	int retardoQ;
	char* ip;
	int puerto;
}metaDataComun;

typedef struct{
	char* tipoPokemon;
	char* posicion;
	char caracterPokeNest;
}metaDataPokeNest;

typedef struct{
	int nivel;
	char caracterPokemon;

}metaDataPokemon;

int leerConfiguracion(char* ruta, metaDataComun **datos);
int leerConfigPokenest(char* ruta, metaDataPokeNest **datos);
int leerConfigPokemon(char* ruta, metaDataPokemon **datos);
struct addrinfo* cargarInfoSocket(char *IP, char* Port);

#endif /* LIBSOCKETS_H_ */
