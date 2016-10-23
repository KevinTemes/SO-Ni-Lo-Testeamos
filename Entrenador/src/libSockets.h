/*
 * libSockets.h
 *
 *  Created on: 1/9/2016
 *      Author: utnso
 */

#ifndef LIBSOCKETS_H_
#define LIBSOCKETS_H_

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/temporal.h>
#include <signal.h>

int servidor;
t_list* ips; // lo libero en el main
t_list* puertos; // lo libero en el main
char* objetivoDeMapa;
char** objetivosMapa;
t_dictionary* pokesDeCadaMapa; // lo uso en el main
char* nombre;
char* simbolo;

typedef struct{
	int cantDeadlocks;
}t_cantidadDeadlocks;

typedef struct{
	int hInicio;
	int mInicio;
	int sInicio;
	int milInicio;
	int hFin;
	int mFin;
	int sFin;
	int milFin;
}t_calculoTiempo;


typedef struct{
int horasBloqueado;
int minutosBloqueado;
int segundosBloqueado;
int milesimasBloqueado;
}t_tiempoBloqueado;

typedef struct{
	int protocolo;
	char* especie;
	char* nombreMetadata;
	int nivelPokemon;
}t_pokemonDeserializado;

typedef struct{
	char* ipMapa;
	int puertoMapa;
}t_mapa;

t_mapa* cosasMapa;

typedef struct {
    char* nombreEntrenador;
    char* caracter;
    t_list* hojaDeViaje;
    t_list* objetivosPorMapa;
    int cantidadInicialVidas;
    int reintentos;
}t_entrenador;


typedef struct Paquete {
	int codigoOperacion;
	int programCounter;
	int pid;
	int quantum;
	int tamanio;
	char *path;
} t_paquete;

int agarrarPokeConMasNivel(t_list*, t_pokemonDeserializado*);
void terminarAventura(t_calculoTiempo*,t_tiempoBloqueado*,t_cantidadDeadlocks*,char*);
char* empezarAventura();
void copiarMedalla(char*);
void copiarArchivo(char*, char*, char*);
void* recibirDatos(int, int);
void moverseEnUnaDireccion(int,int,int,int,char*);
void* solicitarAtraparPokemon(t_calculoTiempo*,t_tiempoBloqueado*, t_dictionary*,t_pokemonDeserializado*,t_cantidadDeadlocks*, t_list*,char*);
void* sacarTiempo(t_calculoTiempo*,t_tiempoBloqueado*,char*,char*,char*);
void borrarArchivosBill();
void borrarMedallas();
void morir(char*);
void reconectarseAlMismoMapa();
void resetear();
void reiniciarHojaDeViaje();
void reciboUnaVida();
void pierdoUnaVida();
void handler(int n);

/* setup_listen(IP,PORT) *
 * Devuelve el socket que se consiguió para escuchar
 *
 * IP = Ip de escucha
 * PORT = Puerto de escucha
 */
int setup_listen(char*, char*);

/* setup_listen(IP,PORT,LOGGER) *
 * Variante del setup_listen con la posibilidad de loggeo
 *
 * IP = Ip de escucha
 * PORT = Puerto de escucha
 * LOGGER = Log para escribir
 */
int setup_listen_con_log(char*, char*, t_log *);

/* cargarInfoSocket(IP,PORT) *
 * Devuelve un addr configurado
 *
 * IP = Ip de escucha
 * PORT = Puerto de escucha
 */
struct addrinfo* cargarInfoSocket(char *, char*);

/* conectarCliente(IP,PORT) *
 * Devuelve el socket que se conectó con el servidor
 *
 * IP = Ip de escucha
 * PORT = Puerto de escucha
 */
int conectarCliente(char *IP, char* Port);

/* conectarCliente_con_log(IP,PORT,LOGGER) *
 * Devuelve el socket que se conectó con el servidor
 *
 * IP = Ip de escucha
 * PORT = Puerto de escucha
 * LOGGER = Log para escribir
 */
int conectarCliente_con_log(char *IP, char* Port, t_log *);

/* esperarConexionEntrante(SocketEscucha,Backlog,LOGGER)
 * Devuelve el socket para mandar datos
 *
 * LOGGER = Log para escribir
 */
int esperarConexionEntrante(int, int, t_log *);
int conectarServidor(char* IP, char* Port, int backlog);
t_paquete *generarPaquete(int codigoOperacion, int tamMessage, char *message,
int programCounter, int quantum, int pid);
char *serializar(t_paquete *unPaquete);
t_paquete *deserializar_header(char *buffer);
void deserializar_data(t_paquete *unPaquete, char *buffer);
void destruirPaquete(t_paquete * unPaquete);
int leerConfigEnt(char *ruta, t_entrenador **datos, char* puntoMontaje);
void obtengoCadaUno(char* elemento);

#endif /* LIBSOCKETS_H_ */
