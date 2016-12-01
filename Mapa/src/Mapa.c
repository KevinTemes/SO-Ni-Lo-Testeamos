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

//variables globales
extern t_infoCliente clientesActivos[2048];
char* nombreMapa;
char* configMapa;

//variables del mapa
int nE = 0; //numero entrenador
metaDataComun* datosMapa; //MATADO
metaDataComun* datosMapa2;
int rows; // nro de filas
int cols; // nro de columnas

//log
t_log* logs;

//listas
t_list* pokemons; //MATADO
t_list* pokenests; //MATADO
t_list* disponibles; //MATADO
t_list* items;
t_list* entrenadoresEnCurso; //MATADO
t_list* colaDeListosImp;
t_list* colaDeBloqImp;
t_list* listaContenedora; //MATADO
t_list* deadlocks; //MATADO
//colas
t_queue* colaListos;

//semaforos
sem_t sem_Listos;
sem_t sem_Bloqueados;
sem_t sem_quantum;
sem_t sem_llego;

int numHilos;

//declara hilos
pthread_t hiloDePlanificador;
pthread_t hiloDeadlock;
pthread_t hiloDeBloqueados;
pthread_t hiloAtenderConexiones[1024];
pthread_attr_t attr;

pthread_mutex_t pokemi = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexEnvio = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMuerte = PTHREAD_MUTEX_INITIALIZER;

