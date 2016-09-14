/*
 * libSockets.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */


#include "libSockets.h"
#define HEADER_PAQUETE (sizeof(int)*3)

int leerConfiguracion(char *ruta, metaDataComun **datos) {
	t_config* archivoConfiguracion = config_create(ruta);//Crea struct de configuracion
	if (archivoConfiguracion == NULL) {
		return 0;
	} else {
		int cantidadKeys = config_keys_amount(archivoConfiguracion);
		if (cantidadKeys < 9) {
			return 0; // sale, 0 es false
		} else {
			char* nombreMapa=string_new();
			string_append(&nombreMapa, config_get_string_value(archivoConfiguracion, "nombreMapa"));
			(*datos)->nombreMapa=nombreMapa;
			// despues para crear el directorio, saco cada elemento de la lista pokenests
			(*datos)->pokenests= config_get_array_value(archivoConfiguracion,"pokenests");
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
			if (cantidadKeys < 4) {
				return 0;
			} else {
				char* tipo=string_new();
				string_append(&tipo, config_get_string_value(archivoConfigPokenest, "Tipo"));
				(*datos)->tipoPokemon=tipo;
				char* posicion=string_new();
				string_append(&posicion,config_get_string_value(archivoConfigPokenest,"Posicion"));
				(*datos)->posicion=posicion;
				(*datos)->caracterPokeNest=config_get_string_value(archivoConfigPokenest,"Identificador");
				// despues para crear el .dat de cada pokemon que tenga en la pokenest
				(*datos)->cantPokemons=config_get_int_value(archivoConfigPokenest,"cantPokemons");

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

///////////////////////////////////////////////////////////////////////////////////////////////////////

int setup_listen(char* IP, char* Port) {
	struct addrinfo * serverInfo = cargarInfoSocket(IP, Port);
	if (serverInfo == NULL)
		return -1;
	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	int resultadoBind;
	resultadoBind = bind(socketEscucha, serverInfo->ai_addr,
			serverInfo->ai_addrlen);
	if (resultadoBind == -1) {
		printf("Error en el Bind \n");
		exit(-1);
	}
	freeaddrinfo(serverInfo);
	return socketEscucha;
}

int setup_listen_con_log(char* IP, char* Port, t_log * logger) {
	struct addrinfo* serverInfo = cargarInfoSocket(IP, Port);
	if (serverInfo == NULL)
		return -1;
	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	log_info(logger,
			string_from_format("Escuchando conexiones en el socket %d",
					socketEscucha));
	bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return socketEscucha;
}

struct addrinfo* cargarInfoSocket(char *IP, char* Port) {
	struct addrinfo hints;
	struct addrinfo * serverInfo;
	int error;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (!strcmp(IP, "localhost")) {
		hints.ai_flags = AI_PASSIVE;
		error = getaddrinfo(NULL, Port, &hints, &serverInfo);
	} else
		error = getaddrinfo(IP, Port, &hints, &serverInfo);
	if (error != 0) {
		printf("Problema con el getaddrinfo()\n");
		return NULL;
	}
	return serverInfo;
}

int conectarCliente(char *IP, char* Port) {
	struct addrinfo* serverInfo = cargarInfoSocket(IP, Port);
	if (serverInfo == NULL) {
		return -1;
	}
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (serverSocket == -1) {
		printf("Error en la creacion del socket\n");
		return -1;
	}
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1) {
		printf("No se pudo conectar con el socket servidor\n");
		close(serverSocket);
		exit(-1);
	}
	freeaddrinfo(serverInfo);
	return serverSocket;
}

int conectarCliente_con_log(char *IP, char* Port, t_log * logger) {
	struct addrinfo* serverInfo = cargarInfoSocket(IP, Port);
	if (serverInfo == NULL) {
		return -1;
	}
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (serverSocket == -1) {
		log_error(logger,
				string_from_format("Error en la creación del socket"));
		return -1;
	}
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1) {
		log_error(logger,
				string_from_format(
						"No se pudo conectar con el socket servidor\n"));
		close(serverSocket);
		return -1;
	}
	freeaddrinfo(serverInfo);
	return serverSocket;
}

int esperarConexionEntrante(int socketEscucha, int BACKLOG, t_log * logger) {

	listen(socketEscucha, BACKLOG);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int socketCliente = accept(socketEscucha, (struct sockaddr *) &addr,
			&addrlen);
	log_info(logger,
			string_from_format("Se asigno el socket %d para el cliente",
					socketCliente));
	return socketCliente;

}

int conectarServidor(char* IP, char* Port, int backlog) {
	struct addrinfo* serverInfo = cargarInfoSocket(IP, Port);
	if (serverInfo == NULL)
		return -1;
	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1) {
		printf("Error en el Bind \n");
	}
	freeaddrinfo(serverInfo);
	if (listen(socketEscucha, backlog) == -1) {
		printf("error en la escucha de un cliente");
		return -5;
	}

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(socketEscucha, (struct sockaddr *) &addr,
			&addrlen);
	if (socketCliente == -1) {
		printf("Error en la conexion, en la funcion accept\n");
		return -2;
	}
	return socketCliente;
}


