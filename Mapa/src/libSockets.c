/*
 * libSockets.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include <dirent.h>
#include "libSockets.h"
#define HEADER_PAQUETE (sizeof(int)*3)

int leerConfiguracion(char *ruta, metaDataComun **datos) {
	t_config* archivoConfiguracion = config_create(ruta); //Crea struct de configuracion
	if (archivoConfiguracion == NULL) {
		return 0;
	} else {
		int cantidadKeys = config_keys_amount(archivoConfiguracion);
		if (cantidadKeys < 7) {
			return 0; // sale, 0 es false
		} else {

			(*datos)->tiempoChequeoDeadlock = config_get_int_value(
					archivoConfiguracion, "TiempoChequeoDeadlock");
			//printf("TiempoChequeo %d\n",(*datos)->tiempoChequeoDeadlock);

			(*datos)->batalla = config_get_int_value(archivoConfiguracion,
					"Batalla");
			//printf("batalla %d\n",(*datos)->batalla);

			char* algoritmo = string_new();
			string_append(&algoritmo,
					config_get_string_value(archivoConfiguracion, "algoritmo"));
			(*datos)->algoritmo = algoritmo;
			//printf("algoritmo  %s\n",(*datos)->algoritmo);

			(*datos)->quantum = config_get_int_value(archivoConfiguracion,
					"quantum");
			//printf("quantum  %d\n",(*datos)->quantum);

			(*datos)->retardoQ = config_get_int_value(archivoConfiguracion,
					"retardo");
			//printf("retardo  %d\n",(*datos)->retardoQ);

			char* ip = string_new();
			string_append(&ip,
					config_get_string_value(archivoConfiguracion, "IP"));
			(*datos)->ip = ip;
			//printf("ip  %s\n",(*datos)->ip);

			(*datos)->puerto = config_get_int_value(archivoConfiguracion,
					"Puerto");
			//printf("puerto %d\n",(*datos)->puerto);

			config_destroy(archivoConfiguracion);
			return 1; // cualquier otra cosa que no es 0, es true
		}
	}
}

int leerConfigPokenest(char *name, t_list *pokenests) {


	{
		DIR *d;
		struct dirent *dir;
		d = opendir(name);
		if (!d) {
			return 0;
		}


		while ((dir = readdir(d)) != NULL ) {
			metaDataPokeNest* datos;
			datos = malloc(sizeof(metaDataPokeNest));

			if (dir->d_type == DT_DIR && (strcmp(dir->d_name, ".") != 0) && (strcmp(dir->d_name, "..") != 0)) {

				char* ruta = string_new();
				string_append(&ruta,string_from_format("%s/%s/metadata", name,dir->d_name));
				t_config* archivoConfigPokenest = config_create(ruta);
				printf("%s\n", dir->d_name);

				if (archivoConfigPokenest == NULL) {
					return 0;
				} else {
					int cantidadKeys = config_keys_amount(
							archivoConfigPokenest);
					if (cantidadKeys < 3) {
						return 0;
					} else {
						char* tipo = string_new();
						string_append(&tipo,
								config_get_string_value(archivoConfigPokenest,
										"Tipo"));
						datos->tipoPokemon = tipo;


						char* posicion = string_new();
						string_append(&posicion,
								config_get_string_value(archivoConfigPokenest,
										"Posicion"));
						datos->posicion = posicion;

						char* simbolo = string_new();
						string_append(&simbolo,
								config_get_string_value(archivoConfigPokenest,
										"Identificador"));
						datos->caracterPokeNest = simbolo;

						{
							int file_count = 0;
							DIR * dirp;
							struct dirent * entry;

							dirp = opendir(
									string_from_format("%s/%s", name,
											dir->d_name));
							while ((entry = readdir(dirp)) != NULL) {
								if (entry->d_type == DT_REG) {
									file_count++;
								}
							}
							closedir(dirp);
							datos->cantPokemons = (file_count - 1);
						}

						list_add(pokenests, (void*) datos);

						config_destroy(archivoConfigPokenest);
						//free(datos);
					}
				}

			}
		}

		/*int ka;
					metaDataPokeNest *a;
					a = malloc(sizeof(metaDataPokeNest));
					   for(ka=0; ka<list_size(pokenests); ka++){
					    	    a = (metaDataPokeNest*) list_get(pokenests,ka);
					    	    printf("%s\n",a->caracterPokeNest);
	                            printf("%s\n",a->tipoPokemon);
	                            printf("%d\n",a->cantPokemons);
	                            printf("%s\n",a->posicion);
					    }*/

		closedir(d);

	}

	return 1;
}

int leerConfigPokemon(char* ruta, metaDataPokemon **datos) {
	t_config* archivoConfigPokemon = config_create(ruta); //Crea struct de configuracion
	if (archivoConfigPokemon == NULL) {
		return 0;
	} else {
		int cantidadKeys = config_keys_amount(archivoConfigPokemon);
		if (cantidadKeys < 1) {
			return 0;
		} else {
			(*datos)->nivel = config_get_int_value(archivoConfigPokemon,
					"Nivel");

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