//deadlock
void banquero() {

	while (1) {
		usleep(datosMapa2->tiempoChequeoDeadlock * 1000);

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
					}

				}
			}

			int auu;
			for (auu = 0; auu < list_size(entrenadoresEnCurso); auu++) {
				entrenador* an;
				tabla* ta;
				tabla* b;
				an = list_get(entrenadoresEnCurso, auu);

				if (!an->fallecio) {
					if (!an->estaMarcado) {
						//log_info(logs, "entrenador %c no esta marcado",an->simbolo);
						int j;
						int val = 1;
						for (j = 0; j < list_size(an->solicitud) && val; j++) {
							ta = list_get(an->solicitud, j);
							b = list_get(vectorT, j);
							val = ta->valor <= b->valor;
						}
						if (val) {
							tabla* fa;
							tabla* fe;
							an->estaMarcado = 1;
							//log_info(logs, "marca a %c", an->simbolo);
							int x;
							for (x = 0; x < list_size(an->asignados); x++) {
								fa = list_get(an->asignados, x);
								fe = list_get(vectorT, x);
								fe->valor = fa->valor + fe->valor;
								auu = 0;
							}

						}

					}
				}

			}
			int pokixi;
			t_list* entrenadoresEnDeadlock;
			entrenadoresEnDeadlock = list_create();
			//	log_info(logs, "crea lista de entrenadores en deadlock");
			for (pokixi = 0; pokixi < list_size(entrenadoresEnCurso);
					pokixi++) {
				//		log_info(logs, "se mete al for pokixi");
				entrenador* entreneitor;
				//
				//	if(entreneitor->fallecio == 0){
				entreneitor = list_get(entrenadoresEnCurso, pokixi);
				//			log_info(logs, "entrenador %c esta marcado? %d",
				//				entreneitor->simbolo, entreneitor->estaMarcado);
				if (!(entreneitor->estaMarcado)) {

					list_add(entrenadoresEnDeadlock, entreneitor);
				}
			}

			//LISTA DE LISTAS DE ENTRENADORES EN DEADLOCK

			while (list_size(entrenadoresEnDeadlock)) {
				entrenador* hp;
				hp = list_remove(entrenadoresEnDeadlock, 0);
				bool entrencontrado(entrenador* parametro) {
					tabla* tab;
					tabla* teb;
					int h3;
					int x = 0;
					for (h3 = 0; h3 < list_size(parametro->asignados); h3++) {
						tab = list_get(parametro->asignados, h3);
						teb = list_get(hp->solicitud, h3);
						if (tab->valor >= teb->valor) {
							x = 1;
						}

					}
					return x;
				}

				entrenador* es = hp;
				t_list* listin = list_create();
				list_add(listin, es);
				list_add(deadlocks, listin);

				while (list_find(entrenadoresEnDeadlock, (void*) entrencontrado)!= NULL) {
					entrenador* jota;
					jota = list_remove_by_condition(entrenadoresEnDeadlock,(void*) entrencontrado);
					hp = jota;
					list_add(listin, jota);
				}

			}


			if(list_size(deadlocks)){
				int comproba=0;
				t_list* listitw;
				int eaf;
				for(eaf=0; eaf<list_size(deadlocks);eaf++){
					listitw = list_get(deadlocks,eaf);
					if(list_size(listitw)>1){
						comproba=1;
					}
				}
				if(comproba){
					log_info(logs,"Hubo deadlock, tablas usadas:");
					log_info(logs,"Asignacion:");
					int auxileg;
					for(auxileg=0;auxileg<list_size(entrenadoresEnCurso);auxileg++){
						entrenador* entw;
						entw = list_get(entrenadoresEnCurso,auxileg);
						log_info(logs,"Entrenador %c",entw->simbolo);
						int casielUlti;
						for(casielUlti=0;casielUlti<list_size(entw->asignados);casielUlti++){
							tabla* tablilla;
							tablilla = list_get(entw->asignados,casielUlti);
							log_info(logs,"%c%d",tablilla->pokenest,tablilla->valor);
						}
					}
					log_info(logs,"Solicitud:");
					int auxilog;
										for(auxilog=0;auxilog<list_size(entrenadoresEnCurso);auxilog++){
											entrenador* entw;
											entw = list_get(entrenadoresEnCurso,auxilog);
											log_info(logs,"Entrenador %c",entw->simbolo);
											int casielUlti;
											for(casielUlti=0;casielUlti<list_size(entw->asignados);casielUlti++){
												tabla* tablilla;
												tablilla = list_get(entw->asignados,casielUlti);
												log_info(logs,"%c%d",tablilla->pokenest,tablilla->valor);
											}
										}
				}
			}

			if (list_size(deadlocks)) {
				t_list* liste;
				int ef;
				for (ef = 0; ef < list_size(deadlocks); ef++) {
					liste = list_get(deadlocks, ef);
					if (list_size(liste) > 1) {
						int efefe;
						for (efefe = 0; efefe < list_size(liste); efefe++) {
							entrenador* pef;
							pef = list_get(liste, efefe);
							log_info(logs, "entrenador %c en deadlock #%d ",
									 pef->simbolo,ef);
						/*	log_info(logs,"sus campos de asignacion y solicitud");
							log_info(logs,"asignacion:");
							int paxu;
							for(paxu=0;paxu<list_size(pef->asignados);paxu++){
								tabla* asigneishon;
								asigneishon = list_get(pef->asignados,paxu);
								log_info(logs,"%c%d",asigneishon->pokenest,asigneishon->valor);
							}
							int poxu;
							for(poxu=0;poxu<list_size(pef->solicitud);poxu++){
								tabla* asignoishon;
								asignoishon = list_get(pef->solicitud,poxu);
								log_info(logs,"%c%d",asignoishon->pokenest,asignoishon->valor);
							}
							//			log_info(logs, "%d", efe); */
						}
					} else {
						entrenador* pof;
						pof = list_get(liste, 0);
						log_info(logs, "entrenador %c en inanicion",
								pof->simbolo);
					}

				}
			}

			if (list_size(deadlocks)) {

				bool llegoPrimero(entrenador* a, entrenador* b) {
					return a->numeroLlegada < b->numeroLlegada;
				}
				int ej;
				for (ej = 0; ej < list_size(deadlocks); ej++) {
					t_list* listie;
					listie = list_get(deadlocks, ej);
					list_sort(listie, (void*) llegoPrimero);
				}
//				log_info(logs,f
				//					"entrenadores ordenados, hora de la batalla pokemon");

				if (datosMapa2->batalla) {
					int yotromas;
					for (yotromas = 0; yotromas < list_size(deadlocks);
							yotromas++) {

						t_list* listota;
						listota = list_get(deadlocks, yotromas);

						if (list_size(listota) > 1) {

							int elultiaux;
							for (elultiaux = 0; elultiaux < list_size(listota);
									elultiaux++) {
								entrenador* elent;
								elent = list_get(listota, elultiaux);
								int accione = 3;
								//int efe;
								send(
										(clientesActivos[elent->numeroCliente]).socket,
										&accione, sizeof(int), 0);

							}
							int ultimo;
							for (ultimo = 0; ultimo < list_size(listota);
									ultimo++) {
								sem_wait(&sem_llego);
							}

							while (list_size(listota) > 1) {

								t_pkmn_factory* facto = create_pkmn_factory();
								t_pokemon* pokegold;
								t_pokemon* pokesilver;
								t_pokemon* pokeperdedor;

								entrenador* gold;
								entrenador* silver;
								gold = list_get(listota, 0);
								silver = list_get(listota, 1);

								if (!gold->fallecio && !silver->fallecio) {

									log_info(logs, "%c peleara con %c",
											gold->simbolo, silver->simbolo);
									pokegold = create_pokemon(facto,
											(gold->pokePeleador)->especie,
											(gold->pokePeleador)->nivel);
									log_info(logs,
											"pokemon del entrenador %c es %s y su nivel %d",
											gold->simbolo,
											(gold->pokePeleador)->especie,
											(gold->pokePeleador)->nivel);
									pokesilver = create_pokemon(facto,
											(silver->pokePeleador)->especie,
											(silver->pokePeleador)->nivel);
									log_info(logs,
											"pokemon del entrenador %c es %s y su nivel %d",
											silver->simbolo,
											(silver->pokePeleador)->especie,
											(silver->pokePeleador)->nivel);
									pokeperdedor = pkmn_battle(pokegold,
											pokesilver);
									log_info(logs, "Perdedor de tipo %s",
											pkmn_type_to_string(
													pokeperdedor->type));
									int accionar = 0;
									if (!strcmp(pokegold->species,
											pokeperdedor->species)
											&& (pokegold->level
													== pokeperdedor->level)) {
										send(
												clientesActivos[silver->numeroCliente].socket,
												&accionar, sizeof(int), 0);
										list_remove(listota, 1);
										log_info(logs,
												"entrenador %c victorioso",
												silver->simbolo);
									} else {
										send(
												clientesActivos[gold->numeroCliente].socket,
												&accionar, sizeof(int), 0);
										list_remove(listota, 0);
										log_info(logs,
												"entrenador %c victorioso",
												gold->simbolo);
									}
								}
							}
							entrenador* muerto;
							muerto = list_get(listota, 0);
							log_info(logs,
									"Seleccionado el entrenador %c como victima",
									muerto->simbolo);
							int auxiliar23;
							for (auxiliar23 = 0;
									auxiliar23 < list_size(muerto->solicitud);
									auxiliar23++) {
								tabla* solici23;
								tabla* asig23;
								solici23 = list_get(muerto->solicitud,
										auxiliar23);
								asig23 = list_get(muerto->asignados,
										auxiliar23);
								solici23->valor = 0;
								asig23->valor = 0;
							}
							int accion = 7;
							send(clientesActivos[muerto->numeroCliente].socket,
									&accion, sizeof(int), 0);

						}
					}
				}

				list_destroy(entrenadoresEnDeadlock);
				int otromas;
				for (otromas = 0; otromas < list_size(deadlocks); otromas++) {
					list_remove_and_destroy_element(deadlocks, otromas,
							(void*) free);
				}
			}
			int auxi23;
			for (auxi23 = 0; auxi23 < list_size(entrenadoresEnCurso);
					auxi23++) {
				entrenador* entri;
				entri = list_get(entrenadoresEnCurso, auxi23);
				entri->estaMarcado = 0;
			}
			list_destroy_and_destroy_elements(vectorT, (void*) free);
			//log_info(logs,"se elimina vector auxiliar");
		}
	}

}

