/*
 * libreriaPokedexServidor.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaPokedexServidor.h"

void imprimirGiladas(void *unCliente){

	t_infoCliente *infoCliente = (t_infoCliente *) unCliente;
	char paquete[1024];
	int status = 1;

	printf("PokeCliente #%d conectado! esperando mensajes... \n", infoCliente->cliente);

	while(status !=0){
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) printf("el PokeCliente #%d dijo: \n %s", infoCliente->cliente, paquete);
	}
}
