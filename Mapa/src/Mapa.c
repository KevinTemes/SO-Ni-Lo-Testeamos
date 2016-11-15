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
#include <pkmn/battle.h>
#include <pkmn/factory.h>

#define PACKAGESIZE 1024

//defino pokimon
pokimons pic;

//variables globales
extern metaDataPokeNest *datos;
extern t_infoCliente clientesActivos[1024];
char* nombreMapa;
char* configMapa;

//variables del mapa
int nE = 0; //numero entrenador
metaDataComun* datosMapa;
metaDataPokeNest* datosPokenest;
metaDataPokemon* datosPokemon;
int rows; // nro de filas
int cols; // nro de columnas

//log
t_log* logs;

//listas
t_list* pokemons;
t_list* pokenests;
t_list* disponibles;
t_list* items;
t_list* entrenadoresEnCurso;
t_list* entrenadoresEnDeadlock;
//colas
t_queue* colaListos;
t_queue* colaBloqueados;

//semaforos
sem_t sem_Listos;
sem_t sem_Bloqueados;
sem_t sem_quantum;

//declara hilos
pthread_t hiloDePlanificador;
pthread_t hiloDeadlock;
pthread_t hiloDeBloqueados;
pthread_t hiloAtenderConexiones[1024];

//deadlock
void banquero() {

	while (1) {
		usleep(datosMapa->tiempoChequeoDeadlock);
		if (list_size(entrenadoresEnCurso)) {
			t_list* vectorT = list_create();
			int auxiliar;
			for (auxiliar = 0; auxiliar < list_size(disponibles); auxiliar++) {
				tabla* vecto = malloc(sizeof(tabla));
				tabla* punter = list_get(disponibles, auxiliar);
				vecto->pokenest = punter->pokenest;
				vecto->valor = punter->valor;
				list_add(vectorT, vecto);
			}
			int iaux;
			for (iaux = 0; iaux < list_size(entrenadoresEnCurso); iaux++) {
				entrenador* en;
				tabla* t;
				en = list_get(entrenadoresEnCurso, iaux);
				int auxixi;
				int flago = 0;
				for (auxixi = 0;
						auxixi < list_size(en->asignados) && flago == 0;
						auxixi++) {
					t = list_get(en->asignados, auxixi);
					if (t->valor != 0) {
						flago = 1;
					}
				}
				if (flago == 0) {
					en->estaMarcado = 1;
				}

			}
			int auu;

			for (auu = 0; auu < list_size(entrenadoresEnCurso); auu++) {
				entrenador* an;
				tabla* ta;
				tabla* b;
				an = list_get(entrenadoresEnCurso, auu);
				if (!an->estaMarcado) {
					int j;
					int val = 1;
					for (j = 0; j < list_size(an->solicitud) || val == 1; j++) {
						ta = list_get(an->solicitud, j);
						b = list_get(vectorT, j);
						val = ta->valor <= b->valor;
					}
					if (j == list_size(an->solicitud)) {
						tabla* fa;
						tabla* fe;
						an->estaMarcado = 1;
						int x;
						for (x = 0; x < list_size(an->asignados); x++) {
							fa = list_get(an->asignados, x);
							fe = list_get(vectorT, x);
							fe->valor = fa->valor + b->valor;
							auu = 0;
						}

					}
				}
			}
			int pooi;
			entrenadoresEnDeadlock = list_create();
			for (pooi = 0; pooi < list_size(entrenadoresEnCurso); pooi++) {
				entrenador* entreneitor;
				entreneitor = list_get(entrenadoresEnCurso, pooi);
				if (!entreneitor->estaMarcado) {
					int accione = 3;
					send((clientesActivos[entreneitor->numeroCliente]).socket,
							(void*) accione, sizeof(int), 0);
					list_add(entrenadoresEnDeadlock, entreneitor);
				}
			}

			if (list_size(entrenadoresEnDeadlock)) {

				bool llegoPrimero(entrenador* a, entrenador* b) {
					return a->numeroLlegada < b->numeroLlegada;
				}
				list_sort(entrenadoresEnDeadlock, (void*) llegoPrimero);




				//hora de peleaaaaar
				if (datosMapa->batalla) {
					int otroAux;
					for (otroAux = 0;
							otroAux < list_size(entrenadoresEnDeadlock);
							otroAux++) {
						t_pkmn_factory* facto = create_pkmn_factory();
						t_pokemon* pokegold;
						t_pokemon* pokesilver;
						t_pokemon* pokeperdedor;
						entrenador* gold;
						entrenador* silver;
						gold = list_get(entrenadoresEnDeadlock, otroAux);
						silver = list_get(entrenadoresEnDeadlock, otroAux + 1);
						pokegold = create_pokemon(facto,
								(gold->pokePeleador)->especie,
								(gold->pokePeleador)->nivel);
						pokesilver = create_pokemon(facto,
								(silver->pokePeleador)->especie,
								(silver->pokePeleador)->nivel);
						pokeperdedor = pkmn_battle(pokegold, pokesilver);
						int accionar = 0;
						if (!strcmp(pokegold->species, pokeperdedor->species)
								&& (pokegold->level == pokeperdedor->level)) {
							send(clientesActivos[silver->numeroCliente].socket,
									(void*) accionar, sizeof(int), 0);
							list_remove(entrenadoresEnDeadlock, otroAux + 1);
							otroAux--;
						} else {
							send(clientesActivos[gold->numeroCliente].socket,
									(void*) accionar, sizeof(int), 0);
							list_remove(entrenadoresEnDeadlock, otroAux);
						}

					}
					entrenador* muerto;
					muerto = list_get(entrenadoresEnDeadlock, 0);
					int accion = 7;
					send(clientesActivos[muerto->numeroCliente].socket,
							(void*) accion, sizeof(int), 0);
					muerto->fallecio=1;
				}
			}
			list_destroy_and_destroy_elements(vectorT, (void*) free);
		}
	}
}