//arranque de planificacion
void planificador(void* argu) {

	char* argument = (char*) argu;

	while (1) {
		sem_wait(&sem_Listos); //semaforo de nuevos bloqueando que se saque un ent si la cola esta vacia

		//	usleep(datosMapa2->retardoQ*1000);

		int q = datosMapa2->quantum;

/////////////////////////////////////////ROUND ROBIN/////////////////////////////////////////////////////////////

		if (!strcmp(datosMapa2->algoritmo, "RR")) {

			//log_info(logs,"RR");
			int acto;
			entrenador* entre;

			int bloqueo = 0;

			if (queue_size(colaListos)) {
				entre = queue_pop(colaListos);

				if (queue_size(entre->colaAccion)) {

					log_info(logs,
							"Extrae entrenador %c de la cola de listos y ejecuta",
							entre->simbolo);

				}

				while (q
						&& (!(entre->fallecio) && queue_size(entre->colaAccion))) {

					usleep(datosMapa2->retardoQ * 1000);

					acto = (int) queue_pop(entre->colaAccion);
					//log_info(logs,"funca3");

					if (isalpha(acto) && !entre->fallecio) {
						int ka;
						for (ka = 0; ka < list_size(pokenests); ka++) {
							metaDataPokeNest* datosPokenest;
							datosPokenest = (metaDataPokeNest*) list_get(
									pokenests, ka);
							if (datosPokenest->caracterPokeNest[0] == acto) {

								//log_info(logs, "antes del send %s",datosPokenest->posicion);

								char** posicionPoke;
								posicionPoke = string_split(
										datosPokenest->posicion, ";");

								entre->posPokex = atoi(posicionPoke[0]);
								entre->posPokey = atoi(posicionPoke[1]);
								entre->pokenestAsignado =
										datosPokenest->caracterPokeNest[0];
								entre->flagLeAsignaronPokenest = 1;

								ka = list_size(pokenests);

								//serializo las coordenadas
								int tamanioTotal = 2 * sizeof(int);
								void* miBuffer = malloc(tamanioTotal);

								memcpy(miBuffer, &(entre->posPokex),
										sizeof(int));
								memcpy(miBuffer + sizeof(int),
										&(entre->posPokey), sizeof(int));

								// lo mando
								send(
										(clientesActivos[entre->numeroCliente]).socket,
										miBuffer, tamanioTotal, 0);

								free(miBuffer);

							}
						}

					}

					//8 es 56, 2 es 50, 4 es 52, 6 es 54

					if (isdigit(acto) && !(entre->fallecio)) {
						if (acto == '2' || acto == '4' || acto == '6'
								|| acto == '8') {

							switch (acto) {

							case '8':
								if (entre->posy > 1 && !entre->fallecio) {
									//usleep(datosMapa2->retardoQ*1000);
									entre->posy--;
									MoverPersonaje(items, entre->simbolo,
											entre->posx, entre->posy);
									nivel_gui_dibujar(items, argument);
									q--;
									log_info(logs,
											"valor de quantum para ejecucion de %c: %d ",
											entre->simbolo, q);
								}
								break;

							case '2':
								if (entre->posy < rows && !entre->fallecio) {
									//usleep(datosMapa2->retardoQ*1000);
									entre->posy++;
									MoverPersonaje(items, entre->simbolo,
											entre->posx, entre->posy);

									nivel_gui_dibujar(items, argument);
									q--;
									log_info(logs,
											"valor de quantum para ejecucion de %c: %d ",
											entre->simbolo, q);
								}
								break;

							case '4':
								if (entre->posx > 1 && !entre->fallecio) {
									//usleep(datosMapa2->retardoQ*1000);
									entre->posx--;
									MoverPersonaje(items, entre->simbolo,
											entre->posx, entre->posy);
									nivel_gui_dibujar(items, argument);
									q--;
									log_info(logs,
											"valor de quantum para ejecucion de %c: %d ",
											entre->simbolo, q);
								}
								break;
							case '6':
								if (entre->posx < cols && !entre->fallecio) {
									//usleep(datosMapa2->retardoQ*1000);
									entre->posx++;
									MoverPersonaje(items, entre->simbolo,
											entre->posx, entre->posy);
									nivel_gui_dibujar(items, argument);

									q--;
									log_info(logs,
											"valor de quantum para ejecucion de %c: %d ",
											entre->simbolo, q);
								}
								break;

							}
						}

						if (acto == '9' && !entre->fallecio) {

							//usleep(datosMapa2->retardoQ*1000);
							q = 0;
							bloqueo = 1;

							entre->flagLeAsignaronPokenest = 0;

							pokimons* p;

							bool esLaPokenest(pokimons *parametro1) {
								return entre->pokenestAsignado
										== parametro1->pokinest;
							}
							p = list_find(pokemons, (void*) esLaPokenest);

							int saux;
							int captu = 0;
							for (saux = 0;
									saux < list_size(p->listaPokemons)
											&& captu == 0; saux++) {
								metaDataPokemon* pokem;
								pokem = list_get(p->listaPokemons, saux);

								if (!pokem->estaOcupado) {
									bloq* e;
									bool esBloq(bloq* param) {
										return param->pokenest
												== pokem->especie[0];
									}
									e = list_find(listaContenedora,
											(void*) esBloq);
									log_info(logs,
											"entrenador %c va a la cola de bloqueados,pokenest %c",
											entre->simbolo, e->pokenest);

									bool esesnt(entrenador* ap) {
										return ap->simbolo == entre->simbolo;
									}
									list_remove_by_condition(colaDeListosImp,
											(void*) esesnt);
									log_info(logs,
											"entrenadores en cola de listos:");
									int auxilie;
									for (auxilie = 0;
											auxilie < list_size(colaDeListosImp);
											auxilie++) {
										entrenador* entprint;
										entprint = list_get(colaDeListosImp,
												auxilie);
										log_info(logs, "entrenador %c",
												entprint->simbolo);
									}

									list_add(colaDeBloqImp, entre);
									log_info(logs,
											"entrenadores en cola de bloqueados:");
									int auxiliere;
									for (auxiliere = 0;
											auxiliere < list_size(colaDeBloqImp);
											auxiliere++) {
										entrenador* entprint;
										entprint = list_get(colaDeBloqImp,
												auxiliere);
										log_info(logs, "entrenador %c",
												entprint->simbolo);
									}

									queue_push(e->colabloq, entre);
									captu = 1;
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
									return entre->pokenestAsignado
											== a->pokenest;
								}
								e = list_find(listaContenedora, (void*) esBloq);
								tab = list_find(entre->solicitud,
										(void*) esLaPokenest2);

								tab->valor = tab->valor + 1;

								entre->sumo = 1;
								//entre->entroBloqueados = 1;
								log_info(logs,
										"entrenador %c va a la cola de bloqueados,pokenest %c",
										entre->simbolo, e->pokenest);
								bool esesnt(entrenador* ap) {
									return ap->simbolo == entre->simbolo;
								}
								list_remove_by_condition(colaDeListosImp,
										(void*) esesnt);
								log_info(logs,
										"entrenadores en cola de listos:");
								int auxilie;
								for (auxilie = 0;
										auxilie < list_size(colaDeListosImp);
										auxilie++) {
									entrenador* entprint;
									entprint = list_get(colaDeListosImp,
											auxilie);
									log_info(logs, "entrenador %c",
											entprint->simbolo);
								}

								list_add(colaDeBloqImp, entre);
								log_info(logs,
										"entrenadores en cola de bloqueados:");
								int auxiliere;
								for (auxiliere = 0;
										auxiliere < list_size(colaDeBloqImp);
										auxiliere++) {
									entrenador* entprint;
									entprint = list_get(colaDeBloqImp,
											auxiliere);
									log_info(logs, "entrenador %c",
											entprint->simbolo);
								}

								queue_push(e->colabloq, entre);
								sem_post(&(e->sem2));
							}

						}
					}
				}

				if (entre->fallecio) {
					log_info(logs, "%c muere", entre->simbolo);
					matar(entre);
					//entre->fallecio=0;

				}

				if (!bloqueo && !entre->fallecio) {
					//usleep(datosMapa2->retardoQ*1000);
					//q = datosMapa->quantum;
					queue_push(colaListos, entre);
					sem_post(&sem_Listos);

				}
			}
		}

////////////////////////////////////////////////////////EMPIEZA SRDF////////////////////////////

		if (!strcmp(datosMapa2->algoritmo, "SRDF")) {

			//log_info(logs,"SRDF");

			t_list* listaAux = list_create();

			entrenador* ent1;

			while (queue_size(colaListos)) {
				entrenador* ent;
				ent = queue_pop(colaListos);
				list_add(listaAux, ent);
			}

			list_sort(listaAux, (void*) esMasCerca);

			int tamanioLista;
			tamanioLista = list_size(listaAux);
			int i;
			for (i = 0; i < tamanioLista; i++) {
				entrenador* ent;
				ent = list_get(listaAux, i);
				queue_push(colaListos, ent);
			}

			list_destroy(listaAux);

			int bloqueo = 0;
			int banderin = 1;

			if (queue_size(colaListos)) {

				ent1 = (entrenador*) queue_pop(colaListos);

				int acto;

				acto = (int) queue_peek(ent1->colaAccion);

				if (isalpha(acto)) {
					log_info(logs,
							"Se extrae al entrenador %c de la cola de listos para asignar pokenest",
							ent1->simbolo);
					queue_pop(ent1->colaAccion);
					int ka;
					for (ka = 0; ka < list_size(pokenests); ka++) {
						metaDataPokeNest* datosPokenest;
						datosPokenest = (metaDataPokeNest*) list_get(pokenests,
								ka);
						if (datosPokenest->caracterPokeNest[0] == acto) {

							//log_info(logs, "antes del send %s",datosPokenest->posicion);

							char** posicionPoke;
							posicionPoke = string_split(datosPokenest->posicion,
									";");

							ent1->posPokex = atoi(posicionPoke[0]);
							ent1->posPokey = atoi(posicionPoke[1]);
							ent1->pokenestAsignado =
									datosPokenest->caracterPokeNest[0];
							ent1->flagLeAsignaronPokenest = 1;

							ka = list_size(pokenests);

							//serializo las coordenadas
							int tamanioTotal = 2 * sizeof(int);
							void* miBuffer = malloc(tamanioTotal);

							memcpy(miBuffer, &(ent1->posPokex), sizeof(int));
							memcpy(miBuffer + sizeof(int), &(ent1->posPokey),
									sizeof(int));

							// lo mando
							send((clientesActivos[ent1->numeroCliente]).socket,
									miBuffer, tamanioTotal, 0);

							free(miBuffer);
							//log_info(logs, "Se envio coordenadas: %d", pedo);

						}
					}

				}

				//8 es 56, 2 es 50, 4 es 52, 6 es 54

				if (isdigit(acto)) {

					log_info(logs,
							"Se extrae al entrenador %c de la cola de listos para ejecucion hasta que corresponda traslado a cola de bloqueados",
							ent1->simbolo);

					while (banderin && !(ent1->fallecio)
							&& queue_size(ent1->colaAccion)) {

						usleep(datosMapa2->retardoQ * 1000);

						acto = (int) queue_pop(ent1->colaAccion);

						if (acto == '2' || acto == '4' || acto == '6'
								|| acto == '8') {

							switch (acto) {

							case '8':
								if (ent1->posy > 1 && !ent1->fallecio) {
									//				log_info(logs, "mueva arriba");
									//usleep(datosMapa2->retardoQ*1000);

									ent1->posy--;
									MoverPersonaje(items, ent1->simbolo,
											ent1->posx, ent1->posy);
									nivel_gui_dibujar(items, argument);

								}
								break;

							case '2':
								if (ent1->posy < rows && !ent1->fallecio) {
									//				log_info(logs, "mueva abajo");
									//usleep(datosMapa2->retardoQ*1000);
									ent1->posy++;
									MoverPersonaje(items, ent1->simbolo,
											ent1->posx, ent1->posy);
									nivel_gui_dibujar(items, argument);

								}
								break;

							case '4':
								if (ent1->posx > 1 && !ent1->fallecio) {
									//				log_info(logs, "mueva izquierda");
									//usleep(datosMapa2->retardoQ*1000);
									ent1->posx--;
									MoverPersonaje(items, ent1->simbolo,
											ent1->posx, ent1->posy);
									nivel_gui_dibujar(items, argument);

								}
								break;
							case '6':
								if (ent1->posx < cols && !ent1->fallecio) {
									//				log_info(logs, "mueva derecha");
									//usleep(datosMapa2->retardoQ*1000);
									ent1->posx++;
									MoverPersonaje(items, ent1->simbolo,
											ent1->posx, ent1->posy);
									nivel_gui_dibujar(items, argument);

								}
								break;

							}

						}
						if (acto == '9' && !ent1->fallecio) {

							//usleep(datosMapa->retardoQ*1000);
							bloqueo = 1;
							banderin = 0;
							ent1->flagLeAsignaronPokenest = 0;

							pokimons* p;
							bool esLaPokenest(pokimons *parametro1) {
								return ent1->pokenestAsignado
										== parametro1->pokinest;
							}
							p = list_find(pokemons, (void*) esLaPokenest);

							int saux;
							int captu = 0;
							for (saux = 0;
									saux < list_size(p->listaPokemons)
											&& captu == 0; saux++) {
								metaDataPokemon* pokem;
								pokem = list_get(p->listaPokemons, saux);
								if (!pokem->estaOcupado) {
									bloq* e;
									bool esBloq(bloq* param) {
										return param->pokenest
												== pokem->especie[0];
									}
									e = list_find(listaContenedora,
											(void*) esBloq);
									log_info(logs,
											"entrenador %c va a la cola de bloqueados,pokenest %c",
											ent1->simbolo, e->pokenest);

									bool esesnt(entrenador* ap){
																		return ap->simbolo == ent1->simbolo;
																	}
																	list_remove_by_condition(colaDeListosImp, (void*)esesnt);
																	log_info(logs, "entrenadores en cola de listos:");
																						int auxilie;
																						for(auxilie=0;auxilie<list_size(colaDeListosImp);auxilie++){
																							entrenador* entprint;
																							entprint = list_get(colaDeListosImp,auxilie);
																							log_info(logs, "entrenador %c",entprint->simbolo);
																						}

																    list_add(colaDeBloqImp, ent1);
																    log_info(logs, "entrenadores en cola de bloqueados:");
																    int auxiliere;
																    for(auxiliere=0;auxiliere<list_size(colaDeBloqImp);auxiliere++){
																    	entrenador* entprint;
																		entprint = list_get(colaDeBloqImp,auxiliere);
																		log_info(logs, "entrenador %c",entprint->simbolo);
																    }


									queue_push(e->colabloq, ent1);
									captu = 1;
									sem_post(&(e->sem2));

								}

							}
							if (!captu) {
								tabla* tab;
								bloq* e;
								bool esBloq(bloq* param) {
									return param->pokenest
											== ent1->pokenestAsignado;
								}
								bool esLaPokenest2(tabla* a) {
									return ent1->pokenestAsignado == a->pokenest;
								}
								e = list_find(listaContenedora, (void*) esBloq);
								tab = list_find(ent1->solicitud,
										(void*) esLaPokenest2);
								tab->valor = tab->valor + 1;
								//ent1->entroBloqueados = 1;
								log_info(logs,
										"entrenador %c va a la cola de bloqueados,pokenest %c",
										ent1->simbolo, e->pokenest);


								bool esesnt(entrenador* ap){
																	return ap->simbolo == ent1->simbolo;
																}
																list_remove_by_condition(colaDeListosImp, (void*)esesnt);
																log_info(logs, "entrenadores en cola de listos:");
																					int auxilie;
																					for(auxilie=0;auxilie<list_size(colaDeListosImp);auxilie++){
																						entrenador* entprint;
																						entprint = list_get(colaDeListosImp,auxilie);
																						log_info(logs, "entrenador %c",entprint->simbolo);
																					}

															    list_add(colaDeBloqImp, ent1);
															    log_info(logs, "entrenadores en cola de bloqueados:");
															    int auxiliere;
															    for(auxiliere=0;auxiliere<list_size(colaDeBloqImp);auxiliere++){
															    	entrenador* entprint;
																	entprint = list_get(colaDeBloqImp,auxiliere);
																	log_info(logs, "entrenador %c",entprint->simbolo);
															    }


								queue_push(e->colabloq, ent1);

								sem_post(&(e->sem2));

							}
						}
					}
				}

				if (ent1->fallecio) {
					log_info(logs, "%c muere", ent1->simbolo);
					matar(ent1);

				}

				if (!bloqueo && !ent1->fallecio) {
					//	usleep(datosMapa2->retardoQ*1000);
					queue_push(colaListos, ent1);
					sem_post(&sem_Listos);
				}
			}
		}
	}

}

