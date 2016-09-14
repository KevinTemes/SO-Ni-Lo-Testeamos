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
	char* nombreMapa;
	char** pokenests;
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
	int cantPokemons;
}metaDataPokeNest;

typedef struct{
	int nivel;
	char caracterPokemon;

}metaDataPokemon;

int leerConfiguracion(char* ruta, metaDataComun **datos);
int leerConfigPokenest(char* ruta, metaDataPokeNest **datos);
int leerConfigPokemon(char* ruta, metaDataPokemon **datos);

int setup_listen(char* IP, char* Port);
int setup_listen_con_log(char* IP, char* Port, t_log * logger);
struct addrinfo* cargarInfoSocket(char *IP, char* Port);
int conectarCliente(char *IP, char* Port);
int conectarCliente_con_log(char *IP, char* Port, t_log * logger);
int esperarConexionEntrante(int socketEscucha, int BACKLOG, t_log * logger);
int conectarServidor(char* IP, char* Port, int backlog) ;


#endif /* LIBSOCKETS_H_ */
