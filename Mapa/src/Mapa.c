/*
 ============================================================================
 Name        : Mapa.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <nivel.h>
#include <tad_items.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "libSockets.h"
#include <curses.h>
#include <string.h>
#include <poll.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include "libreriaMapa.h"

#define PACKAGESIZE 1024

void* socketin(metaDataComun **datosMapa) {
int socketEscucha, retornoPoll;
int fd_index = 0;

struct pollfd fileDescriptors[100];
int cantfds = 0;

char* puertoChar = string_itoa((*datosMapa)->puerto);
socketEscucha = setup_listen((*datosMapa)->ip, puertoChar);

listen(socketEscucha, 1024);

fileDescriptors[0].fd = socketEscucha;
fileDescriptors[0].events = POLLIN;
cantfds++;

int enviar = 1;
int cliente = 1;
t_infoCliente *infoCliente;
t_infoCliente unCliente;
int n = 0;
int *numeroCliente;
//  pthread_t hiloImprimirGiladas[1024];
pthread_t hiloAtenderConexiones[1024];

while (enviar) {

llamadaPoll:

// Inicio la función poll()
retornoPoll = poll(fileDescriptors, cantfds, -1);

// valido que haya iniciado bien
if (retornoPoll == -1) {
printf("Error en la funcion poll\n");
}

// Recorro la lista de file descriptors chequeando si el poll() retornó por una modificación.
// De ser así, acepto la conexión, delego la atención del socket
// a un hilo y vuelvo para arriba.

for (fd_index = 0; fd_index < cantfds; fd_index++) {
if (fileDescriptors[fd_index].fd == socketEscucha) {
listen(socketEscucha, 100);
struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);
int socketCliente = accept(socketEscucha,
(struct sockaddr *) &addr, &addrlen);

infoCliente = malloc(sizeof(t_infoCliente));
infoCliente->cliente = cliente;
infoCliente->socket = socketCliente;

unCliente.cliente = cliente;
unCliente.socket = socketCliente;
clientesActivos[n] = unCliente;
int nroCliente = n++;
numeroCliente = malloc(sizeof(int));
numeroCliente = &nroCliente;

//    pthread_create(&hiloImprimirGiladas[n],NULL, imprimirGiladas, infoCliente);
pthread_create(&hiloAtenderConexiones[n], NULL,
(void*) atenderConexion, numeroCliente);

cliente++;
n++;

fileDescriptors[cantfds].fd = socketCliente;
fileDescriptors[cantfds].events = POLLIN;
cantfds++;

goto llamadaPoll;
}
}
}
}

int main(int argc, char* argv[]) {

//VARIABLES PIOLA
int nE = 0; //numero entrenador

t_log* logs;

t_list *pokenests = list_create();
t_list* items = list_create();
t_list* finalizaAnormal = list_create();
t_list* listaDeColasAccion = list_create();

t_queue* colaListos = queue_create();
t_queue* esperaCapturarPokemon = queue_create();

typedef struct pa {
char simbolo;
char accion;
int numeroLlegada;
int numeroCliente;
int flagEstaEnLista;
int posx;
int posy;
} entrenador;

//LOGS
remove("Mapa.log");
puts("Creando archivo de logueo...\n");
logs = log_create("Mapa.log", "Mapa", true, log_level_from_string("INFO"));
puts("Log Mapa creado exitosamente \n");

// CONFIG
metaDataComun* datosMapa;
metaDataPokeNest* datosPokenest;
metaDataPokemon* datosPokemon;

datosMapa = malloc(sizeof(metaDataComun));
datosPokenest = malloc(sizeof(metaDataPokeNest));
datosPokemon = malloc(sizeof(metaDataPokemon));

char* configMetaMapa = string_from_format("%s/Mapas/%s/metadata", argv[2],
argv[1]);
if (!leerConfiguracion(configMetaMapa, &datosMapa)) {
log_error(logs,
"Error al leer el archivo de configuracion de Metadata\n");
return 1;
}

//por ahora
char* configPokenest = string_from_format("%s/Mapas/%s/PokeNests", argv[2],
argv[1]);
if (!leerConfigPokenest(configPokenest, pokenests)) {
log_error(logs,
"Error al leer el archivo de configuracion de Metadata Pokenest\n");
return 2;
}

char* configPoke = string_from_format(
"%s/Mapas/%s/PokeNests/Pikachu/Pikachu001.dat", argv[2], argv[1]);
if (!leerConfigPokemon(configPoke, &datosPokemon)) {
log_error(logs,
"Error al leer el archivo de configuracion de Metadata de Pokemons\n");
return 3;
}

log_info(logs,
"Los tres archivos de config fueron creados exitosamente!\n");

//SOCKETS
log_info(logs,
"iniciado el servidor principal del Mapa. Aguardando conexiones...\n\n");
pthread_t socketServMapa;
signal(SIGINT, notificarCaida);
pthread_create(&socketServMapa, NULL, (void*) socketin, &datosMapa); // Habre flashiado?

int rows; // nro de filas
int cols; // nro de columnas
int q, p; // dos valores para que se ubique un personaje

int x = 1; // x del otro personaje
int y = 1; // y del otro personaje

nivel_gui_inicializar();
nivel_gui_get_area_nivel(&rows, &cols);

// para que el otro personaje arranque desde la otra punta
p = 0;
q = 0;

CrearPersonaje(items, '@', p, q);
CrearPersonaje(items, '#', x, y);

//POKENEST
int ka;

for (ka = 0; ka < list_size(pokenests); ka++) {
datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
char** posPoke;
posPoke = string_split(datosPokenest->posicion, ";");
char ide;
ide = datosPokenest->caracterPokeNest[0];

CrearCaja(items, ide, atoi(posPoke[0]), atoi(posPoke[1]),
datosPokenest->cantPokemons);
}

nivel_gui_dibujar(items, "Mapa con Entrenadores");
    int quantum = datosMapa -> quantum;

while (1) {
//gestion de llegada (simbolo y accion) ej: @2 $8 etc
while (1) {
entrenador* entrenador; // antes era entrenador entrenador
entrenador->simbolo = paqueton[0]; // entrenador.simbolo = paqueton[0];
entrenador->accion = paqueton[1]; //asi sucesivamente en vez de -> era .
entrenador->flagEstaEnLista = 0;
entrenador->numeroCliente = numEntrenador;

if (!entrenador->flagEstaEnLista) {
entrenador->numeroLlegada = nE;
entrenador->flagEstaEnLista = 1;
entrenador->posx = 0;
entrenador->posy = 0;

t_queue *colaAccion = queue_create();
queue_push(colaAccion, (void*) entrenador->accion); // Fijarse el &
//agrego cola con acciones del entrenador a una lista
list_replace(listaDeColasAccion, entrenador->numeroLlegada,
(void*) colaAccion);

nE++;

CrearPersonaje(items, entrenador->simbolo, entrenador->posx, entrenador->posy);
queue_push(colaListos, (void*) entrenador);
log_info(logs,"entrenador %c a listos", entrenador->simbolo);
}

else {
t_queue *cola = queue_create();
cola = (t_queue*) list_get(listaDeColasAccion,entrenador->numeroLlegada);
queue_push(cola, (void*) entrenador->accion);
list_replace(listaDeColasAccion, entrenador->numeroLlegada,
(void*) cola);

}
}

//arranque de planificacion
while (1) {
int acto;
t_queue *colaAction = queue_create();
entrenador* ent1;
ent1 = (entrenador*) queue_pop(colaListos);
colaAction = (t_queue *) list_get(listaDeColasAccion,
ent1->numeroLlegada);
acto = (int) queue_pop(colaAction);

if (isalpha(acto)) {
int ka;
for (ka = 0; ka < list_size(pokenests); ka++) {
datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
if(datosPokenest->caracterPokeNest == acto){
send((clientesActivos[ent1->numeroCliente]).socket, datosPokenest->posicion, string_length(datosPokenest->posicion), 0);
ka = list_size(pokenests);
}// antes era send (clientesActivos[ent1.numeroCliente], ...
} // Tiene que ser .socket en vez de ->socket porque al hacer clienteActivos
} // [algo] ya lo estas desreferenciando y no va a funcionar porque ya no seria un puntero

if (isdigit(acto)) {
if (acto == 2 || acto == 4 || acto == 6 || acto == 8) {
switch (acto) {

case 8:
if (ent1->posy > 1) {
ent1->posy--;
}
break;

case 2:
if (ent1->posy < rows) {
ent1->posy++;
}
break;

case 4:
if (ent1->posx > 1) {
ent1->posx--;
}
break;
case 6:
if (ent1->posx < cols) {
ent1->posx++;
}
break;

MoverPersonaje(items, ent1->simbolo, ent1->posx, ent1->posy);

}
}

}

/* if (   ((p == 26) && (q == 10)) || ((x == 26) && (y == 10)) ) {
restarRecurso(items, 'H');
}

if (   ((p == 19) && (q == 9)) || ((x == 19) && (y == 9)) ) {
restarRecurso(items, 'F');
}

if (   ((p == 8) && (q == 15)) || ((x == 8) && (y == 15)) ) {
restarRecurso(items, 'M');
} */

nivel_gui_dibujar(items, "Mapa con Entrenadores");
}
}

//liberamos las listas y toda la ganzada


for (ka = 0; ka < list_size(pokenests); ka++) {
datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
char ide;
ide = datosPokenest->caracterPokeNest[0];
BorrarItem(items, ide);
}

nivel_gui_terminar();

free(datos); //siendo datos una variable global para el almacenamiento de pokenest

return EXIT_SUCCESS;

}