void bloqui(void* stru) {
	bloq* strub = (bloq*) stru;
	while (1) {

		sem_wait(&strub->sem2);
		entrenador* ent1;
		ent1 = queue_pop(strub->colabloq);
		//log_info(logs, "saca %c de la cola de %c", ent1->simbolo,strub->pokenest);

		int dal;
		sem_getvalue(&strub->sembloq, &dal);
		log_info(logs, "Pasa el sem wait, el siguiente vale %d", dal);

		//	usleep(datosMapa2->retardoQ*1000);
		//SOLUCION RUDIMENTARISISISISISISISISISIISISISISISISIISISISISISIISISISISISIISISMA

		if (dal == 0) {

			pokimons* a;
			bool esLaPokenest(pokimons *parametro1) {
				return strub->pokenest == parametro1->pokinest;
			}
			a = list_find(pokemons, (void*) esLaPokenest);
			int ew;
			for (ew = 0; ew < list_size(a->listaPokemons) && !ent1->fallecio;
					ew++) {
				metaDataPokemon* meta;
				meta = list_get(a->listaPokemons, ew);
				if (!meta->estaOcupado) {

					sem_post(&(strub->sembloq));
				}
			}
		}

		sem_wait(&(strub->sembloq));

		if (!ent1->fallecio) {

			pthread_mutex_lock(&pokemi);

			pokimons* poki;

			//log_info(logs, "Extrajo un bloqueado");
			bool esLaPokenest(pokimons *parametro1) {
				return strub->pokenest == parametro1->pokinest;
			}

			//log_info(logs, "Ahora busca un poki");
			poki = list_find(pokemons, (void*) esLaPokenest); //sc
			//log_info(logs, "Saca un poki");
			//log_info(logs, "%c", poki->pokinest);
			pthread_mutex_unlock(&pokemi);

			int auxi67;

			bool esLaPokenest2(tabla* a) {
				return strub->pokenest == a->pokenest;
			}

			for (auxi67 = 0;
					auxi67 < list_size(poki->listaPokemons) && !ent1->fallecio;
					auxi67++) {
				metaDataPokemon* pokem;
				pokem = list_get(poki->listaPokemons, auxi67); //sc
				if (!pokem->estaOcupado && !ent1->fallecio) {

					pthread_mutex_lock(&mutexEnvio);
					pokem->estaOcupado = 1;

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

					send((clientesActivos[ent1->numeroCliente]).socket,
							miBuffer,
							(4 * sizeof(int)) + tamanioCosaUno + tamanioCosaDos,
							0);

					//free(caracterNulo); // si rompe, sacarlo
					free(miBuffer);
					auxi67 = list_size(poki->listaPokemons);

					//log_info(logs,"ahora el pokem esta ocupado %d",pokem->estaOcupado);

					tabla* t;
					tabla* d;
					tabla* dispo1;

					t = list_find(ent1->asignados, (void*) esLaPokenest2);
					d = list_find(ent1->solicitud, (void*) esLaPokenest2);
					dispo1 = (tabla*) list_find(disponibles,
							(void*) esLaPokenest2);

					t->valor = t->valor + 1;
					if (ent1->sumo) {
						d->valor = d->valor - 1;
						ent1->sumo = 0;
					}
					dispo1->valor = dispo1->valor - 1;

					//		log_info(logs,"%c captura el pokemon %c la tabla de asignacion en ese campo es %d y la dispo %d ",ent1->simbolo, pokem->especie[0], t->valor,dispo1->valor);
					list_add(ent1->pokemones, pokem);
					log_info(logs, "%c capturo efectivamente a %s",
							ent1->simbolo, pokem->especie);
					//int m;
					/*for (m = 0; m < list_size(ent1->pokemones); m++) {
					 metaDataPokemon* q;
					 q = list_get(ent1->pokemones, m);
					 log_info(logs, "hasta ahora estan capturados por %c %s",
					 ent1->simbolo, q->especie);
					 }*/

					//log_info(logs, "llego a bloqueados");
					//postea semaforo pokemon de P y su valor es 2
					restarRecurso(items, poki->pokinest);
					nivel_gui_dibujar(items, nombreMapa);

					log_info(logs,
							"entrenador %c sale de la cola de %c y entra a listos",
							ent1->simbolo, strub->pokenest);


					bool esesnt(entrenador* ap){
														return ap->simbolo == ent1->simbolo;
													}
													list_remove_by_condition(colaDeBloqImp, (void*)esesnt);
													list_add(colaDeListosImp, ent1);
													log_info(logs, "entrenadores en cola de listos:");
																		int auxilie;
																		for(auxilie=0;auxilie<list_size(colaDeListosImp);auxilie++){
																			entrenador* entprint;
																			entprint = list_get(colaDeListosImp,auxilie);
																			log_info(logs, "entrenador %c",entprint->simbolo);
																		}


												    log_info(logs, "entrenadores en cola de bloqueados:");
												    int auxiliere;
												    for(auxiliere=0;auxiliere<list_size(colaDeBloqImp);auxiliere++){
												    	entrenador* entprint;
														entprint = list_get(colaDeBloqImp,auxiliere);
														log_info(logs, "entrenador %c",entprint->simbolo);
												    }



					queue_push(colaListos, ent1);
					sem_post(&sem_Listos);

					pthread_mutex_unlock(&mutexEnvio);
				}
			}

		} else {

			pthread_mutex_lock(&mutexMuerte);
			log_info(logs, "mata a entrenador %c en bloqueados", ent1->simbolo);

			matar(ent1);

			sem_post(&(strub->sembloq));
			pthread_mutex_unlock(&mutexMuerte);

		}
	}
}