//arranque de planificacion
void planificador(void* argu) {

	char* argument = (char*) argu;

	while (1) {
		sem_wait(&sem_Listos); //semaforo de nuevos bloqueando que se saque un ent si la cola esta vacia

		int q = datosMapa->quantum;

		log_info(logs,"se mete a planificador");
		//caso roun robin
		if (!strcmp(datosMapa->algoritmo, "RR")) {

			int acto;
			entrenador* entre;

			entre = (entrenador*) queue_pop(colaListos);

			while (q && !(entre->fallecio)) {
				//sem_wait(&sem_quantum);
				acto = (int) queue_pop(entre->colaAccion);
			    //log_info(logs,"funca3");

				if (isalpha(acto)) {
					int ka;
					for (ka = 0; ka < list_size(pokenests); ka++) {
						datosPokenest = (metaDataPokeNest*) list_get(pokenests,
								ka);
						if (datosPokenest->caracterPokeNest[0] == acto) {

							log_info(logs, "antes del send %s",
									datosPokenest->posicion);
							int pedo;
							pedo =send((clientesActivos[entre->numeroCliente]).socket,datosPokenest->posicion, 5, 0);

							log_info(logs, "Se envio coordenadas: %d", pedo);

							char** posicionPoke;
							posicionPoke = string_split(datosPokenest->posicion,";");

							entre->posPokex = atoi(posicionPoke[0]);
							entre->posPokey = atoi(posicionPoke[1]);
							entre->pokenestAsignado =
									datosPokenest->caracterPokeNest[0];
							entre->flagLeAsignaronPokenest = 1;

							ka = list_size(pokenests);

						}
					}

				}

				//8 es 56, 2 es 50, 4 es 52, 6 es 54

				if (isdigit(acto)) {
					if (acto == '2' || acto == '4' || acto == '6'
							|| acto == '8') {

						//usleep(datosMapa->retardoQ);
						sleep(1);
						switch (acto) {

						case '8':
							if (entre->posy > 1) {
								log_info(logs, "entrenador %c se mueve arriba",entre->simbolo);
                                usleep(datosMapa->retardoQ);
								entre->posy--;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								log_info(logs,"valor de quantum %d",q);
							}
							break;

						case '2':
							if (entre->posy < rows) {
								log_info(logs, "entrenador %c se mueve abajo", entre->simbolo);
                                usleep(datosMapa->retardoQ);
								entre->posy++;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								log_info(logs,"valor de quantum %d",q);
							}
							break;

						case '4':
							if (entre->posx > 1) {
								log_info(logs, "entrenador %c se mueve a la izquierda", entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posx--;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								log_info(logs,"valor de quantum %d",q);
							}
							break;
						case '6':
							if (entre->posx < cols) {
								log_info(logs, "entrenador % c se mueve a la derecha", entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posx++;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);

								q--;
								log_info(logs, "valor de quantum %d", q);
							}
							break;

						}
					}

					if (acto == '9') {
						usleep(datosMapa->retardoQ);
						queue_push(colaBloqueados, entre);
						q = 0;
						sem_post(&sem_Bloqueados);

					}

				}
			}

			if (entre->fallecio) {
				log_info(logs,"Ahora lo mata");
				matar(entre);
				//sem_post(&sem_quantum);
				entre->fallecio=0;
			}

			if (q == 0 && !(entre->fallecio)) {
				usleep(datosMapa->retardoQ);
				//q = datosMapa->quantum;
				queue_push(colaListos, entre);
				sem_post(&sem_Listos);
			}

		}
		if (!strcmp(datosMapa->algoritmo, "SDRF")) {
			sem_wait(&sem_Listos);
			t_list* listaAux = list_create();
			while (!queue_is_empty(colaListos)) {
				entrenador* ent;
				ent = queue_pop(colaListos);
				list_add(listaAux, ent);
			}
			bool esMasCerca(entrenador *cerca, entrenador *lejos) {
				if (cerca->flagLeAsignaronPokenest
						&& lejos->flagLeAsignaronPokenest) {
					return ((cerca->posx - cerca->posPokex)
							+ (cerca->posy - cerca->posPokey))
							< ((lejos->posx - lejos->posPokex)
									+ (lejos->posy - lejos->posPokey));
				} else {
					return -1;
				}
			}
			list_sort(listaAux, (void*) esMasCerca);

		}

	}
}

void bloqueados() {
	while (1) {
		usleep(datosMapa->retardoQ);
		sem_wait(&sem_Bloqueados);
		log_info(logs, "Se mete a bloqueados");
		entrenador *ent1;
		pokimons* poki;
		log_info(logs, "Va a extraer un bloqueado");
		ent1 = (entrenador*) queue_pop(colaBloqueados);

		if (!ent1->fallecio) {

			log_info(logs, "Extrajo un bloqueado");
			bool esLaPokenest(pokimons *parametro1) {
				return ent1->pokenestAsignado == parametro1->pokinest;
			}
			log_info(logs, "Ahora busca un poki");
			poki = list_find(pokemons, (void*) esLaPokenest);
			log_info(logs, "Saca un poki");
			log_info(logs, "%c", poki->pokinest);
			int auxi67;
			int flagito = 0;
			int capturo = 0;
			bool esLaPokenest2(tabla* a) {
				return ent1->pokenestAsignado == a->pokenest;
			}

			for (auxi67 = 0;
					auxi67 < list_size(poki->listaPokemons) && flagito == 0;
					auxi67++) {
				metaDataPokemon* pokem;
				pokem = list_get(poki->listaPokemons, auxi67);
				log_info(logs, "Saca un pokemon de la lista poki");
				log_info(logs, "%s", pokem->especie);
				log_info(logs, "%d", pokem->nivel);
				if (!pokem->estaOcupado) {
					char* nombreAux = pokem->nombreArch;
					char** nombreSinDatAux = string_split(nombreAux, ".");
					char* nombreSinDAT = nombreSinDatAux[0];

					log_info(logs, "Creo el nombre sin DAT: %s", nombreSinDAT);
					int protocolo = 1;

					int tamanioCosaUno = sizeof(char) * strlen(pokem->especie);
					log_info(logs,"tamanio especie %d", tamanioCosaUno);
					int tamanioCosaDos = sizeof(char) * strlen(nombreSinDAT);
					log_info(logs,"tamanio sin dat %d",tamanioCosaDos);

					int auxilia = pokem->nivel;
					void* miBuffer = malloc(
							(4 * sizeof(int)) + tamanioCosaUno
									+ tamanioCosaDos);
					memcpy(miBuffer, &protocolo, sizeof(int));
					memcpy(miBuffer + sizeof(int), &tamanioCosaUno,
							sizeof(int));
					memcpy(miBuffer + (2 * sizeof(int)), &tamanioCosaDos,
							sizeof(int));

					log_info(logs, "metio bien tamaños en buffer");

					//convertir Ruta de especie, nombreSinDat
					char* caracterNulo = string_new();
					caracterNulo = "\0";
					string_append(&pokem->especie,caracterNulo);
					string_append(&nombreSinDAT,caracterNulo);

					memcpy(miBuffer + (3 * sizeof(int)), pokem->especie,
							tamanioCosaUno); //VERIFICA DESPUES
					log_info(logs, "mete bien especie:%s", pokem->especie);
					memcpy(miBuffer + (3 * sizeof(int)) + tamanioCosaUno,
							nombreSinDAT, tamanioCosaDos); //VERIFICAR DESPUES
					log_info(logs, "mete bien nombreSinDat:%s", nombreSinDAT);
					memcpy(
							miBuffer + (3 * sizeof(int)) + tamanioCosaUno
									+ tamanioCosaDos, &auxilia, sizeof(int)); //VERIFICAR DESPUES
					log_info(logs, "mete bien nivel:%d", pokem->nivel);
					log_info(logs, "mete bien mierda en buffer");

					int e;
					e = send((clientesActivos[ent1->numeroCliente]).socket,
							miBuffer,
							(4 * sizeof(int)) + tamanioCosaUno + tamanioCosaDos,
							0);

					log_info(logs, "envio la mierda %d", e);

					//free(caracterNulo); // si rompe, sacarlo
					free(miBuffer);
					flagito = 1;
					capturo = 1;
					pokem->estaOcupado = 1;

					tabla* t;
					tabla* dispo1;

					t = list_find(ent1->asignados, (void*) esLaPokenest2);
					dispo1 = (tabla*) list_find(disponibles,
							(void*) esLaPokenest2);

					t->valor++;
					dispo1->valor--;

					list_add(ent1->pokemones, pokem);

					log_info(logs, "llego a bloqueados");

					restarRecurso(items, poki->pokinest);
					nivel_gui_dibujar(items, nombreMapa);

					usleep(datosMapa->retardoQ);

					queue_push(colaListos, ent1);

					sem_post(&sem_Listos);

					log_info(logs,"pushea a entrenador a listos");
				}

			}
			if (!capturo && !ent1->entroBloqueados) {
				tabla* tab;
				tab = list_find(ent1->solicitud, (void*) esLaPokenest2);
				tab->valor++;
				ent1->entroBloqueados = 1;
				queue_push(colaBloqueados, ent1);
				sem_post(&sem_Bloqueados);
			}
			if (!capturo && ent1->entroBloqueados) {
				queue_push(colaBloqueados, ent1);
				sem_post(&sem_Bloqueados);
			}
		} else {
			matar(ent1);
		}
	}
}

///////////////////////////////////////////Main//////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	//nombre de mapa
	nombreMapa = argv[1];
	//inicializo listas

	pokemons = list_create();
	disponibles = list_create();
	pokenests = list_create();
	items = list_create();
	entrenadoresEnCurso = list_create();
	//inicializo colas
	colaListos = queue_create();
	colaBloqueados = queue_create();
	//inicializo semaforos
	sem_init(&sem_Listos, 0, 0);
	sem_init(&sem_Bloqueados, 0, 0);
	sem_init(&sem_quantum, 0, 0);

	remove("Mapa.log");
	logs = log_create("Mapa.log", "Mapa", false, log_level_from_string("INFO"));

// CONFIG

	datosMapa = malloc(sizeof(metaDataComun));
	//datosPokenest = malloc(sizeof(metaDataPokeNest));
	//datosPokemon = malloc(sizeof(metaDataPokemon));

	configMapa = string_from_format("%s/Mapas/%s/metadata", argv[2], argv[1]);

	leerConfiguracion();

//por ahora
	char* configPokenest = string_from_format("%s/Mapas/%s/PokeNests", argv[2],
			argv[1]);
	if (!leerConfigPokenest(configPokenest, pokenests)) {
		log_error(logs,
				"Error al leer el archivo de configuracion de Metadata Pokenest\n");
		return 2;
	}

	char* configPoke = string_from_format("%s/Mapas/%s/PokeNests", argv[2],
			argv[1]);
	if (!leerPokemons(configPoke, pokemons)) {
		log_error(logs,
				"Error al leer el archivo de configuracion de Metadata de Pokemons\n");
		return 3;
	}
	//ordeno las listas de pokemons de la lista pokemons
	bool esMenor(metaDataPokemon* pki1, metaDataPokemon* pki2) {
		char *pa;
		pa = string_reverse(pki1->nombreArch);
		char *pe;
		pe = string_from_format("%c%c%c", pa[6], pa[5], pa[4]);
		int numero1 = atoi(pe);
		char *pi;
		pi = string_reverse(pki2->nombreArch);
		char *po;
		po = string_from_format("%c%c%c", pi[6], pi[5], pi[4]);
		int numero2 = atoi(po);
		return numero1 < numero2;
	}
	int auxi2;
	pokimons* pake;
	for (auxi2 = 0; auxi2 < list_size(pokemons); auxi2++) {
		pake = list_get(pokemons, auxi2);
		list_sort(pake->listaPokemons, (void*) esMenor);
	}

	log_info(logs,
			"Los tres archivos de config fueron creados exitosamente!\n");

	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&rows, &cols);

	//POKENESTchar** posPoke;
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

	nivel_gui_dibujar(items, argv[1]);

	//hilo de planificacion

	pthread_create(&hiloDePlanificador, NULL, (void*) planificador,
			(void*) argv[1]);

	//hilo atencion a bloqueados

	pthread_create(&hiloDeBloqueados, NULL, (void*) bloqueados, NULL);

	//hilo deteccion de deadlock
	//pthread_create(&hiloDeadlock, NULL, (void*) banquero, NULL);

