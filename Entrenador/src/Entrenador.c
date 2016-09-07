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
#include <commons/string.h>
#include <commons/collections/list.h>

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

char* crearDirectorioEntrenadores(){

	struct stat st = {0};
		char* directorio = string_from_format("/home/utnso/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/%s", "Entrenadores/");
		if (stat(directorio, &st) == -1) {
		    mkdir( directorio, 0700);
		}
		return directorio;
}

int crearDirectorioDeEntrenador(t_entrenador* entrenador, char* directorio){ //Esto deberia ir en pokedex

	struct stat st = {0};
	char* subdirectorio = string_from_format(directorio, entrenador->nombreEntrenador);
	if (stat(subdirectorio, &st) == -1) {
	    mkdir( subdirectorio, 0700);
	}
	// crearSubdirectorios(directorio); No hace lo esperado(no hace nada en realidad)
	return 0;
}

/*int crearSubdirectorios(char* directorio){ // Revisar, no funciona y se podria mejorar (sacar appends)
	struct stat st = {0};
		char* medalla = string_new();
		string_append(&medalla,directorio);
		string_append(&medalla,"/medalla");
		if (stat(directorio, &st) == -1) {
		    mkdir( medalla, 0700);
		}

		char* dirBill = string_new();
		string_append(&dirBill,directorio);
		string_append(&dirBill,"/Dir de Bill/");
		if (stat(directorio, &st) == -1) {
		    mkdir( dirBill, 0700);
		}
		//string_from_format(directorio,"/metadata");
		char* subdirectorio = string_from_format(directorio, "/metadata");
		//system(command);
		if (stat(subdirectorio, &st) == -1) {
			    mkdir( subdirectorio, 0700);
			}
		char* metadata = string_new();
		string_append(&metadata,directorio);
		string_append(&metadata,"/metadata");
		if (stat(directorio, &st) == -1) {
		    mkdir( metadata, 0700);
		}

		return 0;
}
*/

int main(void) {
	t_list hojaPrueba;
	t_entrenador* prueba = crear_Entrenador("Prueba",1,3,hojaPrueba);
	char* directorioEntrenadores = crearDirectorioEntrenadores();
	crearDirectorioDeEntrenador(prueba, directorioEntrenadores);
		return 0;
	}
