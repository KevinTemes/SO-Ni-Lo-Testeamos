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

t_log* logs;
t_list* ips; // libero main
t_list* puertos; // libero main
//char* objetivoDeMapa; //libero main
//char** objetivosMapa; //libero main
t_dictionary* pokesDeCadaMapa; // lo uso en el main

typedef struct{
	int posXInicial;
	int posYInicial;
}t_actualizarPos;

typedef struct{
	int pos;
	int posObjetivo;
	int cantDeadlocks;
	int salirDeObjetivos;
	int cargarDeNuevoObjetivo;
	int reintentosActualizados;
	int cantMuertes;
}t_posMapaposObjetivoYDeadlocks;


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
}t_tiempoBloqueado;

typedef struct{
	char* especie;
	char* nombreMetadata;
	int nivelPokemon;
}t_pokemonDeserializado;

typedef struct{
	char* ipMapa;
	int puertoMapa;
}t_mapa;

typedef struct {
    char* nombreEntrenador;
    char* caracter;
    t_list* hojaDeViaje;
    t_list* pokemonsPorMapaCapturados;
    t_list* listaNivAtrapados;
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

void* deserializoPokemon(t_calculoTiempo*,t_tiempoBloqueado*,char*,char*,char*);
void* agarrarPokeConMasNivel(t_list*);
void terminarAventura(t_calculoTiempo*,t_tiempoBloqueado*,char*);
char* empezarAventura();
void copiarMedalla(char*);
void copiarArchivo(char*, char*, char*);
void* recibirDatos(int, int);
void* moverseEnUnaDireccion(t_actualizarPos*,int,int);
void* solicitarAtraparPokemon(t_calculoTiempo*,t_tiempoBloqueado*,char*);
void* sacarTiempo(t_calculoTiempo*,t_tiempoBloqueado*,char*,char*,char*);
void borrarArchivosBill();
void borrarMedallas();
void resetear();
void reciboUnaVida();
void pierdoUnaVida();
void handler(int n);
void muerePorSignal();
void muerePorDeadlock();
void resetearDesdeCero();

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
