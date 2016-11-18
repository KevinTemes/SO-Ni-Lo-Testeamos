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
t_list* listaContenedora;
t_list* deadlocks;
//colas
t_queue* colaListos;
t_queue* colaBloqueados;

//semaforos
sem_t sem_Listos;
sem_t sem_Bloqueados;
sem_t sem_quantum;

int numHilos;

//declara hilos
pthread_t hiloDePlanificador;
pthread_t hiloDeadlock;
pthread_t hiloDeBloqueados;
pthread_t hiloAtenderConexiones[1024];

pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexEnvio = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMuerte = PTHREAD_MUTEX_INITIALIZER;

//deadlock
void banquero() {

	while (1) {
		usleep(datosMapa->tiempoChequeoDeadlock);
		//sleep(2);
		//log_info(logs,"chequeo de la existencia de deadlock");

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
			log_info(logs, "cargo vector auxiliar");
			int iaux;
			for (iaux = 0; iaux < list_size(entrenadoresEnCurso); iaux++) {
				entrenador* en;
				tabla* t;
				en = list_get(entrenadoresEnCurso, iaux);
				if (!en->fallecio) {
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
						log_info(logs,
								"entrenador %c no esta en riesgo de deadlock por el momemnto",
								en->simbolo);
					}

				}
			}
			int auu;

			for (auu = 0; auu < list_size(entrenadoresEnCurso); auu++) {
				entrenador* an;
				tabla* ta;
				tabla* b;
				an = list_get(entrenadoresEnCurso, auu);
				log_info(logs, "saca entrenador en curso");
				if (!an->fallecio) {
					if (!an->estaMarcado) {
						log_info(logs, "entrenador %c no esta marcado",
								an->simbolo);
						int j;
						int val = 1;
						for (j = 0; j < list_size(an->solicitud) && val; j++) {
							ta = list_get(an->solicitud, j);
							log_info(logs, "solicitud de %c es %d", an->simbolo,
									ta->valor);
							b = list_get(vectorT, j);
							log_info(logs,
									"y el valor del vector auxiliar ahi es %d",
									b->valor);
							val = ta->valor <= b->valor;
						}
						if (val) {
							tabla* fa;
							tabla* fe;
							an->estaMarcado = 1;
							log_info(logs, "marca a %c", an->simbolo);
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

			}
			int pokixi;
			entrenadoresEnDeadlock = list_create();
			log_info(logs, "crea lista de entrenadores en deadlock");
			for (pokixi = 0; pokixi < list_size(entrenadoresEnCurso);
					pokixi++) {
				log_info(logs, "se mete al for pokixi");
				entrenador* entreneitor;
				//
				//	if(entreneitor->fallecio == 0){
				entreneitor = list_get(entrenadoresEnCurso, pokixi);
				log_info(logs, "entrenador %c esta marcado? %d",
						entreneitor->simbolo, entreneitor->estaMarcado);
				if (!(entreneitor->estaMarcado)) {
					log_info(logs, "entrenador %c en estado de deadlock");
					int accione = 3;
					int efe;
					efe =
							send(
									(clientesActivos[entreneitor->numeroCliente]).socket,
									&accione, sizeof(int), 0);
					log_info(logs, "%d", efe);
					list_add(entrenadoresEnDeadlock, entreneitor);
				}
				//	}
				//
			}

			//LISTA DE LISTAS DE ENTRENADORES EN DEADLOCK
			if (list_size(entrenadoresEnDeadlock)) {
				int auxiliar75;
				for (auxiliar75 = 0;
						auxiliar75 < list_size(entrenadoresEnDeadlock);
						auxiliar75++) {
					entrenador* entit;
					entit = list_get(entrenadoresEnDeadlock, auxiliar75);
					bool entrencontrado(entrenador* parametro) {
						tabla* tab;
						tabla* teb;
						int h3;
						int x = 0;
						for (h3 = 0; h3 < list_size(parametro->asignados);
								h3++) {
							tab = list_get(parametro->asignados, h3);
							teb = list_get(entit->solicitud, h3);
							if (tab->valor == teb->valor) {
								x = 1;
							}
							tab = list_get(parametro->solicitud, h3);
							teb = list_get(entit->asignados, h3);
							if (tab->valor == teb->valor) {
								x = 1;
							}
						}
						return x;

					}
					if (!list_size(deadlocks)) {
						t_list* unaLista = list_create();
						list_add(unaLista, entit);
						list_add(deadlocks, unaLista);
					} else {
						int jej;
						for (jej = 0; jej < list_size(deadlocks); jej++) {
							t_list* listina = list_get(deadlocks, jej);
							if (list_find(listina,
									(void*) entrencontrado)!=NULL) {
								list_add(listina, entit);
							} else {
								t_list* otraLista = list_create();
								list_add(otraLista, entit);
							}
						}
					}
				}
			}
			if (list_size(deadlocks)) {

				bool llegoPrimero(entrenador* a, entrenador* b) {
					return a->numeroLlegada < b->numeroLlegada;
				}
				int ej;
				for(ej=0;ej<list_size(deadlocks);ej++){
				t_list* listiti;
				listiti = list_get(deadlocks,ej);
				list_sort(entrenadoresEnDeadlock, (void*) llegoPrimero);
				}
				log_info(logs,
						"entrenadores ordenados, hora de la batalla pokemon");

				//hora de peleaaaaar
				if (datosMapa->batalla) {
					int otroAux;
					int yotromas;
					for(yotromas=0;yotromas<list_size(deadlocks);yotromas++){

					t_list* listota;
					listota = list_get(deadlocks,yotromas);

						for (otroAux = 0;otroAux < list_size(listota);otroAux++) {


						if (otroAux > 1) {
							t_pkmn_factory* facto = create_pkmn_factory();
							t_pokemon* pokegold;
							t_pokemon* pokesilver;
							t_pokemon* pokeperdedor;

							entrenador* gold;
							entrenador* silver;
							gold = list_get(listota, otroAux);
							silver = list_get(listota,
									otroAux + 1);

							if (!gold->fallecio && !silver->fallecio) {

								log_info(logs, "%c peleara con %c",gold->simbolo, silver->simbolo);
								pokegold = create_pokemon(facto,(gold->pokePeleador)->especie,(gold->pokePeleador)->nivel);
								log_info(logs,"pokemon del entrenador %c es %s y su nivel %d",gold->simbolo,(gold->pokePeleador)->especie,(gold->pokePeleador)->nivel);
								pokesilver = create_pokemon(facto,(silver->pokePeleador)->especie,(silver->pokePeleador)->nivel);
								log_info(logs,"pokemon del entrenador %c es %s y su nivel %d",silver->simbolo,(silver->pokePeleador)->especie,(silver->pokePeleador)->nivel);
								pokeperdedor = pkmn_battle(pokegold,pokesilver);
								int accionar = 0;
								if (!strcmp(pokegold->species,pokeperdedor->species)&& (pokegold->level== pokeperdedor->level)) {
									send(clientesActivos[silver->numeroCliente].socket,&accionar, sizeof(int), 0);
									list_remove(listota,otroAux + 1);
									log_info(logs, "entrenador %c victorioso",silver->simbolo);
									otroAux--;
								} else {
									send(
											clientesActivos[gold->numeroCliente].socket,&accionar, sizeof(int), 0);
									list_remove(listota,otroAux);
									log_info(logs, "entrenador %c victorioso",gold->simbolo);
								}
							}
						}
					}
					entrenador* muerto;
					muerto = list_get(listota, 0);
					int accion = 7;
					send(clientesActivos[muerto->numeroCliente].socket, &accion,sizeof(int), 0);
					log_info(logs, "entrenador %c sera eliminado",muerto->simbolo);
					int y;
					for (y = 0; y < list_size(muerto->pokemones); y++) {
						metaDataPokemon* pok;
						bloq* bli;
						tabla* d;
						pok = list_get(muerto->pokemones, y);
						bool esLaPokenest3(tabla* a) {
							return pok->especie[0] == a->pokenest;
						}
						d = list_find(disponibles, (void*) esLaPokenest3);
						d->valor++;
						sumarRecurso(items, d->pokenest);
						nivel_gui_dibujar(items, nombreMapa);
						pok->estaOcupado = 0;
						bool esLad(bloq* ver) {
							return ver->pokenest == pok->especie[0];
						}
						bli = list_find(listaContenedora, (void*) esLad);
						sem_post(&(bli->sembloq));

					}
					muerto->fallecio = 1;

				}
			}

			list_destroy_and_destroy_elements(vectorT, (void*) free);
			list_destroy(entrenadoresEnDeadlock);
			int otromas;
			for(otromas=0;otromas<list_size(deadlocks);otromas++){
				list_remove_and_destroy_element(deadlocks,otromas,(void*)free);
			}
		}
		int auxi23;
		for (auxi23 = 0; auxi23 < list_size(entrenadoresEnCurso); auxi23++) {
			entrenador* entri;
			entri = list_get(entrenadoresEnCurso, auxi23);
			entri->estaMarcado = 0;
		}
	}
}
}

//arranque de planificacion
void planificador(void* argu) {

	char* argument = (char*) argu;

	while (1) {
		sem_wait(&sem_Listos); //semaforo de nuevos bloqueando que se saque un ent si la cola esta vacia

		int q = datosMapa->quantum;

		//log_info(logs,"se mete a planificador");
		//caso roun robin
		if (!strcmp(datosMapa->algoritmo, "RR")) {

			int acto;
			entrenador* entre;

			int bloqueo = 0;
			entre = queue_pop(colaListos);

			while (q && (!(entre->fallecio) && queue_size(entre->colaAccion))) {
				//sem_wait(&sem_quantum);
				acto = queue_pop(entre->colaAccion);
				//log_info(logs,"funca3");

				if (isalpha(acto) && !entre->fallecio) {
					int ka;
					for (ka = 0; ka < list_size(pokenests); ka++) {
						datosPokenest = (metaDataPokeNest*) list_get(pokenests,
								ka);
						if (datosPokenest->caracterPokeNest[0] == acto) {

							//log_info(logs, "antes del send %s",datosPokenest->posicion);
							int pedo;
							pedo =
									send(
											(clientesActivos[entre->numeroCliente]).socket,
											datosPokenest->posicion, 5, 0);

							//log_info(logs, "Se envio coordenadas: %d", pedo);

							char** posicionPoke;
							posicionPoke = string_split(datosPokenest->posicion,
									";");

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

				if (isdigit(acto) && !(entre->fallecio)) {
					if (acto == '2' || acto == '4' || acto == '6'
							|| acto == '8') {

						//usleep(datosMapa->retardoQ);
						//usleep(datosMapa->retardoQ);
						sleep(1);
						switch (acto) {

						case '8':
							if (entre->posy > 1 && !entre->fallecio) {
								//log_info(logs, "entrenador %c se mueve arriba",entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posy--;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								//log_info(logs,"valor de quantum %d",q);
							}
							break;

						case '2':
							if (entre->posy < rows && !entre->fallecio) {
								//log_info(logs, "entrenador %c se mueve abajo", entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posy++;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								//log_info(logs,"valor de quantum %d",q);
							}
							break;

						case '4':
							if (entre->posx > 1 && !entre->fallecio) {
								//log_info(logs, "entrenador %c se mueve a la izquierda", entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posx--;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);
								q--;
								//log_info(logs,"valor de quantum %d",q);
							}
							break;
						case '6':
							if (entre->posx < cols && !entre->fallecio) {
								//log_info(logs, "entrenador % c se mueve a la derecha", entre->simbolo);
								usleep(datosMapa->retardoQ);
								entre->posx++;
								MoverPersonaje(items, entre->simbolo,
										entre->posx, entre->posy);
								nivel_gui_dibujar(items, argument);

								q--;
								//log_info(logs, "valor de quantum %d", q);
							}
							break;

						}
					}

					if (acto == '9' && !entre->fallecio) {

						//usleep(datosMapa->retardoQ);
						q = 0;
						bloqueo = 1;

						pokimons* p;

						//bool(poki*)
						//log_info(logs, "Extrajo un bloqueado");
						bool esLaPokenest(pokimons *parametro1) {
							return entre->pokenestAsignado
									== parametro1->pokinest;
						}
						//log_info(logs, "Ahora busca un poki");
						p = list_find(pokemons, (void*) esLaPokenest);

						int saux;
						int captu = 0;
						for (saux = 0; saux < list_size(p->listaPokemons);
								saux++) {
							metaDataPokemon* pokem;
							//log_info(logs,"Extrae lista de pokemon");
							pokem = list_get(p->listaPokemons, saux);
							//log_info(logs, "Saca un pokemon de la lista poki");
							//log_info(logs, "%s", pokem->especie);
							//log_info(logs, "%d", pokem->nivel);
							//log_info(logs, "%d", pokem->estaOcupado);
							if (!pokem->estaOcupado) {
								bloq* e;
								bool esBloq(bloq* param) {
									return param->pokenest == pokem->especie[0];
								}
								e = list_find(listaContenedora, (void*) esBloq);
								queue_push(e->colabloq, entre);
								captu = 1;
								//sem_post(&(e->sembloq));
								sem_post(&(e->sem2));
							}

						}
						if (!captu) {
							tabla* tab;
							bloq* e;
							bool esBloq(bloq* param) {
								return param->pokenest
										== entre->pokenestAsignado;
							}
							bool esLaPokenest2(tabla* a) {
								return entre->pokenestAsignado == a->pokenest;
							}
							e = list_find(listaContenedora, (void*) esBloq);
							tab = list_find(entre->solicitud,
									(void*) esLaPokenest2);
							tab->valor++;
							entre->entroBloqueados = 1;
							queue_push(e->colabloq, entre);
							//	sem_post(&(e->sembloq));
							sem_post(&(e->sem2));
						}

						/*						usleep(datosMapa->retardoQ);
						 //banderin = 0;
						 bloqueo = 1;
						 queue_push(colaBloqueados, entre);
						 sem_post(&sem_Bloqueados);
						 */
					}
				}
			}

			if (entre->fallecio) {
				log_info(logs, "Ahora lo mata en planificador");
				matar(entre);
				//entre->fallecio=0;

			}

			if (!bloqueo && !entre->fallecio) {
				usleep(datosMapa->retardoQ);
				//q = datosMapa->quantum;
				queue_push(colaListos, entre);
				sem_post(&sem_Listos);
			}

		}

////////////////////////////////////////////////////////EMPIEZA SRDF

		if (!strcmp(datosMapa->algoritmo, "SRDF")) {
			log_info(logs, "entre a srdf");

			t_list* listaAux = list_create();

			entrenador* ent1;
			entrenador* ent;

			while (!queue_is_empty(colaListos)) {
				ent = queue_pop(colaListos);
				list_add(listaAux, ent);
			}

			list_sort(listaAux, (void*) esMasCerca);

			int tamanioLista;
			tamanioLista = list_size(listaAux);
			int i;
			for (i = 0; i < tamanioLista; i++) {

				ent = list_get(listaAux, i);
				queue_push(colaListos, ent);
			}
			int bloqueo = 0;
			int banderin = 1;
			ent1 = (entrenador*) queue_pop(colaListos);

			int acto;

			acto = (int) queue_peek(ent1->colaAccion);

			//log_info(logs,"funca3");

			if (isalpha(acto)) {
				queue_pop(ent1->colaAccion);
				int ka;
				for (ka = 0; ka < list_size(pokenests); ka++) {
					datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
					if (datosPokenest->caracterPokeNest[0] == acto) {

						log_info(logs, "antes del send %s",
								datosPokenest->posicion);
						int pedo;
						pedo = send(
								(clientesActivos[ent1->numeroCliente]).socket,
								datosPokenest->posicion, 5, 0);

						log_info(logs, "Se envio coordenadas: %d", pedo);

						char** posicionPoke;
						posicionPoke = string_split(datosPokenest->posicion,
								";");

						ent1->posPokex = atoi(posicionPoke[0]);
						ent1->posPokey = atoi(posicionPoke[1]);
						ent1->pokenestAsignado =
								datosPokenest->caracterPokeNest[0];
						ent1->flagLeAsignaronPokenest = 1;

						ka = list_size(pokenests);

						log_info(logs, "la posicion es  %d y %d",
								ent1->posPokex, ent1->posPokey);
						log_info(logs, "el acto es  %d o %c ", acto, acto);

					}
				}

			}

			//8 es 56, 2 es 50, 4 es 52, 6 es 54

			if (isdigit(acto)) {
				while (banderin && (!(ent1->fallecio))
						&& queue_size(ent1->colaAccion)) {
					acto = (int) queue_pop(ent1->colaAccion);
					log_info(logs, "entre como un campeon");
					if (acto == '2' || acto == '4' || acto == '6'
							|| acto == '8') {

						usleep(70000);
						switch (acto) {

						case '8':
							if (ent1->posy > 1 && !ent1->fallecio) {
								log_info(logs, "mueva arriba");

								ent1->posy--;
								MoverPersonaje(items, ent1->simbolo, ent1->posx,
										ent1->posy);
								nivel_gui_dibujar(items, argument);

							}
							break;

						case '2':
							if (ent1->posy < rows && !ent1->fallecio) {
								log_info(logs, "mueva abajo");

								ent1->posy++;
								MoverPersonaje(items, ent1->simbolo, ent1->posx,
										ent1->posy);
								nivel_gui_dibujar(items, argument);

							}
							break;

						case '4':
							if (ent1->posx > 1 && !ent1->fallecio) {
								log_info(logs, "mueva izquierda");

								ent1->posx--;
								MoverPersonaje(items, ent1->simbolo, ent1->posx,
										ent1->posy);
								nivel_gui_dibujar(items, argument);

							}
							break;
						case '6':
							if (ent1->posx < cols && !ent1->fallecio) {
								log_info(logs, "mueva derecha");

								ent1->posx++;
								MoverPersonaje(items, ent1->simbolo, ent1->posx,
										ent1->posy);
								nivel_gui_dibujar(items, argument);

							}
							break;

						}

					}
					if (acto == '9' && !ent1->fallecio) {

						//usleep(datosMapa->retardoQ);
						q = 0;
						bloqueo = 1;

						pokimons* p;

						//bool(poki*)
						//log_info(logs, "Extrajo un bloqueado");
						bool esLaPokenest(pokimons *parametro1) {
							return ent1->pokenestAsignado
									== parametro1->pokinest;
						}
						//log_info(logs, "Ahora busca un poki");
						p = list_find(pokemons, (void*) esLaPokenest);

						int saux;
						int captu = 0;
						for (saux = 0; saux < list_size(p->listaPokemons);
								saux++) {
							metaDataPokemon* pokem;
							//log_info(logs,"Extrae lista de pokemon");
							pokem = list_get(p->listaPokemons, saux);
							//log_info(logs, "Saca un pokemon de la lista poki");
							//log_info(logs, "%s", pokem->especie);
							//log_info(logs, "%d", pokem->nivel);
							//log_info(logs, "%d", pokem->estaOcupado);
							if (!pokem->estaOcupado) {
								bloq* e;
								bool esBloq(bloq* param) {
									return param->pokenest == pokem->especie[0];
								}
								e = list_find(listaContenedora, (void*) esBloq);
								queue_push(e->colabloq, ent1);
								captu = 1;
								//sem_post(&(e->sembloq));
								sem_post(&(e->sem2));
							}

						}
						if (!captu) {
							tabla* tab;
							bloq* e;
							bool esBloq(bloq* param) {
								return param->pokenest == ent1->pokenestAsignado;
							}
							bool esLaPokenest2(tabla* a) {
								return ent1->pokenestAsignado == a->pokenest;
							}
							e = list_find(listaContenedora, (void*) esBloq);
							tab = list_find(ent1->solicitud,
									(void*) esLaPokenest2);
							tab->valor++;
							ent1->entroBloqueados = 1;
							queue_push(e->colabloq, ent1);
							//	sem_post(&(e->sembloq));
							sem_post(&(e->sem2));
						}

						/*						usleep(datosMapa->retardoQ);
						 //banderin = 0;
						 bloqueo = 1;
						 queue_push(colaBloqueados, entre);
						 sem_post(&sem_Bloqueados);
						 */
					}

				}
			}

			if (ent1->fallecio) {
				log_info(logs, "Ahora lo mata en planificador");
				matar(ent1);
				//entre->fallecio=0;

			}

			if (!bloqueo && !ent1->fallecio && !bloqueo) {
				usleep(datosMapa->retardoQ);
				//q = datosMapa->quantum;
				queue_push(colaListos, ent1);
				sem_post(&sem_Listos);
			}

		}

	}

}

void bloqui(void* stru) {
	bloq* strub = (bloq*) stru;
	while (1) {

		sem_wait(&strub->sem2);
		sem_wait(&strub->sembloq);
		entrenador* ent1;
		pokimons* poki;
		ent1 = queue_pop(strub->colabloq);
		if (!ent1->fallecio) {

			//log_info(logs, "Extrajo un bloqueado");
			bool esLaPokenest(pokimons *parametro1) {
				return ent1->pokenestAsignado == parametro1->pokinest;
			}
			//log_info(logs, "Ahora busca un poki");
			pthread_mutex_lock(&mu);
			poki = list_find(pokemons, (void*) esLaPokenest);
			pthread_mutex_unlock(&mu);
			//log_info(logs, "Saca un poki");
			//log_info(logs, "%c", poki->pokinest);
			int auxi67;

			bool esLaPokenest2(tabla* a) {
				return ent1->pokenestAsignado == a->pokenest;
			}

			for (auxi67 = 0; auxi67 < list_size(poki->listaPokemons);
					auxi67++) {
				metaDataPokemon* pokem;
				//log_info(logs,"Extrae lista de pokemon");
				pokem = list_get(poki->listaPokemons, auxi67);
				//log_info(logs, "Saca un pokemon de la lista poki");
				//log_info(logs, "%s", pokem->especie);
				//log_info(logs, "%d", pokem->nivel);
				//log_info(logs, "%d", pokem->estaOcupado);
				if (!pokem->estaOcupado) {

					pthread_mutex_lock(&mutexEnvio);
					char* nombreAux = pokem->nombreArch;
					char** nombreSinDatAux = string_split(nombreAux, ".");
					char* nombreSinDAT = nombreSinDatAux[0];

					//log_info(logs, "Creo el nombre sin DAT: %s", nombreSinDAT);
					int protocolo = 1;

					int tamanioCosaUno = sizeof(char) * strlen(pokem->especie);
					//log_info(logs,"tamanio especie %d", tamanioCosaUno);
					int tamanioCosaDos = sizeof(char) * strlen(nombreSinDAT);
					//log_info(logs,"tamanio sin dat %d",tamanioCosaDos);

					int auxilia = pokem->nivel;
					void* miBuffer = malloc(
							(4 * sizeof(int)) + tamanioCosaUno
									+ tamanioCosaDos);
					memcpy(miBuffer, &protocolo, sizeof(int));
					memcpy(miBuffer + sizeof(int), &tamanioCosaUno,
							sizeof(int));
					memcpy(miBuffer + (2 * sizeof(int)), &tamanioCosaDos,
							sizeof(int));

					//log_info(logs, "metio bien tamaños en buffer");

					//convertir Ruta de especie, nombreSinDat
					char* caracterNulo = string_new();
					caracterNulo = "\0";
					string_append(&pokem->especie, caracterNulo);
					string_append(&nombreSinDAT, caracterNulo);

					memcpy(miBuffer + (3 * sizeof(int)), pokem->especie,
							tamanioCosaUno); //VERIFICA DESPUES
					//log_info(logs, "mete bien especie:%s", pokem->especie);
					memcpy(miBuffer + (3 * sizeof(int)) + tamanioCosaUno,
							nombreSinDAT, tamanioCosaDos); //VERIFICAR DESPUES
					//log_info(logs, "mete bien nombreSinDat:%s", nombreSinDAT);
					memcpy(
							miBuffer + (3 * sizeof(int)) + tamanioCosaUno
									+ tamanioCosaDos, &auxilia, sizeof(int)); //VERIFICAR DESPUES
					//log_info(logs, "mete bien nivel:%d", pokem->nivel);
					//log_info(logs, "mete bien mierda en buffer");

					int e;
					e = send((clientesActivos[ent1->numeroCliente]).socket,
							miBuffer,
							(4 * sizeof(int)) + tamanioCosaUno + tamanioCosaDos,
							0);

					//log_info(logs, "envio la mierda %d", e);

					//free(caracterNulo); // si rompe, sacarlo
					free(miBuffer);
					auxi67 = list_size(poki->listaPokemons);

					pokem->estaOcupado = 1;

					//log_info(logs,"ahora el pokem esta ocupado %d",pokem->estaOcupado);

					tabla* t;
					tabla* dispo1;

					t = list_find(ent1->asignados, (void*) esLaPokenest2);
					dispo1 = (tabla*) list_find(disponibles,
							(void*) esLaPokenest2);

					t->valor++;
					dispo1->valor--;

					list_add(ent1->pokemones, pokem);

					//log_info(logs, "llego a bloqueados");

					restarRecurso(items, poki->pokinest);
					nivel_gui_dibujar(items, nombreMapa);

					queue_push(colaListos, ent1);
					sem_post(&sem_Listos);

					pthread_mutex_unlock(&mutexEnvio);

					//log_info(logs,"pushea a entrenador a listos");
				}

			}

			/*				if (!ent1->entroBloqueados && !ent1->fallecio) {
			 //log_info(logs,"no hay recurso disponible para %c",ent1->simbolo);
			 tabla* tab;
			 tab = list_find(ent1->solicitud, (void*) esLaPokenest2);
			 tab->valor++;
			 ent1->entroBloqueados = 1;
			 queue_push(colaBloqueados, ent1);

			 }       */

		} else {
			pthread_mutex_lock(&mutexMuerte);
			log_info(logs, "mata a entrenador en bloqueados");
			matar(ent1);
			pthread_mutex_unlock(&mutexMuerte);
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
	listaContenedora = list_create();
	deadlocks = list_create();
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

	//hilo de bloqueados
	int auxili3;
	for (auxili3 = 0; auxili3 < list_size(listaContenedora); auxili3++) {
		bloq* stru;
		stru = list_get(listaContenedora, auxili3);
		pthread_create(&(stru->hilobloq), NULL, (void*) bloqui, (void*) stru);
	}

	//hilo deteccion de deadlock
	pthread_create(&hiloDeadlock, NULL, (void*) banquero, NULL);

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
