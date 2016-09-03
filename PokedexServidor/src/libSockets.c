/*
 * libSockets.C
 *
 *  Created on: 1/9/2016
 *      Author: utnso
 */

#include "libSockets.h"
#define HEADER_PAQUETE (sizeof(int)*3)

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

/* Funcion que serializa una estructura paquete */
char *serializar(Paquete *unPaquete) {
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

Paquete *deserializar_header(char *buffer) {
	Paquete *contexto_ejecucion = malloc(sizeof(Paquete));
	memcpy(&contexto_ejecucion->codigoOperacion, buffer, sizeof(int));
	memcpy(&contexto_ejecucion->programCounter, buffer + sizeof(int),
			sizeof(int));
	memcpy(&contexto_ejecucion->tamanio, buffer + sizeof(int) + sizeof(int),
			sizeof(int));

	return contexto_ejecucion;
}
/* deserializa la data del buffer con los datos recibidos en el deserializar_header */
void deserializar_data(Paquete *unPaquete, char *buffer) {
	unPaquete->path = malloc(unPaquete->tamanio);
	memcpy(unPaquete->path, buffer, unPaquete->tamanio);
}
/* Funcion que genera un paquete. agarra los valores correspondientes y
 * los coloca dentro de la estructura Paquete */

Paquete *generarPaquete(int codigoOperacion, int tamMessage, char *message,
		int programCounter, int quantum, int pid) {
	Paquete * paquete = malloc(sizeof(Paquete));

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
void destruirPaquete(Paquete * unPaquete) {
	free(unPaquete->path);
	free(unPaquete);
}


