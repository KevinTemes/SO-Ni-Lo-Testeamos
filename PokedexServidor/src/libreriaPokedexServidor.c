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
void *serializarString(char *unString){
	int tamanioString = sizeof(char) * strlen(unString);
	void *buffer = malloc(sizeof(int) + tamanioString);
	memcpy(buffer, &tamanioString, sizeof(int));
	memcpy(buffer + sizeof(int), &unString, tamanioString);

	return buffer;
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

int buscarArchivo(char *unaRuta){
	t_infoDirectorio miDirectorio = getInfoDirectorio(unaRuta);
	int posicion = 0;
	char *fname = string_new();
	char *nombrePadre = string_new();
	char *nombreAbuelo = string_new();
	int parentBlock = 999999;
	int i;
	int iPadre;
	int iAbuelo;

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

	if (parentBlock != 999999){
		posicion = parentBlock;
	}
	else {
		posicion = -1;
	}

	return posicion;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void copiarBloque(void *buffer, int bloque, int offset){
	void *puente = malloc(64);
	int inicioDatos = miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap
			+ miDisco.cantBloques.bloques_tablaDeArchivos
			+ miDisco.cantBloques.bloques_tablaDeAsignaciones;
	seekBloques(miDisco.disco, inicioDatos + bloque);
	fread(puente, 64, 1, miDisco.disco);
	memcpy(buffer + offset, puente, 64);
	fseek(miDisco.disco, 0, SEEK_SET);
	free(puente);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
	int unCliente = *((int *) numeroCliente);
	char paquete[1024];
	int status = 1;
	int codOp;
	int tamanioRuta, tamanioNuevoContenido, tamanioNombre;;
	char *ruta;
	void *buffer;

	printf("%sPokeCliente #%d conectado! esperando mensajes... \n", KAMARILLO,
				clientesActivos[unCliente].cliente);

	while(status !=0){

		//status = recv(clientesActivos[unCliente].socket, (void*) paquete, 1024, 0);
		if (status != 0) {
			recv(clientesActivos[unCliente].socket, &codOp, sizeof(int), MSG_WAITALL);

			switch(codOp){

			case 0: // .getAttr

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer; // chequear este casteo y los proximos que sean iguales
				// int tipoArchivo = osada_getAttr(ruta);
				// send(clientesActivos[unCliente].socket, tipoArchivo, sizeof(int) + tamanioContenido, 0);
				// free(buffer);

			break;

			case 1: // .readdir (- ls)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				char *contenido = osada_readdir(ruta);
				int tamanioContenido = sizeof(char) * strlen(contenido);
				void *bufferDir = serializarString(contenido);
				send(clientesActivos[unCliente].socket, bufferDir, sizeof(int) + tamanioContenido, 0);
				free(buffer);
				free(bufferDir);

			break;

			case 2: // .read

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				void *buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				void *contenidoArchivo = osada_read(ruta);
				int tamanioArchivo; // ver después cómo obtener este valor;
				buffer = malloc(tamanioArchivo + sizeof(int));
				// serializarArchivo(buffer, contenidoArchivo, tamanioArchivo);
				send(clientesActivos[unCliente].socket, buffer, sizeof(int) + tamanioContenido, 0);
				free(buffer);

			break;

			case 3: // crear archivo (por ahora supungo que el nombre ya viene en la ruta)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_crearArchivo(ruta);
				// send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 4: // modificar archivo


				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &tamanioNuevoContenido, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				void *bufferContenido = malloc(tamanioNuevoContenido);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				recv(clientesActivos[unCliente].socket, bufferContenido,
						tamanioNuevoContenido, MSG_WAITALL);
				//int exito = osada_write(ruta, bufferContenido);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);
				free(bufferContenido);

			break;

			case 5: // borrar archivo (unlink)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_unlink(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 6: // Crear directorio/subdirectorio (mkdir)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_mkdir(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 7: // Borrar directorio (rmdir)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_rmdir(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 8: // Renombrar archivo (rename)


				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &tamanioNombre, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				void *bufferNombre = malloc(tamanioNombre);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, bufferNombre, tamanioNombre, MSG_WAITALL);
				ruta = (char *) buffer;
				char *nombre = (char *) bufferNombre;
				// int exito = osada_rename(ruta, nombre);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);
				free(bufferNombre);

			break;

			}

		}

	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_getattr(char *unaRuta){
	int tipoArchivo;
	int posicion = buscarArchivo(unaRuta);
	if(miDisco.tablaDeArchivos[posicion].state == REGULAR){
		tipoArchivo = 1;
	}
	else if(miDisco.tablaDeArchivos[posicion].state == DIRECTORY){
		tipoArchivo = 2;
	}

	return tipoArchivo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
char *osada_readdir(char *unaRuta){

	char *contenido = string_new();
	char *fname = string_new();
	char *nombreArchivo = string_new();
	char *separador = ";";
	int parentBlock = buscarArchivo(unaRuta);
	int i;

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
void *osada_read(char *ruta){

	int i = buscarArchivo(ruta);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	void *buffer = malloc(miDisco.tablaDeArchivos[i].file_size);
	int tamanioActualBuffer = 0;

	while(miDisco.tablaDeAsignaciones[siguienteBloque] != 65535){
		copiarBloque(buffer, siguienteBloque, tamanioActualBuffer);
		tamanioActualBuffer += 64;
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

	}

	char *imprimir = (char *)buffer;
;	printf("%s\n", imprimir);
	return(buffer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_crearArchivo(char *ruta){
	int exito;
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_editaArchivo(char *ruta, void *nuevoContenido){
	int exito;
	exito = 0;
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_unlink(char *ruta){
	int exito;
	int i = buscarArchivo(ruta);
	miDisco.tablaDeArchivos[i].state = '\0';
	//actualizarTablaDeArchivos(miDisco);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	while(siguienteBloque != 65535){
		bitarray_set_bit(miDisco.bitmap, siguienteBloque);
		//actualizarBitmap(miDisco);
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
	}
	exito = 0; // hay chances de error? validar
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_mkdir(char *ruta, char nombreDir){
	int exito;
	exito = 0;
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rmdir(char *ruta){
	int exito;
	int i = buscarArchivo(ruta);
		miDisco.tablaDeArchivos[i].state = '\0';
		//actualizarTablaDeArchivos(miDisco);
		int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
		while(siguienteBloque != 65535){
			bitarray_set_bit(miDisco.bitmap, siguienteBloque);
			//actualizarBitmap(miDisco);
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
		}
		exito = 0; // hay chances de error? validar
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rename(char *ruta, char *nuevoNombre){
	int exito;
	int i = buscarArchivo(ruta);
	strcpy(miDisco.tablaDeArchivos[i].fname, nuevoNombre);
	//actualizarTablaDeArchivos(miDisco);
	exito = 0;
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
