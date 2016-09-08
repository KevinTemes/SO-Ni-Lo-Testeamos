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



int main(int argc, char* argv[]) {

	t_log* logs;
	logs = log_create("Mapa.log", "Mapa", true, log_level_from_string("INFO"));

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


	if (!leerConfiguracion(argv[1], &datosMapa)) {
			log_error(logs,"Error al leer el archivo de configuracion de Metadata\n");
			return 1;
	} else if (!leerConfigPokenest(argv[1],&datosPokenest)){
		log_error(logs,"Error al leer el archivo de configuracion de Metadata Pokenest\n");
		return 1;
	}



	// MAS ADELANTE CUANDO ESTE TODO CREADO
	printf("Ahora dibujamos el mapa\n");
	//t_list* items = list_create();
	//char* nombre_niv;
	nivel_gui_inicializar();
	return EXIT_SUCCESS;
}
