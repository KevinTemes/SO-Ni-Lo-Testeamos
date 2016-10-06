/*
 ============================================================================

 ============================================================================
 Name        : Entrenador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 ============================================================================
 Name        : Entrenador.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/config.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "libSockets.h"
#include <commons/collections/dictionary.h>

#define IP "127.0.0.1"
#define PUERTO "7900"
#define PACKAGESIZE 10

int servidor;
t_log* logs;

void terminarAventura(char*);
char* empezarAventura();
void copiarMedalla(char*, char*, t_entrenador*);
void* recibirUbicacionPokenest(int, int);
void moverseEnUnaDireccion(int,int,int,int,char*,int);

// estas no estan implementadas
//void morir(t_entrenador*);
void mostrarMotivo();
void borrarArchivosBill(t_entrenador*);
int leQuedanVidas(t_entrenador*);
void resetear(t_entrenador*);
void reconectarse(t_entrenador*);



int main(int argc, char* argv[]){ // PARA EJECUTAR: ./Entrenador Ash /home/utnso/workspace/pokedex
	char* nombreEnt = argv[1];
	char* puntoMontaje = argv[2];

	// LOGS
	 remove("Entrenador.log");
	 puts("Creando archivo de logueo...\n");
	 logs = log_create("Entrenador.log", "Entrenador", true, log_level_from_string("INFO"));
	 puts("Log Entrenador creado exitosamente \n");

	 //CONFIG
	 t_entrenador* ent;

	 ent = malloc(sizeof(t_entrenador));

	 char* configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",puntoMontaje,nombreEnt);

	 //para cuando debuggeamos
	 //char* configEntrenador = "/home/utnso/workspace/pokedex/Entrenadores/Red/metadata";

	 if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");

	 //VARIABLES USADAS Y CONEXION

	 int pos;
	 int cantMapas = list_size((ent)->hojaDeViaje);
	 int cantPokemonesPorMapa = list_size((ent)->objetivosPorMapa);
	 int posObjetivo;
	 char* protocolo = string_new();
	 char* numConcatenado="1";
	 string_append(&protocolo,(ent)->caracter);
	 string_append(&protocolo,numConcatenado);
	 char* protocAManejar = strdup(protocolo);
	 char* coordPokenest;
	 char** posPokenest;
	 char* horaInicio;

	 for(pos = 0;pos<cantMapas;pos++){

		//printf("entra devuelta \n");

		char* miIP;
		char* miPuerto;

		miIP= list_get(ips,pos);
		miPuerto = list_get(puertos,pos);
		//printf("llegue a saber ip y puerto \n");

		//el problema esta aca porque el mapa se cierra, hay que revisar mapa
		servidor = conectarCliente(miIP, miPuerto);
		//printf("aca no llega");


		char *resultado = malloc(sizeof(int));
		int resultadoEnvio = 0;

		char* mapa = list_get((ent)->hojaDeViaje,pos);
		printf("Conectado al Mapa %s. Ingrese el mensaje que desee enviar, o cerrar para salir\n",mapa);


		//////////////// recibo y mando datos al Mapa /////////////////////
		horaInicio = empezarAventura();

		// cuando pase a otro mapa, vuelve a arrancar en (0;0)
		int posXInicial =0;
		int posYInicial =0;


		for(posObjetivo=0;posObjetivo<cantPokemonesPorMapa;posObjetivo++){

			// MANDO: CARACTER + POKENEST
			char* caracterPoke = list_get((ent)->objetivosPorMapa,posObjetivo);
			string_append(&protocolo,caracterPoke);
			printf("%s \n", caracterPoke);

			char carPoke = caracterPoke[0];
			protocAManejar[1]=carPoke;

			send(servidor, protocAManejar, 2, 0);
			//recibo 5 chars, ej: "34;12"
			//coordPokenest = (char*)recibirUbicacionPokenest(servidor,5);

			coordPokenest= "02;03";

			posPokenest = string_split(coordPokenest,";");
			int x = atoi (posPokenest[0]);
			printf("Coordenada X pokenest: %d\n", x);
			int y = atoi (posPokenest[1]);
			printf("Coordenada Y pokenest: %d\n", y);


			moverseEnUnaDireccion(posXInicial, posYInicial, x, y, protocAManejar, servidor);


			protocAManejar[1]='9'; // Solicitud Atrapar Pokemon
			send(servidor,protocAManejar,2,0);

			// por si se cae
			recv(servidor, (void *)resultado, sizeof(int), 0);
			resultadoEnvio = *((int *)resultado);

			if(resultadoEnvio == 9){
				printf("Servidor caído! imposible reconectar. Cerrando...\n");
				exit(0);
			}

		} // cierro el for de los objetivos


		copiarMedalla(puntoMontaje, mapa, ent);

		close(servidor); // se desconecta el entrenador
	}

terminarAventura(horaInicio);

list_destroy_and_destroy_elements(ips,free);
list_destroy_and_destroy_elements(puertos,free);
free(protocAManejar);
free(protocolo);
free(ent);

return EXIT_SUCCESS;
}


///////////////////// FUNCIONES DEL ENTRENADOR ///////////////////////////
void terminarAventura(char* horaInicio){
	log_info(logs, "Ahora sos un maestro pokemon \n");
	time_t fechaFin;
	time(&fechaFin);
	char* horaFin = ctime(&fechaFin);
	int fin = atoi(horaFin);
	int inicio = atoi(horaInicio);
	int tiempoAventura = fin-inicio;
	char* mensTiempo = string_from_format("La aventura duró: %d \n",tiempoAventura);
	log_info(logs,mensTiempo);
	return;
}

char* empezarAventura(){
	time_t fechaActual;
	time(&fechaActual);
	char* horaInicio = ctime(&fechaActual);
	char* mensaje = string_from_format("Empezo: %s \n", ctime(&fechaActual));
	log_info(logs, mensaje);
	return horaInicio;
}

void copiarMedalla(char* puntoMontaje, char* mapa, t_entrenador* ent){

	char* medalla=string_from_format("cp %s/Mapas/%s/medalla-%s.jpg %s/Entrenadores/%s/medallas/medalla-%s.jpg", puntoMontaje, mapa, mapa, puntoMontaje, (ent)->nombreEntrenador,mapa);
	system(medalla);

	char* logueo = string_from_format("Copiada medalla del Mapa %s con exito \n", mapa);
	log_info(logs, logueo);
	return;
}


void* recibirUbicacionPokenest(int conexion, int tamanio){
	void* mensaje=(void*)malloc(tamanio);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios=string_new();
		string_append(&adios,"0\0");
		return adios;}
	return mensaje;
}


void moverseEnUnaDireccion(int posXInicial, int posYInicial,int x, int y, char* protocAManejar, int servidor){
	int cantMovX = x - posXInicial; // cant total mov de x
	int cantMovY = y - posYInicial; // cant total mov de y
	char ultMov;
	int movDeX = 0; // movimientos que se hicieron de x
	int movDeY = 0; // movimientos que se hicieron de y

	do
	{
		if(cantMovY==0)
			ultMov='y';
		if((cantMovX>0) && (ultMov!='x')){
			protocAManejar[1]='6'; // derecha
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		} else if((cantMovX<0) && (ultMov!='x')){
			protocAManejar[1]='4'; //izquierda
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		}

		if (cantMovX==0) ultMov = 'x';
		if((cantMovY>0) && (ultMov!='y')){
			protocAManejar[1]='2'; // abajo
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovY--;
			movDeY++;
		} else if((cantMovY<0) && (ultMov!='y')){
			protocAManejar[1]='8'; //arriba
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovY--;
			movDeY++;
		}

	} while((cantMovX!=0) || (cantMovY != 0));

	return;
}


/*void morir(t_entrenador* ent){
	mostrarMotivo();
	borrarArchivosBill(ent);
	close(servidor); // o cerrarConexion(ent);
	if (leQuedanVidas(ent)){
		ent->cantidadInicialVidas = ent->cantidadInicialVidas-1;
		reconectarse(ent);
	}
	else
	{
	 printf("Desea reiniciar juego?\n");
	 char* respuesta;
     fgets(respuesta, PACKAGESIZE, stdin);
     if (!strcmp(respuesta,"si\n")){
    	 ent->reintentos = ent->reintentos+1;
    	 resetear(ent);
     	 }
     else {
    	 exit(0);
     }
	}

}*/

void mostrarMotivo(){
	printf("Un motivo");
}


void borrarArchivosBill(t_entrenador* ent){
	return;
}


int leQuedanVidas(t_entrenador* ent){
	return ent->cantidadInicialVidas;
}


void resetear(t_entrenador* ent){
	//reiniciarHojaDeViaje(ent);
	//borrarMedallas(ent);
	//borrarPokemons(ent);
}


int capturaUltimoOK(t_entrenador* entrenador){
	return 1;
}


void reconectarse(t_entrenador* ent){
	return;
}
