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

typedef struct {
	char* nombreEntrenador;
	int identificador;
	int cantidadInicialVidas;
	t_list hojaDeViaje; // Fijarse que tipo de dato debería ser
}t_entrenador;

t_entrenador* crear_Entrenador(char* nombre, int id, int vidas, t_list hoja){
	t_entrenador* nuevoEntrenador = NULL;
	nuevoEntrenador = malloc(sizeof(t_entrenador));
	nuevoEntrenador->nombreEntrenador = nombre;
	nuevoEntrenador->identificador = id;
	nuevoEntrenador->cantidadInicialVidas = vidas;
	nuevoEntrenador->hojaDeViaje = hoja;
	return nuevoEntrenador;
}

void crearDirectorioDeEntrenador(t_entrenador* entrenador){ //Esto deberia ir en pokedex
	char* comando_Directorio_Entrenador = string_from_format("mkdir -p /home/utnso/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/Entrenadores/%s/", entrenador->nombreEntrenador);
	system(comando_Directorio_Entrenador); // Crea los directorios Entrenadores (si es que no existe) y el del entrenador en particular

	char* comando_Directorio_Entrenador_Medallas = string_from_format("mkdir -p /home/utnso/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/Entrenadores/%s/%s/", entrenador->nombreEntrenador, "medallas");
	system(comando_Directorio_Entrenador_Medallas); // Crea el directorio medalla en la ruta del entrenador particular

	char* comando_Directorio_Entrenador_Metadata = string_from_format("mkdir -p /home/utnso/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/Entrenadores/%s/%s/", entrenador->nombreEntrenador, "metadata");
	system(comando_Directorio_Entrenador_Metadata); // Crea el directorio metadata en la ruta del entrenador particular

	char* comando_Directorio_Entrenador_DirBill = string_from_format("mkdir -p /home/utnso/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/Entrenadores/%s/%s/", entrenador->nombreEntrenador, "Dir\\ de\\ Bill");
	system(comando_Directorio_Entrenador_DirBill);
}

int main(void) {
	t_list hojaPrueba;
	t_entrenador* prueba = crear_Entrenador("Prueba",1,3,hojaPrueba);
	crearDirectorioDeEntrenador(prueba);
		return 0;
	}
