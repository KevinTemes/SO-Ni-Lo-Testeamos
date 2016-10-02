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
void* recibirUbicacionPokenest(int, int);
void moverseEnUnaDireccion(int,int,int,int,char*,int);
//void morir(t_entrenador*);
void mostrarMotivo();
void borrarArchivosBill(t_entrenador*);
int leQuedanVidas(t_entrenador*);
void resetear(t_entrenador*);
int capturaUltimoOK(t_entrenador*);
void reconectarse(t_entrenador*);



int main(int argc, char* argv[]){ // PARA EJECUTAR: ./Entrenador Ash /home/utnso/workspace/pokedex

	// LOGS
	 remove("Entrenador.log");
	 puts("Creando archivo de logueo...\n");
	 logs = log_create("Entrenador.log", "Entrenador", true, log_level_from_string("INFO"));
	 puts("Log Entrenador creado exitosamente \n");

	 //CONFIG
	 t_entrenador* ent;

	 ent = malloc(sizeof(t_entrenador));

	 char* configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",argv[2],argv[1]);

	 //para cuando debuggeamos
	 //char* configEntrenador = "/home/utnso/workspace/pokedex/Entrenadores/Red/metadata";

	 if (!leerConfigEnt(configEntrenador,&ent, argv[2])) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");


	 // DE ACA SACO CADA MAPA Y CADA OBJETIVO,CADA IP Y CADA PUERTO
	 //list_iterate((ent)->hojaDeViaje,(void*)obtengoCadaUno);
	 //list_iterate((ent)->objetivosPorMapa,(void*)obtengoCadaUno);
	 /*list_iterate(ips, (void*)obtengoCadaUno);
	 list_iterate(puertos, (void*)obtengoCadaUno); */

	 //CONEXIONES
	 int pos;

	 for(pos = 0;pos<list_size((ent)->hojaDeViaje);pos++){


		char* miIP= list_get(ips,pos); // no se estan usando por el momento
		char* miPuerto = list_get(puertos,pos); // no se esta usando por el momento

		printf("%s \n", miIP);
		printf("%s \n", miPuerto);

		servidor = conectarCliente(IP, PUERTO); // siguen los valores hardcodeados

		//int enviar = 1;
		//char message[PACKAGESIZE];
		char *resultado = malloc(sizeof(int));
		int resultadoEnvio = 0;
		printf("Conectado al Mapa. Ingrese el mensaje que desee enviar, o cerrar para salir\n");


		//////////////// recibo y mando datos al Mapa /////////////////////
		//int num=0;
		//char* numConcatenado = string_itoa(num);
		char* protocolo = string_new();
		char* numConcatenado;
		string_append(&protocolo,(ent)->caracter);
		string_append(&protocolo,numConcatenado);
		char* protocAManejar = strdup(protocolo);

		char* coordPokenest;
		char** posPokenest;

		int posXInicial =0;
		int posYInicial =0;

		int posObjetivo;

		for(posObjetivo=0;posObjetivo<list_size((ent)->objetivosPorMapa);posObjetivo++){

			// MANDO: CARACTER + POKENEST
			char* caracterPoke = list_get((ent)->objetivosPorMapa,posObjetivo);
			string_append(&protocolo,caracterPoke);
			printf("%s \n", caracterPoke);

			protocAManejar[1]='P';

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

			//if(capturaUltimoOk(ent)){
			//	copiarMedalla();
			//	close(servidor);
			//}
		} // cierro el for de los objetivos

		free(protocAManejar);
		close(servidor); // puse aca por ahora
	}

/*list_destroy_and_destroy_elements(ips,free);
list_destroy_and_destroy_elements(puertos,free);
list_destroy_and_destroy_elements((ent)->hojaDeViaje,free);
list_destroy_and_destroy_elements((ent)->objetivosPorMapa,free);*/

   // verificarHojadeViaje(ent); // Se fija a donde tiene que ir y se conecta al mapa

return EXIT_SUCCESS;

}


///////////////////// FUNCIONES DEL ENTRENADOR ///////////////////////////

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
