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
#include "libreriaMapa.h"

//Para testear sockets, despues vuela
#define IP "127.0.0.1"
#define PUERTO "7900"
#define PACKAGESIZE 1024
t_log* logs;

/*
void crearDirectorioDeMapa(t_mapa* mapa){
    char* comando_Directorio_Mapa = string_from_format
            ("mkdir -p /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/Mapas/%s/", mapa->nombreMapa);
    system(comando_Directorio_Mapa);

    char* comando_Directorio_Mapa_Medallas = string_from_format
            ("mkdir -p /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/Mapas/%s/Medalla-%s/", mapa->nombreMapa, mapa->nombreMapa);
    system(comando_Directorio_Mapa_Medallas);

    char* comando_Directorio_Mapa_Metadata = string_from_format
            ("mkdir -p /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/Mapas/%s/%s/", mapa->nombreMapa, "PokeNest");
    system(comando_Directorio_Mapa_Metadata);

    char* comando_Directorio_Entrenador_DirBill = string_from_format
            ("mkdir -p /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/Entrenadores/%s/%s/", mapa->nombreMapa, "Dir\\ de\\ Bill");
    system(comando_Directorio_Entrenador_DirBill);
}*/


int main(int argc, char* argv[]) {

    //LOGS
	remove("Mapa.log");
    puts("Creando archivo de logueo...\n");
    logs = log_create("Mapa.log", "Mapa", false, log_level_from_string("INFO"));
    puts("Log Mapa creado exitosamente \n");

    /*if(argc < 2){
    	perror("Error en la cantidad de argumentos");
    	exit (1);
    }
*/
	signal(SIGINT, notificarCaida);

/* inicio todas las variables para arrancar */
	//SOCKETS
	log_info(logs, "iniciado el servidor principal de la Pokedéx. Aguardando conexiones...\n\n");

	int socketEscucha, retornoPoll;
	int fd_index = 0;


	struct pollfd fileDescriptors[100];
	int cantfds = 0;
	socketEscucha = setup_listen("localhost", PUERTO);
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
	pthread_t hiloImprimirGiladas[1024];
	pthread_t hiloAtenderConexiones[1024];



	while(enviar){

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

			//	pthread_create(&hiloImprimirGiladas[n],NULL, imprimirGiladas, infoCliente);
				pthread_create(&hiloAtenderConexiones[n], NULL, atenderConexion, numeroCliente);


				cliente++;
				n++;

				fileDescriptors[cantfds].fd = socketCliente;
				fileDescriptors[cantfds].events = POLLIN;
				cantfds++;

				goto llamadaPoll;
			}
		}
	}


    t_queue* listoParaMoverse;
    t_queue* esperaCapturarPokemon;
    t_list* finalizaAnormal;
    listoParaMoverse=queue_create();
    esperaCapturarPokemon=queue_create();
    finalizaAnormal=list_create();


    // CONFIG
    metaDataComun* datosMapa;
    metaDataPokeNest* datosPokenest;
    metaDataPokemon* datosPokemon;

    datosMapa=malloc(sizeof(metaDataComun));
    datosPokenest= malloc(sizeof(metaDataPokeNest));
    datosPokemon= malloc(sizeof(metaDataPokemon));


    if (!leerConfiguracion("metadata", &datosMapa)) {
            log_error(logs,"Error al leer el archivo de configuracion de Metadata\n");
            return 1;
    }

    if (!leerConfigPokenest("MetadataPokenest",&datosPokenest)){
        log_error(logs,"Error al leer el archivo de configuracion de Metadata Pokenest\n");
        return 2;
    }

    if (!leerConfigPokemon("MetadataPokemon.dat",&datosPokemon)){
        log_error(logs,"Error al leer el archivo de configuracion de Metadata de Pokemons\n");
        return 3;
    }

    log_info(logs,"Los tres archivos de config fueron creados exitosamente!\n");


    t_list* items = list_create();
    int rows; // nro de filas
    int cols; // nro de columnas
    int q, p; // dos valores para que se ubique un personaje

    int x = 1; // x del otro personaje
    int y = 1; // y del otro personaje

    nivel_gui_inicializar();
    nivel_gui_get_area_nivel(&rows, &cols);

    // para que el otro personaje arranque desde la otra punta
    p = cols;
    q = rows;

    CrearPersonaje(items, '@', p, q);
    CrearPersonaje(items, '#', x, y);

    CrearCaja(items, 'H', 26, 10, 5);
    CrearCaja(items, 'M', 8, 15, 3);
    CrearCaja(items, 'F', 19, 9, 2);

    //POKENEST
    char** posPoke;

    posPoke = string_split(datosPokenest->posicion,";");
    char ide;
    ide = datosPokenest->caracterPokeNest[0];

    CrearCaja(items, ide, atoi (posPoke[0]),atoi (posPoke[1]), datosPokenest->cantPokemons);

   // printf("\n%d",atoi(posich[0]));

    nivel_gui_dibujar(items, "Mapa con Entrenadores");

    while (1) {
            int key = getch();

            switch( key ) {

                case KEY_UP:
                    if (y > 1) {
                        y--;
                    }
                break;

                case KEY_DOWN:
                    if (y < rows) {
                        y++;
                    }
                break;

                case KEY_LEFT:
                    if (x > 1) {
                        x--;
                    }
                break;
                case KEY_RIGHT:
                    if (x < cols) {
                        x++;
                    }
                break;
                case 'w':
                case 'W':
                    if (q > 1) {
                        q--;
                    }
                break;

                case 's':
                case 'S':
                    if (q < rows) {
                        q++;
                    }
                break;

                case 'a':
                case 'A':
                    if (p > 1) {
                        p--;
                    }
                break;
                case 'D':
                case 'd':
                    if (p < cols) {
                        p++;
                    }
                break;
                case 'Q':
                case 'q':
                    nivel_gui_terminar();
                    exit(0);
                break;
            }

    MoverPersonaje(items, '@', p, q);
    MoverPersonaje(items, '#', x, y);

    if (   ((p == 26) && (q == 10)) || ((x == 26) && (y == 10)) ) {
        restarRecurso(items, 'H');
    }

    if (   ((p == 19) && (q == 9)) || ((x == 19) && (y == 9)) ) {
        restarRecurso(items, 'F');
    }

    if (   ((p == 8) && (q == 15)) || ((x == 8) && (y == 15)) ) {
        restarRecurso(items, 'M');
    }

    if((p == x) && (q == y)) {
        BorrarItem(items, '#'); //si chocan, borramos uno (!)
    }

    nivel_gui_dibujar(items, "Mapa con Entrenadores");
    }

        BorrarItem(items, '#');
        BorrarItem(items, '@');

        BorrarItem(items, 'H');
        BorrarItem(items, 'M');
        BorrarItem(items, 'F');

        BorrarItem(items, ide);

        nivel_gui_terminar();




    return EXIT_SUCCESS;
}
