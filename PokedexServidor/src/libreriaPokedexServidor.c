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
char *osada_leerContenidoDirectorio(char *unDirectorio){

	char *contenido = string_new();
	char *separador = ";";
	int tamanioBloque = 64;
	int *disco;
	int tamanioHeader = 1;
	int tamanioBitmap = mainHeader.bitmap_blocks;
	int directorioPadre;
	int i;

	// Puede que acá haya que agregar un if() adicional para el caso del directorio raíz

	for (i = 0; i <= 2048; i++){
		// Recorro la tabla buscando la estructura asociada al directorio que nos pasaron
		if(tablaDeArchivos[i].fname == unDirectorio){
				directorioPadre = i;
		};
	}
	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2048; i++){
			if(tablaDeArchivos[i].parent_directory == directorioPadre){
				string_append(&contenido, tablaDeArchivos[i].fname);
				// Concateno los nombres en un string, separados por la variable separador.
				// Después la idea es separarlos y manejarlos como corresponda afuera de esta función
				string_append(&contenido, separador);
			}
		}

free(tablaDeArchivos);
return contenido;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void *osada_leerArchivo(char *unArchivo){
	void *buffer;
	int tamanioBloque = 64;
	int tamanioBitmap = mainHeader.bitmap_blocks;
	int inicioBloquesDeDatos = mainHeader.fs_blocks - (mainHeader.fs_blocks - mainHeader.data_blocks);
	int tamanioTablaAsignaciones = ((mainHeader.fs_blocks - 1 - tamanioBitmap) * 4)
									/ tamanioBloque;
	int i;
	tabla_Asignaciones *tablaDeAsignaciones = malloc(tamanioTablaAsignaciones);
	// Me copio la tabla de asignaciones
	memcpy(tablaDeAsignaciones, discoMapeado[mainHeader.allocations_table_offset * 64],
			tamanioTablaAsignaciones);

	for(i = 0; i <= 2048; i++){
		//Busco en la tabla de archivos el que tiene el mismo nombre
		if(tablaDeArchivos[i].fname == unArchivo){

			buffer = malloc(tablaDeArchivos[i].file_size);
			int desplazamiento = 0;
			int siguienteBloque = tablaDeAsignaciones[tablaDeArchivos[i].first_block];

			// Voy agregando al buffer los bloques de datos correspondientes al archivo, uno a uno

			while(tablaDeAsignaciones[siguienteBloque]){ //Valido así porque FFFFFFFF == -1
				memcpy(buffer + desplazamiento, discoMapeado[siguienteBloque * 64], tamanioBloque);
				desplazamiento += 64;
			}

		}
	}
	free(tablaDeArchivos);
	free(tablaDeAsignaciones);

	return(buffer);
	free(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
