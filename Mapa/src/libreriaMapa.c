/*
 * libreriaMapa.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaMapa.h"
#include "libSockets.h"
#include <tad_items.h>

#define MAX_LEN 128

pthread_mutex_t mutexPaqueton = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexRegistro = PTHREAD_MUTEX_INITIALIZER;


int numEntrenador;

t_infoCliente clientesActivos[2048];

extern sem_t sem_Listos;
extern char* nombreMapa;
extern metaDataComun* datosMapa;
extern t_log* logs;
extern t_queue* colaListos;
extern t_list* entrenadoresEnCurso;
extern t_list* pokenests;
extern t_list* items;

/////////////////////////////////////////////////////////////////////////////
void* recibirDatos(int conexion, int tamanio) {
	void* mensaje = (void*) malloc(tamanio);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios = string_new();
		string_append(&adios, "0\0");
		return adios;
	}
	return mensaje;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimirGiladas(void *unCliente) {

	t_infoCliente *infoCliente = (t_infoCliente *) unCliente;
	char paquete[1024];
	int status = 1;

	void enviarAvisoDeCierre() {
		printf("\n");
		enviarHeader(infoCliente->socket, 9);
		printf(
				"AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
		exit(0);
	}

	printf("Entrenador #%d conectado! esperando mensajes... \n",
			infoCliente->cliente);

	signal(SIGINT, enviarAvisoDeCierre);

	while (status != 0) {
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el Entrenador #%d dijo: \n %s", infoCliente->cliente,
					paquete);
			enviarHeader(infoCliente->socket, 1);
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
int calcularDistancia(entrenador* ent)
{
	return abs(ent->posx - ent->posPokex)+abs(ent->posy - ent->posPokey);
}

bool esMasCerca(entrenador* cerca, entrenador* lejos)
{
	return calcularDistancia(cerca) < calcularDistancia(lejos);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
void enviarHeader(int unSocket, int unHeader) {
	char *recepcion = malloc(sizeof(int));
	memcpy(recepcion, &unHeader, sizeof(int));
	send(unSocket, recepcion, sizeof(int), 0);
	free(recepcion);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimir_archivo(char *rutaDelArchivo) {
	int c;
	FILE *file;
	file = fopen(rutaDelArchivo, "r");
	if (file) {
		while ((c = getc(file)) != EOF)
			putchar(c);
		fclose(file);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *txtAString(char *rutaDelArchivo) {
	char * buffer = 0;
	long length;
	FILE * f = fopen(rutaDelArchivo, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}
	return buffer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void notificarCaida() {
	int i;
	for (i = 0; i <= 1024; i++) {
		enviarHeader(clientesActivos[i].socket, 9);
	}
	printf("\n");
	printf(
			"AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void matar(entrenador* entreni) {
	log_info(logs,"sale entrenado %c de los que estan en curso",entreni->simbolo);
	bool esEntrenador(entrenador* entiti){
		return entreni->simbolo == entiti->simbolo;
	}
	list_remove_by_condition(entrenadoresEnCurso, (void*)esEntrenador);
	free(entreni->pokePeleador);
	log_info(logs,"libera pokemon del entrenador %c",entreni->simbolo);
	list_destroy_and_destroy_elements(entreni->asignados, (void*) free);
	log_info(logs,"libera lista de asignados de %c",entreni->simbolo);
	list_destroy_and_destroy_elements(entreni->solicitud, (void*) free);
	log_info(logs,"libera lista de solicitud de %c",entreni->simbolo);
	int y;
	for (y = 0; y < list_size(entreni->pokemones); y++) {
		metaDataPokemon* pok;
		pok = list_get(entreni->pokemones, y);
		pok->estaOcupado = 0;
	}
	list_destroy(entreni->pokemones);
	queue_destroy(entreni->colaAccion);
	BorrarItem(items, entreni->simbolo);
	nivel_gui_dibujar(items, nombreMapa);
	free(entreni);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente) {
	int unCliente = *((int *) numeroCliente);
	char paquete[1024];
	char cambio[2];
	int status = 1;

	entrenador* ent1 = malloc(sizeof(entrenador));
	ent1->flagEstaEnLista = 0; //al ser nuevo no esta registrado en la lista

	t_log* logi;
	remove("Mapi.log");

	logi = log_create("Mapi.log", "Mapa", false, log_level_from_string("INFO"));

	//printf("Entrenador #%d conectado! esperando mensajes... \n",
	//clientesActivos[unCliente].cliente);
	while (status != 0) {

		// paquete = recibirDatos(clientesActivos[unCliente].socket,2);
		status = recv(clientesActivos[unCliente].socket, (void*) paquete,
				sizeof(char), MSG_WAITALL);
		if (status != 0) {
			//printf("el Entrenador #%d dijo: \n %s", clientesActivos[unCliente].cliente, paquete);
			//status = recv(clientesActivos[unCliente].socket, paquete, 2, 0);
			//log_info(logs,"paquete: %c%c",paquete[0],paquete[1]);
			cambio[0] = paquete[0];
			if (isalpha(cambio[0]) || cambio[0] == '2' || cambio[0] == '6'
					|| cambio[0] == '8' || cambio[0] == '4'
					|| cambio[0] == '9') {
				status = recv(clientesActivos[unCliente].socket,
						(void*) paquete, sizeof(char), MSG_WAITALL);

				cambio[1] = paquete[0];

				log_info(logi, "paquete global:%c %c", cambio[0], cambio[1]);
				log_info(logi,
						"unCliente:%d clientesactivos[uncliente].cliente:%d",
						unCliente, clientesActivos[unCliente].cliente);
				//printf("%c",paqueton[0]);

				//enviarHeader(clientesActivos[unCliente].socket, 1);

				log_info(logi, "Este seria un caracter del entrenador: %c",
						cambio[1]);
				log_info(logi, "Este seria su accionar: %d", cambio[0]);

				//mientras no haya recibido nada

				//paso variable global al entrenador

				//ent1->accion = cambio[0]; //almaceno que hacer en entrenador paqueton global

				//si el entrenador no esta registrado
				if (!ent1->flagEstaEnLista) {


					ent1->simbolo = cambio[1]; //almaceno simbolo en entrenador (paqueton [0] es variable global
					ent1->numeroCliente = unCliente; //numero de cliente para envio de informacion
					ent1->flagLeAsignaronPokenest = 0;
					ent1->numeroLlegada = (clientesActivos[unCliente].cliente
							- 1); //numero del entrenador
					ent1->flagEstaEnLista = 1; //ahora este entrenador nuevo esta en la lista
					ent1->estaMarcado = 0;
					ent1->entroBloqueados = 0;
					ent1->fallecio = 0;
					ent1->pokePeleador=malloc(sizeof(metaDataPokemon));
					ent1->posx = 0; //posicion en x inicializada en 0
					ent1->posy = 0; // idem en y
					ent1->asignados = list_create();
					ent1->solicitud = list_create();
					ent1->pokemones = list_create();
					int ka;
					metaDataPokeNest* datosPo;
					for (ka = 0; ka < list_size(pokenests); ka++) {
						datosPo = (metaDataPokeNest*) list_get(pokenests, ka);
						tabla* t = malloc(sizeof(tabla));
						tabla* otre = malloc(sizeof(tabla));
						t->pokenest = datosPo->caracterPokeNest[0];
						t->valor = 0;
						otre->pokenest = datosPo->caracterPokeNest[0];
						otre->valor = 0;
						list_add(ent1->asignados, t);
						list_add(ent1->solicitud, otre);
					}

					ent1->colaAccion = queue_create();
					queue_push(ent1->colaAccion, cambio[0]); // meto la accion del entrenador en la cola

					//list_replace(listaDeColasAccion, ent1->numeroLlegada, coli); // agrego en la lista que contiene su cola de accion

					//list_add(listaDeColasAccion, coli);

					//list_add_in_index(listaDeColasAccion,ent1->numeroLlegada,coli);


					CrearPersonaje(items, ent1->simbolo, ent1->posx,
							ent1->posy); //mete al pj en el mapa

					nivel_gui_dibujar(items, nombreMapa);

					list_add(entrenadoresEnCurso, ent1);

					pthread_mutex_lock(&mutexPaqueton);

					usleep(datosMapa->retardoQ);
					queue_push(colaListos, ent1); //llego un entrenador entonces lo meto en la cola de listos

					log_info(logi, "entrenador %c a listos", ent1->simbolo); //informo por archivo de log la llegada del entrenador

					int otroauxix;

					t_list* listAux = list_create();

					for(otroauxix=0;otroauxix<queue_size(colaListos);otroauxix++){
						list_add(listAux,queue_pop(colaListos));
					}

					for(otroauxix=0;otroauxix<list_size(listAux);otroauxix++){
						entrenador* entrena;
						entrena = list_get(listAux,otroauxix);
						log_info(logs,"entrenadores en cola listos: %c",entrena->simbolo);
						queue_push(colaListos,entrena);
					}


					sem_post(&sem_Listos); //produce un ent en colaListos

				/*	if(!strcmp(datosMapa->algoritmo,"RR")){
					sem_post(&sem_quantum);
					} */

					pthread_mutex_unlock(&mutexPaqueton);



					//	aux = '\0';

				}

				//si el entrenador se encontraba registrado
				else {
					pthread_mutex_lock(&mutexRegistro);

					queue_push(ent1->colaAccion, cambio[0]); //pusheo nuevo accionar a la cola auxiliar
					/*int ac;
					 ac = queue_pop(cola);
					 log_info(logs,"acto es %d", ac); */

					//list_replace(listaDeColasAccion, ent1->numeroLlegada, cola); //reemplaza la cola de la lista por la auxiliar
					log_info(logi, "paso el replace");

		/*			if(!strcmp(datosMapa->algoritmo,"RR")){
					sem_post(&sem_quantum);
					}
*/
					pthread_mutex_unlock(&mutexRegistro);

					//aux = '\0';
				}

				//aux = '\0';
			}
			if (cambio[0] == 5) {
				int tamanioUno, tamanioDos;

				ent1->pokePeleador = malloc(sizeof(metaDataPokemon));

				recv(clientesActivos[ent1->numeroCliente].socket, &tamanioUno,
						sizeof(int), MSG_WAITALL);
				recv(clientesActivos[ent1->numeroCliente].socket, &tamanioDos,
						sizeof(int), MSG_WAITALL);

				void *bufferCosaUno = malloc(tamanioUno);
				void *bufferCosaDos = malloc(tamanioDos);

				recv(clientesActivos[ent1->numeroCliente].socket, bufferCosaUno,
						tamanioUno, MSG_WAITALL);
				recv(clientesActivos[ent1->numeroCliente].socket, bufferCosaDos,
						tamanioDos, MSG_WAITALL);

				((ent1->pokePeleador)->especie) = (char*) bufferCosaUno;
				((ent1->pokePeleador)->nivel) = (int) bufferCosaDos;

				free(bufferCosaDos);
				free(bufferCosaUno);
			}

			if(cambio[0] == 1){
				status = 0;

			}

		}

	}
	ent1->fallecio = 1;
	queue_clean(ent1->colaAccion);
    log_info(logi,"entrenador fallece");

}
