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
#define PACKAGESIZE 1024
 t_log* logs;

 // copiar todos los archivos del Entrenador en /home/utnso/workspace/pokedex

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

	 //CONEXIONES

    int servidor;
    servidor = conectarCliente(IP, PUERTO);

    int enviar = 1;
    char message[PACKAGESIZE];
    char *resultado = malloc(sizeof(int));
	int resultadoEnvio = 0;
    printf("Conectado al Mapa. Ingrese el mensaje que desee enviar, o cerrar para salir\n");

    while(enviar != 0){
        fgets(message, PACKAGESIZE, stdin);
        if (!strcmp(message,"cerrar\n")) enviar = 0;
        if (enviar) send(servidor, message, strlen(message) + 1, 0);
        recv(servidor, (void *)resultado, sizeof(int), 0);
		resultadoEnvio = *((int *)resultado);

		if(resultadoEnvio == 1) {
			printf("el servidor recibió el mensaje!: %d\n", resultadoEnvio);
		}
		else if(resultadoEnvio == 9){
			printf("Servidor caído! imposible reconectar. Cerrando...\n");
			exit(0);
		}

    }

    close(servidor);

return EXIT_SUCCESS;

}
