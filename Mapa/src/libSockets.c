/*
 * libSockets.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */


#include "libSockets.h"

int leerConfiguracion(char *ruta, metaDataComun **datos) {
	t_config* archivoConfiguracion = config_create(ruta);//Crea struct de configuracion
	if (archivoConfiguracion == NULL) {
		return 0;
	} else {
		int cantidadKeys = config_keys_amount(archivoConfiguracion);
		if (cantidadKeys < 7) {
			return 0; // sale, 0 es false
		} else {
			(*datos)->tiempoChequeoDeadlock = config_get_int_value(archivoConfiguracion, "TiempoChequeoDeadlock");
			(*datos)->batalla = config_get_int_value(archivoConfiguracion,"Batalla");
			char* algoritmo=string_new();
			string_append(&algoritmo, config_get_string_value(archivoConfiguracion, "algoritmo"));
			(*datos)->algoritmo=algoritmo;
			(*datos)->quantum = config_get_int_value(archivoConfiguracion, "quantum");
			(*datos)->retardoQ = config_get_int_value(archivoConfiguracion, "retardo");
			char* ip=string_new();
			string_append(&ip,config_get_string_value(archivoConfiguracion,"IP"));
			(*datos)->ip=ip;
			(*datos)->puerto= config_get_int_value(archivoConfiguracion,"Puerto");

			config_destroy(archivoConfiguracion);
			return 1; // cualquier otra cosa que no es 0, es true
		}
	}
}

int leerConfigPokenest(char *ruta, metaDataPokeNest **datos) {
	t_config* archivoConfigPokenest = config_create(ruta);//Crea struct de configuracion
		if (archivoConfigPokenest == NULL) {
			return 0;
		} else {
			int cantidadKeys = config_keys_amount(archivoConfigPokenest);
			if (cantidadKeys < 3) {
				return 0;
			} else {
				char* tipo=string_new();
				string_append(&tipo, config_get_string_value(archivoConfigPokenest, "Tipo"));
				(*datos)->tipoPokemon=tipo;
				char* posicion=string_new();
				string_append(&posicion,config_get_string_value(archivoConfigPokenest,"Posicion"));
				(*datos)->posicion=posicion;
				(*datos)->caracterPokeNest=config_get_string_value(archivoConfigPokenest,"Identificador");

				config_destroy(archivoConfigPokenest);
				return 1;
			}
		}

}

int leerConfigPokemon(char* ruta, metaDataPokemon **datos){
	t_config* archivoConfigPokemon = config_create(ruta);//Crea struct de configuracion
			if (archivoConfigPokemon == NULL) {
				return 0;
			} else {
				int cantidadKeys = config_keys_amount(archivoConfigPokemon);
				if (cantidadKeys < 2) {
					return 0;
				} else {
					(*datos)->nivel = config_get_int_value(archivoConfigPokemon, "Nivel");
					(*datos)->caracterPokemon= config_get_string_value(archivoConfigPokemon,"[Ascii Art]");

					config_destroy(archivoConfigPokemon);
					return 1;
				}
			}
}
