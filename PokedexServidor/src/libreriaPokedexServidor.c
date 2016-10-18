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
/* Función para remover un char en concreto de un string */
void removeChar(char *string, char basura) {
	char *src, *dst;
	for (src = dst = string; *src != '\0'; src++) {
		*dst = *src;
		if (*dst != basura)
			dst++;
	}
	*dst = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
char *getFileName(unsigned char *nombreArchivo){
	int i = 0;
	while(nombreArchivo[i] != "\0" && i <= 17){
		i++;
	}
	char *nombre = malloc(i);
	memcpy(nombre, nombreArchivo, i);

	return nombre;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *getNombreDirectorio(char *ruta){
	char *nombre;
	int i = 0;
	char **separador = string_split(ruta, "/");
	while(separador[i] != NULL){
		nombre = separador[i];
		i++;
	}

	return nombre;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *getDirectorioPadre(char *ruta){
	char *nombrePadre = string_new();
	int i = 0;
	char **separador = string_split(ruta, "/");
	int encontreNull = 0;
	while(encontreNull == 0){
		if(separador[i] == NULL){encontreNull = 1;}
		i++;
	}
	if(i >= 3){
		nombrePadre = separador[i - 3];
	}
	else{if(strlen(nombrePadre) <= 1){
		nombrePadre = NULL;}
	}
	return nombrePadre;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *getDirectorioAbuelo(char *ruta){
	char *nombreAbuelo = string_new();
	int i = 0;
	char **separador = string_split(ruta, "/");
	int encontreNull = 0;
	while(encontreNull == 0){
		if(separador[i] == NULL){encontreNull = 1;}
		i++;
	}
	if(i >= 4){
		nombreAbuelo = separador[i - 4];
	}
	else{if (strlen(nombreAbuelo) <= 1){
		nombreAbuelo = NULL;}
	}
	return nombreAbuelo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
t_infoDirectorio getInfoDirectorio(char *ruta){
	t_infoDirectorio directorio;
	directorio.nombre = getNombreDirectorio(ruta);
	directorio.padre = getDirectorioPadre(ruta);
	directorio.abuelo = getDirectorioAbuelo(ruta);
	directorio.largoNombre = strlen(directorio.nombre);
	if(directorio.padre != NULL){
	directorio.largoPadre = strlen(directorio.padre);
	}
	if(directorio.abuelo != NULL){
	directorio.largoAbuelo = strlen(directorio.abuelo);
	}
	return directorio;
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
char *osada_leerContenidoDirectorio(char *unaRuta){

	char *contenido = string_new();
	char *fname = string_new();
	char *nombreArchivo = string_new();
	char *nombrePadre = string_new();
	char *nombreAbuelo = string_new();
	char *separador = ";";
	int parentBlock = 999999;
	int i;
	int iPadre;
	int iAbuelo;
	t_infoDirectorio miDirectorio = getInfoDirectorio(unaRuta);


	for (i = 0; i <= 2048; i++){
		// Recorro la tabla buscando la estructura asociada al directorio que nos pasaron
		fname = getFileName(miDisco.tablaDeArchivos[i].fname);
		if(strncmp(fname, miDirectorio.nombre, miDirectorio.largoNombre) == 0){
			if(miDirectorio.padre == NULL && (int)miDisco.tablaDeArchivos[i].parent_directory == 65535){
				parentBlock = i;}

			// Me fijo si el directorio que me pasaron está alojado en otra carpeta padre, y valido
			else if (miDirectorio.padre != NULL){
				iPadre = miDisco.tablaDeArchivos[i].parent_directory;
				if(iPadre == 65535){nombrePadre = "root";}
				else {nombrePadre = getFileName(miDisco.tablaDeArchivos[iPadre].fname);}

				if(strncmp(nombrePadre, miDirectorio.padre, miDirectorio.largoPadre) == 0){
			// Me fijo si el directorio padre también está alojado a su vez en otro directorio
					if(miDirectorio.abuelo != NULL){
						iAbuelo = miDisco.tablaDeArchivos[iPadre].parent_directory;
						if(iAbuelo == 65535){nombreAbuelo = "root";}
						else{nombreAbuelo = getFileName(miDisco.tablaDeArchivos[iAbuelo].fname);}
						removeChar(nombreAbuelo, ' ');
						removeChar(miDirectorio.abuelo, ' ');
						if(strncmp(nombreAbuelo, miDirectorio.abuelo, miDirectorio.largoAbuelo) == 0){
							parentBlock = i;
						}
					}
					else {
						parentBlock = i;
					}
				}
			}
		}
	}
	if(parentBlock == 999999){
		printf("no existe el directorio o subdirectorio especificado\n");
		exit(0);
	}
	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2048; i++){
			if(miDisco.tablaDeArchivos[i].parent_directory == parentBlock){
				nombreArchivo = getFileName(miDisco.tablaDeArchivos[i].fname);
				if(strlen(nombreArchivo) > 0){
				string_append(&contenido, nombreArchivo);
				// Concateno los nombres en un string, separados por la variable separador.
				// Después la idea es separarlos y manejarlos como corresponda afuera de esta función
				string_append(&contenido, separador);
				}
			}
		}

char **contenidoDirectorio = string_split(contenido, ";");
int j;
printf("contenido del directorio:\n");
for (j = 0; contenidoDirectorio[j] != NULL; j++){
	printf("%s\n", contenidoDirectorio[j]);
}

return contenido;
free(fname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void *osada_leerArchivo(char *unArchivo){
	void *buffer;
	int tamanioBloque = 64;
	int tamanioBitmap = miDisco.header->bitmap_blocks;
	int inicioBloquesDeDatos = miDisco.header->fs_blocks - (miDisco.header->fs_blocks
								- miDisco.header->data_blocks);
	int tamanioTablaAsignaciones = ((miDisco.header->fs_blocks - 1 - tamanioBitmap) * 4)
									/ tamanioBloque;
	int i;
	tabla_asignaciones *tablaDeAsignaciones = malloc(tamanioTablaAsignaciones);
	// Me copio la tabla de asignaciones
	memcpy(tablaDeAsignaciones, discoMapeado[mainHeader.allocations_table_offset * 64],
			tamanioTablaAsignaciones);

	for(i = 0; i <= 2048; i++){
		//Busco en la tabla de archivos el que tiene el mismo nombre
		if(tablaDeArchivos[i].fname == unArchivo){

			buffer = malloc(tablaDeArchivos[i].file_size);
			int desplazamiento = 0;
			int siguienteBloque = (int)tablaDeAsignaciones[tablaDeArchivos[i].first_block];

			// Voy agregando al buffer los bloques de datos correspondientes al archivo, uno a uno

			while(tablaDeAsignaciones[siguienteBloque]){ //Valido así porque FFFFFFFF == -1
				memcpy(buffer + desplazamiento, &discoMapeado[siguienteBloque * 64], tamanioBloque);
				desplazamiento += 64;
			}

		}
	}

	free(tablaDeAsignaciones);

	return(buffer);
	free(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