//SOCKETS
	log_info(logs,
			"iniciado el servidor principal del Mapa. Aguardando conexiones...\n\n");

	signal(SIGINT, notificarCaida);

	signal(SIGUSR2, leerConfiguracion);

	int socketEscucha, retornoPoll;
	int fd_index = 0;

	struct pollfd fileDescriptors[100];
	int cantfds = 0;

	char* puertoChar = string_itoa(datosMapa->puerto);
	socketEscucha = setup_listen(datosMapa->ip, puertoChar);

	listen(socketEscucha, 1024);

	fileDescriptors[0].fd = socketEscucha;
	fileDescriptors[0].events = POLLIN;
	cantfds++;

	int enviar = 1;
	int cliente = 1;
	//t_infoCliente *infoCliente;
	t_infoCliente unCliente;
	int n = 0;
	int *numeroCliente;

	//conexion
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

				/*	infoCliente = malloc(sizeof(t_infoCliente));
				 infoCliente->cliente = cliente;
				 infoCliente->socket = socketCliente;*/

				unCliente.cliente = cliente;
				unCliente.socket = socketCliente;
				clientesActivos[n] = unCliente;
				int nroCliente = n++;
				numeroCliente = malloc(sizeof(int));
				numeroCliente = &nroCliente;

				//    pthread_create(&hiloImprimirGiladas[n],NULL, imprimirGiladas, infoCliente);
				pthread_create(&hiloAtenderConexiones[n], NULL, atenderConexion,
						numeroCliente);

				cliente++;
				n++;

				fileDescriptors[cantfds].fd = socketCliente;
				fileDescriptors[cantfds].events = POLLIN;
				cantfds++;

				goto llamadaPoll;
			}
		}
	}

	//liberamos las listas y toda la ganzada

	for (ka = 0; ka < list_size(pokenests); ka++) {
		datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
		char ide;
		ide = datosPokenest->caracterPokeNest[0];
		BorrarItem(items, ide);
	}
	close(socketEscucha);
	nivel_gui_terminar();

	free(datos); //siendo datos una variable global para el almacenamiento de pokenest
	free(datosMapa);
	free(datosPokenest);
	free(datosPokemon);
	/*	list_destroy_and_destroy_elements(pokenests,free);
	 list_destroy_and_destroy_elements(items,free);
	 list_destroy_and_destroy_elements(listaDeColasAccion,free);
	 queue_destroy_and_destroy_elements(colaListos,free);
	 queue_destroy_and_destroy_elements(colaBloqueados,free);
	 */
	return EXIT_SUCCESS;

}
