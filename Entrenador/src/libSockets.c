/*
 * libSockets.C
 *
 *  Created on: 1/9/2016
 *      Author: utnso
 */

#include "libSockets.h"
#define HEADER_PAQUETE (sizeof(int)*3)


int leerConfigEnt(char *ruta, t_entrenador **datos, char* puntoMontaje){
	t_config* archivoConfiguracion = config_create(ruta);//Crea struct de configuracion
	if (archivoConfiguracion == NULL) {
		return 0;
	} else {
		int cantidadKeys = config_keys_amount(archivoConfiguracion);
			if (cantidadKeys < 6) {
				return 0;
			} else {

			char* nombre = string_new();
			string_append(&nombre, config_get_string_value(archivoConfiguracion, "nombre"));
			(*datos)->nombreEntrenador=nombre;
			//printf("nombre=%s \n",(*datos)->nombreEntrenador);

			char* simbolo = string_new();
			string_append(&simbolo,config_get_string_value(archivoConfiguracion,"simbolo"));
			(*datos)->caracter=simbolo;
			//printf("simbolo= %s \n",(*datos)->caracter);

			char** hojaViaje =config_get_array_value(archivoConfiguracion,"hojaDeViaje");

			int i=1;
			int l=1;
			int j=0;
			int k=0;

			char* mapa = string_new();
			char* poke = string_new();

			pokesDeCadaMapa =dictionary_create();

			(*datos)->hojaDeViaje = list_create();
			(*datos)->pokemonsPorMapaCapturados = list_create();
			(*datos)->listaNivAtrapados = list_create();
			ips = list_create();
			puertos= list_create();


			do{
				if (hojaViaje[j]!=NULL){
					l=1;
					char* cadaMapa = string_from_format("%s",hojaViaje[j]);
					string_append(&mapa, cadaMapa);
					list_add((*datos)->hojaDeViaje,cadaMapa);

					char* objetivoDeMapa = string_from_format("obj[%s]",hojaViaje[j]);

					char** objetivosMapa = config_get_array_value(archivoConfiguracion,objetivoDeMapa);
					k=0;
					do {
						if (objetivosMapa[k]!=NULL){
							char* cadaPoke = string_from_format("%s",objetivosMapa[k]);
							string_append(&poke, cadaPoke);

							dictionary_put(pokesDeCadaMapa,hojaViaje[j] , cadaPoke);
							//printf("tengo que conseguir el Poke %s del mapa %s \n", cadaPoke, hojaViaje[j] );

							k++;
						} else {
							l=0;
						}
					} while (l);

					t_mapa* cosasMapa;
					cosasMapa = malloc(sizeof(t_mapa));

					char* configMapa = string_from_format("%s/Mapas/%s/metadata",puntoMontaje,hojaViaje[j]);
					//creamos el config para leer las cosas de mapa
					t_config* archivoMapa = config_create(configMapa);
						if (archivoMapa == NULL) {
							log_info(logs,"Error, no hay nada \n");
						} else {
						char* ip = string_new();
						string_append(&ip, config_get_string_value(archivoMapa,"IP"));
						(cosasMapa)->ipMapa= ip;
						//printf("%s \n", (cosasMapa)->ipMapa);
						(cosasMapa)->puertoMapa=config_get_int_value(archivoMapa,"Puerto");
						//printf("%d \n", (cosasMapa)->puertoMapa);
						}

					free(configMapa);
					config_destroy(archivoMapa);

					list_add(ips,(cosasMapa)->ipMapa);

					char* puertoChar = string_itoa((cosasMapa)->puertoMapa);
					list_add(puertos,puertoChar);

					// probar si funciona aca y sino lo cambio
					free(cosasMapa);
					free(objetivoDeMapa);
					free(objetivosMapa);

					j++;
				}
				else {
					i=0;
				}
			} while(i);

			(*datos)->cantidadInicialVidas= config_get_int_value(archivoConfiguracion,"vidas");
			//printf("vidas=%d \n",(*datos)->cantidadInicialVidas);

			(*datos)->reintentos= config_get_int_value(archivoConfiguracion,"reintentos");
			//printf("reintentos=%d \n",(*datos)->reintentos);

			free(mapa);
			free(poke);
			free(hojaViaje);
			config_destroy(archivoConfiguracion);

			return 1;
			}
	}
}


//para el list_iterate que uso en el main
void obtengoCadaUno(char* elemento){
		printf("%s \n",elemento);
		return;
}

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
	/*if (serverInfo == NULL) {
		printf("rompio aca \n");
		return -1;
	}*/
	while(serverInfo == NULL){
		serverInfo = cargarInfoSocket(IP, Port);
	}
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (serverSocket == -1) {
		printf("Error en la creacion del socket\n");
		return -1;
	}
	/*if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1) {
		printf("No se pudo conectar con el socket servidor\n");
		close(serverSocket);
		exit(-1);
	}*/
	while (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1){
		connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
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

/* Funcion que serializa una estructura paquete */
char *serializar(t_paquete *unPaquete) {
	void *buffer = malloc(
			sizeof(int)/*CodOp*/+ sizeof(int)/*ProgCounter*/+ sizeof(int)/*PidProceso*/
			+ sizeof(int)/*quantum*/+ sizeof(int)/*Tamañopath*/
			+ sizeof(char) * unPaquete->tamanio);
	memcpy(buffer, &unPaquete->codigoOperacion, sizeof(int));
	memcpy(buffer + sizeof(int), &unPaquete->programCounter, sizeof(int));
	memcpy(buffer + (sizeof(int) * 2), &unPaquete->pid, sizeof(int));
	memcpy(buffer + (sizeof(int) * 3), &unPaquete->quantum, sizeof(int));
	memcpy(buffer + (sizeof(int) * 4), &unPaquete->tamanio, sizeof(int));
	memcpy(buffer + (sizeof(int) * 5), &unPaquete->path, unPaquete->tamanio);
	return buffer;
}
/* deserializar elheader del buffer a la estructura paquete
 *  devuelve la direccion a la estructura Paquete */

t_paquete *deserializar_header(char *buffer) {
	t_paquete *contexto_ejecucion = malloc(sizeof(t_paquete));
	memcpy(&contexto_ejecucion->codigoOperacion, buffer, sizeof(int));
	memcpy(&contexto_ejecucion->programCounter, buffer + sizeof(int),
			sizeof(int));
	memcpy(&contexto_ejecucion->tamanio, buffer + sizeof(int) + sizeof(int),
			sizeof(int));

	return contexto_ejecucion;
}
/* deserializa la data del buffer con los datos recibidos en el deserializar_header */
void deserializar_data(t_paquete *unPaquete, char *buffer) {
	unPaquete->path = malloc(unPaquete->tamanio);
	memcpy(unPaquete->path, buffer, unPaquete->tamanio);
}
/* Funcion que genera un paquete. agarra los valores correspondientes y
 * los coloca dentro de la estructura Paquete */

t_paquete *generarPaquete(int codigoOperacion, int tamMessage, char *message,
		int programCounter, int quantum, int pid) {
	t_paquete * paquete = malloc(sizeof(t_paquete));

	paquete->codigoOperacion = codigoOperacion;
	paquete->programCounter = programCounter;
	paquete->pid = pid;
	paquete->quantum = quantum;
	paquete->tamanio = tamMessage;
	paquete->path = malloc(tamMessage);
	memcpy(paquete->path, message, paquete->tamanio);
	return paquete;
}
/* funcion para destruir paquete */
void destruirPaquete(t_paquete * unPaquete) {
	free(unPaquete->path);
	free(unPaquete);
}


