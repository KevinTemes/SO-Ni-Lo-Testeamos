/*
 * libreriaPokedexServidor.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaPokedexServidor.h"

#define MAX_LEN 128

#define KNORMAL "\x1B[0m"
#define KROJO "\x1B[31m"
#define KVERDE "\x1B[32m"
#define KAMARILLO "\x1B[33m"
#define KAZUL "\x1B[34m"
#define KMAGENTA "\x1B[35m"
#define KCYAN "\x1B[36m"
#define KBLANCO "\x1B[37m"



////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimirGiladas(void *unCliente){

	t_infoCliente *infoCliente = (t_infoCliente *) unCliente;
	char paquete[1024];
	int status = 1;

	void enviarAvisoDeCierre(){
		printf("\n");
		enviarHeader(infoCliente->socket, 9);
		printf("AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
		exit(0);
	}

	printf("PokeCliente #%d conectado! esperando mensajes... \n", infoCliente->cliente);

	signal(SIGINT, enviarAvisoDeCierre);

	while(status !=0){
		status = recv(infoCliente->socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el PokeCliente #%d dijo: \n %s", infoCliente->cliente, paquete);
			enviarHeader(infoCliente->socket, 1);
			}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void enviarHeader(int unSocket, int unHeader){
	char *recepcion = malloc(sizeof(int));
	memcpy(recepcion, &unHeader, sizeof(int));
	send(unSocket, recepcion, sizeof(int), 0);
	free(recepcion);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void imprimir_archivo(char *rutaDelArchivo){
	int c;
	FILE *file;
	file = fopen(rutaDelArchivo, "r");
	if (file) {
	    while ((c = getc(file)) != EOF)
	        putchar(c);
	    fclose(file);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *txtAString(char *rutaDelArchivo) {
	char * buffer = 0;
	long length;
	FILE * f = fopen(rutaDelArchivo, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}
	return buffer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void notificarCaida(){
	int i;
	for( i = 0; i <= 1024; i++){
		enviarHeader(clientesActivos[i].socket, 9);
	}
	printf("\n");
	printf("%sAVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n", KROJO);
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
	int unCliente = *((int *) numeroCliente);
	char paquete[1024];
	int status = 1;

	printf("%sPokeCliente #%d conectado! esperando mensajes... \n", KAMARILLO,
				clientesActivos[unCliente].cliente);

	while(status !=0){
		status = recv(clientesActivos[unCliente].socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			printf("el PokeCliente #%d dijo: \n %s", clientesActivos[unCliente].cliente, paquete);
			enviarHeader(clientesActivos[unCliente].socket, 1);
			}

	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void osada_leerContenidoDirectorio(char *unDirectorio, archivoOsada unArchivo){

	// Declaro variables
	unsigned char* directorio;
	strncpy((char *)directorio, unDirectorio);
	char *bloques = (malloc(sizeof(osada_file)) * 2048);
	osada_file tablaDeArchivos[2048];
	int inicioTablaDeArchivos;
	int tamanioBitmap;
	int tamanioTablaDeAsignaciones;
	int bloquesTablaDeAsignaciones;
	int bloquesDeDatos;

	// Asigno lo que haya que asignar

	archivoOsada archivo = unArchivo;
	osada_header header;
	osada(&header, &archivo);
	tamanioBitmap = (header.fs_blocks / 8 / 64);
	inicioTablaDeArchivos = 1 + tamanioBitmap;
	tamanioTablaDeAsignaciones = header.fs_blocks - 1 - tamanioBitmap - 1024;
	bloquesTablaDeAsignaciones = tamanioTablaDeAsignaciones / 64;
	bloquesDeDatos = header.data_blocks;

	//Me muevo al inicio del bloque de archivos
	//y copio el bloque en mi variable (REVISAR! No estoy seguro de que esta parte esté bien)
	seekBloques(&archivo, inicioTablaDeArchivos);
	memcpy(bloques, &archivo, sizeof(osada_file)*2048);

	tablaDeArchivos = (char *)bloques;

	// Me muevo al inicio del bloque de la tabla de asignaciones, y me la copio
	seekBloques(&archivo, header.allocations_table_offset);
	char *asignaciones = malloc(tamanioTablaDeAsignaciones);
	memcpy(asignaciones, &archivo, tamanioTablaDeAsignaciones);
	int tablaDeAsignaciones[bloquesDeDatos] = (char *)asignaciones; // Revisar también este casteo!

	// Me muevo al inicio del bloque de datos y lo copio

	/* NOTA: esta solución es la "maso maso" que decian los ayudantes durante la charla;
	 * lo que habria que hacer acá es lo de mmap(), para no
	 * consumir memoria al cuete (en los pasos de arriba creo que también
	 * se puede hacer), pero no estoy muy canchero con eso todavia
	 * (y me tengo que ir en media hora :P ) asi que va esta negrada, por ahora...
	 * */

	seekBloques(&archivo, (header.allocations_table_offset + bloquesTablaDeAsignaciones));
	char *datos = malloc(header.data_blocks * 64);
	memcpy(datos, &archivo, header.data_blocks * 64);
	int tablaDeDatos[bloquesDeDatos] = (char *)datos;


	// Recorro la tabla, busco el directorio en cuestión, y copio el contenido
	int i;
	for(i = 0; i <= 2048; i++){
		if(tablaDeArchivos[i].fname == unDirectorio){
			char *contenido = malloc(tablaDeArchivos[i].file_size);
			/* Acá habría que hacer lo siguiente: revisar el campo
			 * tablaDeArchivos[i].first_block para saber el N° del primer bloque de datos
			 * del archivo. voy a la tabla de bloques de datos, copio el contenido de ese bloque
			 * en un buffer, y me muevo al siguiente bloque, cuyo N° va a ser el que esté en la
			 * posición de la tabla de asignaciones que leimos antes. Agregamos el contenido de
			 * ese bloque al buffer, y repetimos lo de buscar-siguiente-bloque,
			 * copiar-contenido-del bloque, hasta llegar al final. Devolvemos el contenido
			 * total del buffer(cambiar el tipo de la función!), LO LIBERAMOS DESPUES, y listo.
			 * */

		}
	}

	free(bloques);
	free(asignaciones);
	free(datos);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
