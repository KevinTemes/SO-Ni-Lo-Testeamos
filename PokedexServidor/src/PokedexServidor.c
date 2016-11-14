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

#define KNORMAL "\x1B[0m"
#define KROJO "\x1B[31m"
#define KVERDE "\x1B[32m"
#define KAMARILLO "\x1B[33m"
#define KAZUL "\x1B[34m"
#define KMAGENTA "\x1B[35m"
#define KCYAN "\x1B[36m"
#define KBLANCO "\x1B[37m"

#define BACKLOG 100
/* para testear sockets */
#define PACKAGESIZE 1024
#define PUERTO "7777"
t_log* logs;

int main(int argc, char **argv) {





	//LOGS
	remove("PokeServidor.log");
	puts("Creando archivo de logueo PokeServidor...\n");
	logs = log_create("PokeServidor.log", "PokedexServidor", true, log_level_from_string("INFO"));
	puts("Log Pokedex Servidor creado exitosamente \n");

	//Levanto el disco Osada
	 miDisco = osada_iniciar();
	 int fd_disco;
	 struct stat discoStat;
	 fd_disco = open("/home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/challenge.bin", O_RDWR);
	 fstat(fd_disco, &discoStat);
	 miDisco.discoMapeado = mmap(0, discoStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_disco, 0);



		// ZONA DE TESTEO
/*		printf("ingrese la accion a testear, o escriba 'ayuda' para ver comandos disponibles:\n");
		char *accion = malloc(sizeof(char) * 32);
		scanf("%[^\n]%*c", accion);
		char *algo = accion;
		while(strcmp(algo,"exit")!=0){
		test_funcionalidad(algo);
		printf("ingrese la accion a testear, o escriba 'ayuda' para ver comandos disponibles:\n");
		scanf("%[^\n]%*c", accion);
		};
		// FIN ZONA DE TESTEO
*/





	signal(SIGINT, notificarCaida);

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
	t_infoCliente unCliente;
	int n = 0;
	int *numeroCliente;
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


				unCliente.cliente = cliente;
				unCliente.socket = socketCliente;
				clientesActivos[n] = unCliente;
				int nroCliente = n++;
				numeroCliente = malloc(sizeof(int));
				numeroCliente = &nroCliente;

				pthread_create(&hiloAtenderConexiones[n], NULL, atenderConexion, numeroCliente);

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

