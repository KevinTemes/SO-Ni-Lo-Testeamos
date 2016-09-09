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


int main(int argc, char* argv[]) {

	remove("Mapa.log");
	puts("Creando archivo de logueo...\n");
	t_log* logs;
	logs = log_create("Mapa.log", "Mapa", true, log_level_from_string("INFO"));
	puts("Log creado exitosamente \n");


	t_queue* listoParaMoverse;
	t_queue* esperaCapturarPokemon;
	t_list* finalizaAnormal;
	listoParaMoverse=queue_create();
	esperaCapturarPokemon=queue_create();
	finalizaAnormal=list_create();


	// CONFIGURACION
	metaDataComun* datosMapa;
	metaDataPokeNest* datosPokenest;
	metaDataPokemon* datosPokemon;

	datosMapa=malloc(sizeof(metaDataComun));
	datosPokenest= malloc(sizeof(metaDataPokeNest));
	datosPokemon= malloc(sizeof(metaDataPokemon));

	/*
	if (!leerConfiguracion("MetadataComun", &datosMapa)) {
			log_error(logs,"Error al leer el archivo de configuracion de Metadata\n");
			return 1;
	}

	if (!leerConfigPokenest("MetadataPokenest",&datosPokenest)){
		log_error(logs,"Error al leer el archivo de configuracion de Metadata Pokenest\n");
		return 2;
	}

	if (!leerConfigPokemon("MetadataPokemon",&datosPokemon)){
		log_error(logs,"Error al leer el archivo de configuracion de Metadata de Pokemons\n");
		return 3;
	}*/

	char* inicio = string_new();

	printf("Queres dibujar el mapa? Responde \"Si\" si queres dibujarlo, o otra cosa si no queres\n");
	scanf("%s", inicio);

	if(!strcmp(inicio,"Si")){ // porque el strcmp devuelve 0 si son iguales, si lo negamos devuelve 1 y entra al if

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

		nivel_gui_terminar();
	}

	return EXIT_SUCCESS;
}
