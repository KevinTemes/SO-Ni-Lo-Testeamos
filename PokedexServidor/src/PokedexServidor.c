/*
 ============================================================================
 Name        : PokedexServidor.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "PokedexServidor.h"
#include <pthread.h>


#define BACKLOG 100
/* para testear sockets */
#define PACKAGESIZE 1024
#define PUERTO "7777"
t_log* logs;

int main() {

	//LOGS
	remove("PokeServidor.log");
	puts("Creando archivo de logueo PokeServidor...\n");
	logs = log_create("PokeServidor.log", "PokedexServidor", true, log_level_from_string("INFO"));
	puts("Log Pokedex Servidor creado exitosamente \n");


	//carga de variables

	osada_header head;
	osada_file archi;


	osada(&head,&archi);



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
				listen(socketEscucha, BACKLOG);
				struct sockaddr_in addr;
				socklen_t addrlen = sizeof(addr);
				int socketCliente = accept(socketEscucha,
				(struct sockaddr *) &addr, &addrlen);

				infoCliente = malloc(sizeof(t_infoCliente));
				infoCliente->cliente = cliente;
				infoCliente->socket = socketCliente;

	/*			unCliente.cliente = cliente;
				unCliente.socket = socketCliente;
				clientesActivos[n] = unCliente;
				numeroCliente = malloc(sizeof(int));
				numeroCliente = &n; */

				pthread_create(&hiloImprimirGiladas[n],NULL, imprimirGiladas, infoCliente);
			//	pthread_create(&hiloAtenderConexiones[n], NULL, atenderConexion, numeroCliente);

				cliente++;
				n++;

				fileDescriptors[cantfds].fd = socketCliente;
				fileDescriptors[cantfds].events = POLLIN;
				cantfds++;

				goto llamadaPoll;
			}
		}
	}

close(socketEscucha);

return 0;

}

