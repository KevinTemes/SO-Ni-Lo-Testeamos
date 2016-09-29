/*
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

t_log* logs;
void* recibirUbicacionPokenest(int, int);
void moverseEnUnaDireccion(int,int,int,int,char*,int);



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

	 if (!leerConfigEnt(configEntrenador,&ent, argv[2])) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");

	 // DE ACA SACO CADA MAPA Y CADA OBJETIVO,CADA IP Y CADA PUERTO

	 /*list_iterate((ent)->hojaDeViaje,(void*)obtengoCadaUno);
	 list_iterate((ent)->objetivosPorMapa,(void*)obtengoCadaUno);
	 list_iterate(ips, (void*)obtengoCadaUno);
	 list_iterate(puertos, (void*)obtengoCadaUno); */

	 //CONEXIONES

    int servidor;
    servidor = conectarCliente(IP, PUERTO);

    int enviar = 1;
    //char message[PACKAGESIZE];
    char *resultado = malloc(sizeof(int));
	int resultadoEnvio = 0;
    printf("Conectado al Mapa. Ingrese el mensaje que desee enviar, o cerrar para salir\n");


    //////////////// recibo y mando datos al Mapa /////////////////////
    int num = 1;
    char* numConcatenado = string_itoa(num);
    char* protocolo = string_new();
    string_append(&protocolo,(ent)->caracter);
    string_append(&protocolo,numConcatenado);
    char* coordPokenest;
    char** posPokenest;

    int posXInicial =0;
    int posYInicial = 0;

    while(enviar){

    // le mando mi caracter y que quiero solicitar ubicacion de pokenest
    send(servidor, protocolo, 2, 0);
    //recibo 5 chars, ej: "34;12"
    coordPokenest = (char*)recibirUbicacionPokenest(servidor,5);

    posPokenest = string_split(coordPokenest,";");
    int x = atoi (posPokenest[0]);
    int y = atoi (posPokenest[1]);

    moverseEnUnaDireccion(posXInicial, posYInicial, x, y, protocolo, servidor);


    // por si se cae
    recv(servidor, (void *)resultado, sizeof(int), 0);
    resultadoEnvio = *((int *)resultado);

	if(resultadoEnvio == 9){
		printf("Servidor caído! imposible reconectar. Cerrando...\n");
		exit(0);
	}

	enviar = 0;
    }
    close(servidor);

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


void moverseEnUnaDireccion(int posXInicial, int posYInicial,int x, int y, char* protocolo, int servidor){
if((posXInicial != x) && (posYInicial != y)){
    	protocolo = "@6"; // muevo a la derecha primero
    	send(servidor, protocolo, 2, 0);
    	posXInicial++;

    	protocolo = "@8"; // muevo para arriba
    	send(servidor, protocolo, 2, 0);
    	posYInicial++;

    	if(posXInicial==x){ // llego al x, se mueve solo en y
    		send(servidor, protocolo, 2, 0);
    		posYInicial++;
    	} else if(posYInicial==y) { // llego al y, se mueve solo en x
    		protocolo = "@6";
    		send(servidor, protocolo, 2, 0);
    		posXInicial++;
    	}
    }
}