///////////////////////////////////////////Main//////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	//nombre de mapa
	nombreMapa = argv[1];
	char* papaya = string_new();
	string_append(&papaya, nombreMapa);
	string_append(&papaya, ".log");
	//inicializo listas

	pokemons = list_create();
	disponibles = list_create();
	pokenests = list_create();
	items = list_create();
	entrenadoresEnCurso = list_create();
	listaContenedora = list_create();
	deadlocks = list_create();
	colaDeListosImp = list_create();
	colaDeBloqImp = list_create();
	//inicializo colas
	colaListos = queue_create();
	//inicializo semaforos
	sem_init(&sem_Listos, 0, 0);
	sem_init(&sem_Bloqueados, 0, 0);
	sem_init(&sem_quantum, 0, 0);
	sem_init(&sem_llego, 0, 0);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	remove(papaya);
	logs = log_create(papaya, "Mapa", false, log_level_from_string("INFO"));

	free(papaya);

// CONFIG

	datosMapa = malloc(sizeof(metaDataComun));
	datosMapa2 = malloc(sizeof(metaDataComun));
	//datosPokenest = malloc(sizeof(metaDataPokeNest));
	//datosPokemon = malloc(sizeof(metaDataPokemon));

	configMapa = string_from_format("%s/Mapas/%s/metadata", argv[2], argv[1]);

	leerConfiguracion();

	leerConfiguracion2();

	signal(SIGUSR2, leerConfiguracion2);
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
		metaDataPokeNest* datosPokenest;
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

	int socketEscucha;
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
		poll(fileDescriptors, cantfds, -1);

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
				pthread_create(&hiloAtenderConexiones[n], &attr,
						atenderConexion, numeroCliente);

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
		metaDataPokeNest* datosPokenest;
		datosPokenest = (metaDataPokeNest*) list_get(pokenests, ka);
		char ide;
		ide = datosPokenest->caracterPokeNest[0];
		BorrarItem(items, ide);
	}
	close(socketEscucha);
	nivel_gui_terminar();

	signal(SIGINT, terminarMapa);

	return EXIT_SUCCESS;

}
