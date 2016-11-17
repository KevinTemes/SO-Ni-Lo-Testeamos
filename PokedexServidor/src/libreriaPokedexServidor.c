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

pthread_mutex_t mutexOsada=PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////////////////////////////
void *serializarString(char *unString){
	int tamanioString = sizeof(char) * strlen(unString);
	void *buffer = malloc(sizeof(int) + tamanioString);
	memcpy(buffer, &tamanioString, sizeof(int));
	memcpy(buffer + sizeof(int), &unString, tamanioString);

	return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

int buscarArchivo(char *unaRuta){
	int parentDir = 65535;
//	int existeArchivo = 0;
	if(strlen(unaRuta) == 1){
		goto terminar;
	}

	else{
		char **separadas = string_split(unaRuta, "/");
		int a = 0;

		while(separadas[a] != NULL){
			parentDir = recorrerDirectorio(separadas[a], parentDir);
			if(parentDir == -1){
				goto terminar;
			}
			a++;
		}
		free(separadas);
	}
terminar:

return parentDir;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int recorrerDirectorio(char *nombre, int parentDir){
	int posicion, i;
	posicion = -1;
	char *archivo = nombre;
	if(strcmp(archivo, "")== 0){
			goto finalizar;
		}
	int nameLength = strlen(archivo);

	for(i = 0; i <= 2047; i++){
		if(miDisco.tablaDeArchivos[i].state != DELETED){
			if(nameLength == 17){
				if((strncmp(archivo, miDisco.tablaDeArchivos[i].fname, 16) == 0) &&
								((int)miDisco.tablaDeArchivos[i].parent_directory == parentDir)){
									posicion = i;

							}
			}
			else{
				if((strcmp(archivo, miDisco.tablaDeArchivos[i].fname) == 0) &&
						((int)miDisco.tablaDeArchivos[i].parent_directory == parentDir)){
						posicion = i;

				}
			}

			//*fname = '\0';
			//  memset(fname, 0, strlen(fname) * sizeof(fname[0]));
			//free(fname);

		}

	}

	//free(fname);
	finalizar:
	return posicion;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *obtenerNombre(char *unaRuta){
	char *nombreArchivo = string_new();
	char **separador = string_split(unaRuta, "/");
	char nullChar[1];
	nullChar[0] = '\0';
	int n = 0;
	while(separador[n] != NULL){
		if(separador[n + 1] == NULL){
			string_append(&nombreArchivo, separador[n]);
			string_append(&nombreArchivo, nullChar);
		}
		n++;
	}
	return nombreArchivo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void copiarBloque(void *buffer, int bloque, int offset){

	int inicioDatos = (miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap
			+ miDisco.cantBloques.bloques_tablaDeArchivos
			+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;
	memcpy(buffer + offset, &miDisco.discoMapeado[inicioDatos + (bloque * 64)], 64);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void copiarBloqueIncompleto(void *buffer, int bloque, int offset, int tamanio){

	int inicioDatos = (miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap
				+ miDisco.cantBloques.bloques_tablaDeArchivos
				+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;
	memcpy(buffer + offset, &miDisco.discoMapeado[inicioDatos + (bloque * 64)], tamanio);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void *serializarArchivo(void* buffer, void* contenidoArchivo, int tamanioArchivo){
	memcpy(buffer, &tamanioArchivo, sizeof(int));
	memcpy(buffer + sizeof(int), &contenidoArchivo, tamanioArchivo);

	return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
int obtenerTamanioArchivo(char* ruta){
	int posicionArchivo = buscarArchivo(ruta);
	int tamanio = miDisco.tablaDeArchivos[posicionArchivo].file_size;
	return tamanio;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t consultarTiempo(){
	time_t ahora = time(NULL);
	struct tm tiem;
	tiem = *localtime(&ahora);

	int segundos = tiem.tm_sec;
	int minutos = tiem.tm_min;
	int horas = tiem.tm_hour;
	int dia = tiem.tm_mday;
	int mes = tiem.tm_mon + 1;
	//int anio = tiem.tm_year + 1900;

	uint32_t stamp = (mes*100000000)+(dia*1000000)+(horas*10000)+(minutos*100)+segundos;
	return stamp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void iterarNombre(char* origen, unsigned char respuesta[17]){
	int i;
		for (i = 0; i <= 17; i++ ){
			respuesta[i] = origen[i];
			if (i == 17){
				respuesta[i] = '\0';
			}
		}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void iterarNombreAlReves(char* origen, unsigned char respuesta[17]){
	int i;
		for (i = 0; i <= 17; i++ ){
			origen[i] = respuesta[i];
			if (i == 17){
				respuesta[i] = '\0';
			}
		}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void llenarEstructuraNuevo(osada_file archivo, char* ruta, int bloqueTablaAsignacionesLibre){ // revisar lo del char[17]

//	t_infoDirectorio directorio = getInfoDirectorio(ruta); // Ver que onda con esta estructura
	uint32_t tiempo = consultarTiempo();

	archivo.state = '\1'; // Regular
//	iterarNombre(directorio.nombre,archivo.fname); // Hay que buscar una solucion mas copada a esto
//	archivo.parent_directory = directorio.padre;
	archivo.file_size = 0 ; // Se crean vacios
	archivo.lastmod = tiempo;
	archivo.first_block = bloqueTablaAsignacionesLibre;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
char *convertirString(void *buffer, int tamanioRuta){
	char *nuevaRuta = string_new();
	char *puente = string_new();
	puente = (char *)buffer;
	nuevaRuta = string_substring_until(puente, tamanioRuta);
	nuevaRuta[tamanioRuta] = '\0';
	return nuevaRuta;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
	int unCliente = *((int *) numeroCliente);
	int status = 1;
	int codOp, exito;
	int tamanioRuta, tamanioNombre;
	size_t tamanioNuevoContenido, nuevoTamanio;
	off_t offset;
	char *ruta = string_new();
	char *contenidoDir = string_new();
	char *nombre = string_new();
	void *contenido;
	void *buffer, *bufferDir, *nuevoContenido;
	void *respuesta;

	printf("PokeCliente #%d conectado! esperando solicitudes... \n",
			clientesActivos[unCliente].cliente);

	while(status !=0){

		if (status != 0) {
			recv(clientesActivos[unCliente].socket, &codOp, sizeof(int), MSG_WAITALL);

			switch(codOp){

			case 0: // .getAttr

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				t_getattr archivo = osada_getattr(ruta);
				respuesta = malloc(2 * sizeof(int));
				memcpy(respuesta, &archivo.tipo_archivo, sizeof(int));
				memcpy(respuesta + sizeof(int), &archivo.size, sizeof(int));
				send(clientesActivos[unCliente].socket, respuesta, 2 * sizeof(int), MSG_WAITALL);
				*ruta = '\0';
				free(buffer);

			break;

			case 1: // .readdir (- ls)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				contenidoDir = osada_readdir(ruta);
				int tamanio = strlen(contenidoDir);
				if(tamanio == 0){
					bufferDir = malloc(sizeof(int));
					memcpy(bufferDir, &tamanio, sizeof(int));
					send(clientesActivos[unCliente].socket, bufferDir, sizeof(int), 0);
				}
				else{
					bufferDir = malloc(tamanio + sizeof(int));
					memcpy(bufferDir, &tamanio, sizeof(int));
					memcpy(bufferDir + sizeof(int), contenidoDir, tamanio);
					send(clientesActivos[unCliente].socket, bufferDir, sizeof(int) + tamanio, 0);
				}
				*ruta = '\0';
				free(buffer);


			break;

			case 2: // .read

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				int j = buscarArchivo(ruta);
				int tamanioArchivo = (int)miDisco.tablaDeArchivos[j].file_size;
				if(tamanioArchivo == 0){
					buffer = malloc(sizeof(int));
					memcpy(buffer, &tamanioArchivo, sizeof(int));
					send(clientesActivos[unCliente].socket, buffer, sizeof(int), 0);
				}
				else{
					void *contenidoArchivo = osada_read(ruta);
					buffer = malloc(tamanioArchivo + sizeof(int));
					memcpy(buffer, &tamanioArchivo, sizeof(int));
					memcpy(buffer + sizeof(int), contenidoArchivo, tamanioArchivo);
					send(clientesActivos[unCliente].socket, buffer, sizeof(int) + tamanioArchivo, 0);
					free(contenidoArchivo);
				}
				*ruta = '\0';
				free(buffer);
			break;

			case 3: // .create (por ahora supongo que el nombre ya viene en la ruta)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_create(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
				//free(buffer);

			break;

			case 4: // .write


				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &tamanioNuevoContenido, sizeof(size_t), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				nuevoContenido = malloc(tamanioNuevoContenido);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, nuevoContenido, tamanioNuevoContenido, MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &offset, sizeof(off_t), MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_write(ruta, nuevoContenido, tamanioNuevoContenido, offset);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);

			break;

			case 5: // .unlink

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_unlink(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
				free(buffer);

			break;

			case 6: // .mkdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_mkdir(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
				free(buffer);

			break;

			case 7: // .rmdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_rmdir(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
				free(buffer);

			break;

			case 8: // .rename

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &tamanioNombre, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				void *bufferNombre = malloc(tamanioNombre);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, bufferNombre, tamanioNombre, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				nombre = convertirString(bufferNombre, tamanioNombre);
				exito = osada_rename(ruta, nombre);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
				free(buffer);
				free(bufferNombre);

			break;

			case 9: // .open

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_open(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), 0);
			//	free(buffer);

			break;

			case 10: // .truncate

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				recv(clientesActivos[unCliente].socket, &nuevoTamanio, sizeof(size_t), MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_truncate(ruta, nuevoTamanio);



			}
			//free(buffer);
		}

	}
	free(contenido);
	free(nuevoContenido);
	free(bufferDir);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarBitmap(){
	int inicioBitmap = miDisco.cantBloques.bloques_header * 64;
	int tamanioBitmap = miDisco.cantBloques.bloques_bitmap * 64;
	memcpy(miDisco.discoMapeado + inicioBitmap, miDisco.bitmap, tamanioBitmap);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarTablaDeArchivos(){
	int inicioTabla = ((miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap) * 64);
	memcpy(miDisco.discoMapeado + inicioTabla, &miDisco.tablaDeArchivos, sizeof(osada_file) * 2047);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarTablaDeAsignaciones(){
	int inicioTabla = ((miDisco.cantBloques.bloques_header +miDisco.cantBloques.bloques_bitmap +
			miDisco.cantBloques.bloques_tablaDeArchivos) * 64);
	int tamanioTabla = miDisco.cantBloques.bloques_tablaDeAsignaciones * 64;
	memcpy(miDisco.discoMapeado + inicioTabla, miDisco.tablaDeAsignaciones, tamanioTabla);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int redondearDivision(unsigned int dividendo, unsigned int divisor){
	return (dividendo + (divisor / 2) / divisor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int primerBloqueBitmapLibre(){
	int pos = -1;

	int bloquesDeDatos = miDisco.header->data_blocks;

	int i;
	for(i = 0; i <= bloquesDeDatos; i++){
		if(!bitarray_test_bit(miDisco.bitmap, i)){
			pos = i;
			break;
		}
	}


	return pos;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int primerBloqueTablaAsignacionesLibre(){
	unsigned int nroBloque = 0;
	while(miDisco.tablaDeAsignaciones[nroBloque] != NULL){ // revisar
		nroBloque++;
	}
	return nroBloque;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int  asignarPosicionTablaArchivos(osada_file archivo){
	int i, posAsignada = -1;
	for (i=0; i <=2047; i++){
		if(miDisco.tablaDeArchivos[i].state == DELETED){
			miDisco.tablaDeArchivos[i] = archivo;
			posAsignada = i;
			break;
		}
	}
	return posAsignada;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int buscarPosicionLibre(){
	int pos = -1;
	int i;
	for(i = 0; i<= 2047; i++){
		if(miDisco.tablaDeArchivos[i].state == DELETED){
			pos = i;
			break;
		}
	}
	return pos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

t_getattr osada_getattr(char *unaRuta){
	t_getattr atributos;
	int posicion = buscarArchivo(unaRuta);
	if (posicion == 65535){
		atributos.tipo_archivo = 2;
		atributos.size = 0;
	}
	else if(posicion == -1){
		atributos.tipo_archivo = 0;
		atributos.size = 0;
	}
	else{
		if(miDisco.tablaDeArchivos[posicion].state == REGULAR){
			atributos.tipo_archivo = 1;
			atributos.size = (int) miDisco.tablaDeArchivos[posicion].file_size;
		}
		else if(miDisco.tablaDeArchivos[posicion].state == DIRECTORY){
			atributos.tipo_archivo = 2;
			atributos.size = 0;
		}
	}

	return atributos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
char *osada_readdir(char *unaRuta){

	char *contenidoDir = string_new();
	char *separador = ";";
	char nullChar[1];
	nullChar[0] = '\0';
	int parentBlock = buscarArchivo(unaRuta);
	int numElems = 0;
	int i;

	if(parentBlock == 999999){
		printf("no existe el directorio o subdirectorio especificado\n");
		exit(0);
	}
	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2047; i++){

			if(miDisco.tablaDeArchivos[i].parent_directory == parentBlock
					&& miDisco.tablaDeArchivos[i].state != DELETED){

				string_append(&contenidoDir, miDisco.tablaDeArchivos[i].fname);
				string_append(&contenidoDir, nullChar);

				// Concateno los nombres en un string, separados por la variable separador.

				string_append(&contenidoDir, separador);
				string_append(&contenidoDir, nullChar);
				numElems++;

			}

		}

	if(numElems == 0){
		string_append(&contenidoDir, nullChar);
	}

return contenidoDir;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_open(char *ruta){
	int exito = 0;
	t_getattr archivo = osada_getattr(ruta);

	if(archivo.tipo_archivo != 0){
		exito = 1;
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void *osada_read(char *ruta){

	t_log* logRead;
	remove("osada_read.log");
	logRead = log_create("osada_read.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));

	int i = buscarArchivo(ruta);
	log_info(logRead, "Recibida solicitud de lectura (.read) del archivo %s", ruta);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	if(miDisco.tablaDeArchivos[i].file_size == 0){
		goto terminar;
	}
	void *buffer = malloc(miDisco.tablaDeArchivos[i].file_size);
	div_t bloquesOcupados = div(miDisco.tablaDeArchivos[i].file_size, 64);
	int tamanioActualBuffer = 0;
	int inicioDatos = (miDisco.cantBloques.bloques_header
				+ miDisco.cantBloques.bloques_bitmap
				+ miDisco.cantBloques.bloques_tablaDeArchivos
				+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;
	void *desplazamiento;
	if(bloquesOcupados.rem == 0){
		while(siguienteBloque != -1){
			desplazamiento = &miDisco.discoMapeado[inicioDatos + (siguienteBloque) * 64];
			memcpy(buffer + tamanioActualBuffer, desplazamiento, 64);

			tamanioActualBuffer += 64;
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
		}
	}
	else{
		int bloquesCopiados = 0;
		while(siguienteBloque != -1){
			if(bloquesCopiados < bloquesOcupados.quot){
				desplazamiento = &miDisco.discoMapeado[inicioDatos + (siguienteBloque * 64)];
				memcpy(buffer + tamanioActualBuffer,desplazamiento, 64);
				tamanioActualBuffer += 64;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				bloquesCopiados++;
			}
			else{
				desplazamiento = &miDisco.discoMapeado[inicioDatos + (siguienteBloque * 64)];
				memcpy(buffer + tamanioActualBuffer, desplazamiento, bloquesOcupados.rem);
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				tamanioActualBuffer += bloquesOcupados.rem;
				bloquesCopiados++;
			}

		}
	}

	//log_info(logRead, "Cantidad de bytes leidos: %d", tamanioActualBuffer);
	//char *epifania = buffer;
	//printf("%s\n", epifania);

	terminar:
	return(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_create(char *ruta){

	t_log* logCreate;
	remove("osada_create.log");
	logCreate = log_create("osada_create.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	log_info(logCreate, "recibida solicitud de creación (.create) del archivo %s", ruta);
	int exito = 1;

	unsigned int bloqueLibre = primerBloqueBitmapLibre();
	char *nombreArchivo = obtenerNombre(ruta);
	int largo = strlen(nombreArchivo);
	int posTablaArchivos = buscarPosicionLibre();
	if(bloqueLibre >= 0 && posTablaArchivos >= 0 && largo <= 17){
		int largoRuta = strlen(ruta);
		char *dirPadre = string_substring_until(ruta, largoRuta - largo);
		int parentDir = buscarArchivo(dirPadre);
		pthread_mutex_lock(&mutexOsada);
		crearArchivo(nombreArchivo, parentDir, bloqueLibre, posTablaArchivos);
		pthread_mutex_unlock(&mutexOsada);
		exito = 0;
	}

	if(exito == 0){
	log_info(logCreate, "El archivo %s se creo exitosamente", ruta);
	}
	else{
		if(bloqueLibre < 0){
			log_error(logCreate, "error al crear el archivo %s: espacio en disco insuficiente", ruta);
		}
		if(posTablaArchivos < 0){
			log_error(logCreate, "error al crear el archivo %s: se ha alcanzado el número máximo de"
					"archivos soportados por el sistema", ruta);
		}
		if(largo > 17){
			log_error(logCreate, "error al crear el archivo %s: el nombre del archivo no es válido"
					"(longitud superior a 17 caracteres)", ruta);
		}
		log_error(logCreate, "No se pudo crear el archivo");
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_write(char *ruta, void *nuevoContenido, size_t sizeAgregado, off_t offset){

	t_log* logWrite;
	remove("osada_write.log");
	logWrite = log_create("osada_write.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	log_info(logWrite, "Comienza el write sobre el archivo %s", ruta);
    int exito;
    exito = 0;

    int posicionArchivo = buscarArchivo(ruta);
    osada_file archivo = miDisco.tablaDeArchivos[posicionArchivo];
    int sizeOriginal = archivo.file_size;
    int siguienteBloque = archivo.first_block;
    void *contenidoOriginal = osada_read(ruta);
    int sizeFinal = offset + sizeAgregado;
    div_t bloquesOriginal = div(sizeOriginal,64);
    div_t bloquesAgregado = div(sizeAgregado, 64);
    div_t bloquesNecesarios = div(sizeFinal,64);
    int bloquesRestantesEnt = bloquesNecesarios.quot - bloquesAgregado.quot;
    int bloquesRestantesRem = bloquesNecesarios.rem - bloquesAgregado.rem;

	int inicioDatos = (miDisco.cantBloques.bloques_header
				+ miDisco.cantBloques.bloques_bitmap
				+ miDisco.cantBloques.bloques_tablaDeArchivos
				+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;

	void *desplazamiento;
    int tamanioBuffer = 0;
    int bloquesCopiados = 0;

    if(sizeOriginal == sizeFinal){ // Tienen el mismo tamanio
    	log_info(logWrite,"El archivo final tiene el mismo tamanio que el original");
    	void* buffer = malloc(sizeFinal);
    	memcpy(buffer,nuevoContenido, sizeFinal); // Copiamos nuevo contenido al buffer (nada mas porque si no son = nos dariamos cuenta
    	while(bloquesCopiados <= bloquesOriginal.quot){ // Va copiando el contenido original por bloques

    		while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
    			desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
    			memcpy(desplazamiento, buffer + tamanioBuffer, 64);
    			tamanioBuffer += 64;
    			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

    			if (bloquesOriginal.rem  != 0){ // Si queda algun remainder
    				float bloqueAux = bloquesOriginal.rem/10; // Si vos tenes 4.5, .rem te devuelve 5 => 5/10 es 0.5
    				desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
    				memcpy(desplazamiento, buffer + tamanioBuffer, bloqueAux*64); // si bloqueAux es 0.5 (medio bloque) => *64 serian 32
    				tamanioBuffer += bloqueAux*64;								   // bytes, que seria lo que falta escribir
    				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
    			}
    		}
				bloquesCopiados++;
    	}

    	log_info(logWrite, "Cantidad de bytes escritos: %d", sizeAgregado);
    	log_info(logWrite, "Tamanio final archivo: %d", sizeFinal);
    	exito = archivo.file_size == sizeFinal; // Revisar esta linea merquera
    }


    if(sizeFinal > sizeOriginal){ // Se agrego contenido
    	 int progresoBuffer = 0;
    	log_info(logWrite,"El archivo final es de mayor tamanio que el original");
    	void* buffer = malloc(sizeFinal);
    	char* truncarAFinal = string_from_format("truncate -s %d %s", sizeFinal, ruta);
    	system(truncarAFinal);
	    memcpy(buffer,contenidoOriginal,offset);
	    progresoBuffer += offset;
	    memcpy(buffer + progresoBuffer, nuevoContenido, sizeFinal);
	    //progresoBuffer += sizeAgregado;
		//memcpy(buffer + progresoBuffer, contenidoOriginal, sizeFinal);

    	if(1 >= bloquesNecesarios.quot){ // Si hay espacio en bitmap
    		while(bloquesCopiados <= bloquesOriginal.quot){ // Va copiando el contenido original por bloques

    			while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
    				desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
            		memcpy(desplazamiento, buffer + tamanioBuffer, 64);
            		tamanioBuffer += 64;
            		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

            		if (bloquesOriginal.rem  != 0){ // Si queda algun remainder
            			float bloqueAux = bloquesOriginal.rem/10;
            			desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
            			memcpy(desplazamiento, buffer + tamanioBuffer, bloqueAux*64); // si bloqueAux es 0.5 (medio bloque) => *64 serian 32
            			tamanioBuffer += bloqueAux*64;								   // bytes, que seria lo que falta escribir
            			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

            					// Aca como contabilizariamos el bloque?
            				}
        					bloquesCopiados++;
    					}
    				}

               bloquesCopiados = 0;

                while(bloquesCopiados <= bloquesRestantesEnt){ // Va copiando el contenido nuevo en bloques
                	unsigned int bloqueBitmap = primerBloqueBitmapLibre();
   					siguienteBloque = bloqueBitmap;
   					desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
            		memcpy(desplazamiento, buffer + tamanioBuffer, 64);
            		tamanioBuffer += 64;

            		if (bloquesRestantesRem  != 0){ // Si queda algun remainder
            			float bloqueAux = bloquesOriginal.rem/10;
            			desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
            			memcpy(desplazamiento, buffer + tamanioBuffer, bloqueAux*64); // si bloqueAux es 0.5 (medio bloque) => *64 serian 32
            			tamanioBuffer += bloqueAux*64;								   // bytes, que seria lo que falta escribir

            					// Aca como contabilizariamos el bloque?
           			}
   					bloquesCopiados++;
                    actualizarTablaDeAsignaciones();
                    actualizarBitmap();
                }
    	}

    	log_info(logWrite, "Cantidad de bytes escritos: %d", sizeAgregado);
    	log_info(logWrite, "Tamanio final archivo: %d", sizeFinal);
    	exito = archivo.file_size == sizeFinal; // Revisar esta linea merquera
    }

    int progresoBuffer = 0;
    if(sizeFinal < sizeOriginal){ // truncar a 0 y volver a escribirlo
    	log_info(logWrite,"El archivo final es de menor tamanio que el original");
    	char* truncarA0 = string_from_format("truncate -s 0 %s", ruta);
    	system(truncarA0);
    	void* buffer = malloc(sizeFinal);
    	memcpy(buffer,contenidoOriginal,offset);
    	progresoBuffer += offset;
    	memcpy(buffer + progresoBuffer, nuevoContenido, sizeFinal);

        //modificoLosBloquesQueMeQuedaron();
		while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
			int bloquesEscritos;
	        //marcoElUltimoBloqueEnLaTablaAsignaciones();
			for(bloquesEscritos = 0;bloquesEscritos <= redondearDivision(sizeFinal,64); bloquesEscritos++){ // Revisar esto a ver que onda
				desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
				memcpy(desplazamiento, buffer + tamanioBuffer, 64);
				tamanioBuffer += 64;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

				if(bloquesOriginal.rem  != 0){ // Si queda algun remainder
					float bloqueAux = bloquesOriginal.rem/10;
					desplazamiento = &miDisco.discoMapeado[(inicioDatos) + (siguienteBloque * 64)];
					memcpy(desplazamiento, buffer + tamanioBuffer, bloqueAux*64); // si bloqueAux es 0.5 (medio bloque) => *64 serian 32
					tamanioBuffer += bloqueAux*64;								   // bytes, que seria lo que falta escribir
					siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				}
			}
	        //liberoLosOtrosBloques();
			int bloqueActual = siguienteBloque;
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			actualizarBitmap();
			actualizarTablaDeAsignaciones(); // estos 2 iran aca?
			siguienteBloque = miDisco.tablaDeAsignaciones[bloqueActual];
		}
			bloquesCopiados++;
			log_info(logWrite, "Cantidad de bytes escritos: %d", sizeAgregado);
			log_info(logWrite, "Tamanio final archivo: %d", sizeFinal);
			exito = archivo.file_size == sizeFinal; // Revisar esta linea merquera
    }

    return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_unlink(char *ruta){

	t_log* logUnlink;
	remove("osada_unlink.log");
	logUnlink = log_create("osada_unlink.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito = -1;
	log_info(logUnlink, "recibida solicitud de borrado (.unlink) del archivo %s", ruta);
	int i = buscarArchivo(ruta);
	if(i >= -1){
		pthread_mutex_lock(&mutexOsada);
		borrarArchivo(i);
		pthread_mutex_unlock(&mutexOsada);

		exito = 0;
	}

	if(exito == 0){
		log_info(logUnlink, "El archivo %s se ha borrado exitosamente", ruta);
	}
	else{
		log_error(logUnlink, "Error al borrar el archivo %s", ruta);
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_mkdir(char *ruta){
	t_log* logMkdir;
	remove("osada_Mkdir.log");
	logMkdir = log_create("osada_Mkdir.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito = -1;
	log_info(logMkdir, "Solicitud de creación de directorio (.mkdir): %s", ruta);
	int posTablaArchivos = buscarPosicionLibre();
	char *nombreDir = obtenerNombre(ruta);
	int largo = strlen(nombreDir);
	nombreDir[largo] = '\0';
	if(posTablaArchivos >= 0 && largo <= 17){
		pthread_mutex_lock(&mutexOsada);
		crearDirectorio(nombreDir, ruta, posTablaArchivos);
		pthread_mutex_unlock(&mutexOsada);
		exito = 0;

	}

	if(exito == 0){
		log_info(logMkdir, "El directorio  %s se creo exitosamente", ruta);
	}
	else{
		if(posTablaArchivos < 0){
			log_info(logMkdir, "Error al crear el directorio: espacio insuficiente en la tabla de"
					"archivos");
		}
		if(largo > 17){
			log_info(logMkdir, "Error al crear el directorio: nombre de directorio inválido"
					" (supera los 17 caracteres)");
		}
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rmdir(char *ruta){
	t_log* logRmdir;
	remove("osada_rmdir.log");
	logRmdir = log_create("osada_rmdir.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito;
	log_info(logRmdir, "Recibida solicitud de borrado (.rmdir)del directorio %s", ruta);

	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	borrarDirectorio(i);
	pthread_mutex_unlock(&mutexOsada);
	exito = 0; // hay chances de error? validar
	log_info(logRmdir, "El directorio %s se ha borrado exitosamente", ruta);

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rename(char *ruta, char *nuevoNombre){
	t_log* logRename;
	int i;
	remove("osada_rename.log");
	logRename = log_create("osada_rename.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito = 1;
	log_info(logRename, "Recibida solicitud de cambio de nombre (.rename) en la ruta %s", ruta);
	char *nombre = obtenerNombre(nuevoNombre);
	if(strlen(nombre) <= 17){
		i = buscarArchivo(ruta);


		pthread_mutex_lock(&mutexOsada);

		renombrar(nombre, i);

		pthread_mutex_unlock(&mutexOsada);

		exito = 0;
	}

	if(exito == 0){
		log_info(logRename, "El archivo  %s se ha renombrado exitosamente. "
			"Nuevo nombre:%s", ruta, miDisco.tablaDeArchivos[i].fname);
	}
	else{
		log_error(logRename, "error al renombrar el archivo %s: el nombre solicitado es inválido.",
				ruta);
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_truncate(char *ruta, size_t nuevoTamanio){
	int exito = -1;
	int i = buscarArchivo(ruta);
	int tamanioActual = miDisco.tablaDeArchivos[i].file_size;
	int siguienteBloque;
	int aux, aux2;

	if(nuevoTamanio > tamanioActual){ //Agregamos bloques!
		int diferencia = nuevoTamanio - tamanioActual;
		int bloquesNecesarios = calcularBloquesNecesarios(diferencia);
		int hayEspacio = hayEspacioEnDisco(bloquesNecesarios);
		if(hayEspacio < 0){
			goto finalizar;
		}
		siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
		while(siguienteBloque != -1){
			aux = siguienteBloque;
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

		}

		while(bloquesNecesarios > 0){
			miDisco.tablaDeAsignaciones[aux] = primerBloqueBitmapLibre();
			bitarray_set_bit(miDisco.bitmap, miDisco.tablaDeAsignaciones[aux]);
			aux2 = aux;
			aux = miDisco.tablaDeAsignaciones[aux];
			bloquesNecesarios--;
		}
		miDisco.tablaDeAsignaciones[aux2] = -1;
		miDisco.tablaDeArchivos[i].file_size = nuevoTamanio;
		miDisco.tablaDeArchivos[i].lastmod = consultarTiempo();
		actualizarBitmap();
		actualizarTablaDeArchivos();
		actualizarTablaDeAsignaciones();
		exito = 1;

	}
	else if(nuevoTamanio < tamanioActual){ // Desvinculamos bloques!
		int diferencia = tamanioActual - nuevoTamanio;
		int bloquesNecesarios = calcularBloquesNecesarios(diferencia);
		siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
		while(bloquesNecesarios > 0){
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
			bloquesNecesarios--;
		}

		while(siguienteBloque != -1){
			aux = miDisco.tablaDeAsignaciones[siguienteBloque];
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			siguienteBloque = aux;

		}
		miDisco.tablaDeArchivos[i].file_size = nuevoTamanio;
		miDisco.tablaDeArchivos[i].lastmod = consultarTiempo();
		actualizarBitmap();
		actualizarTablaDeArchivos();
		actualizarTablaDeAsignaciones();
		exito = 1;

	}

	finalizar:
	return exito;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void crearDirectorio(char *nombreArchivo, char *ruta, int pos){
	int largoRuta = strlen(ruta);
	int largo = strlen(nombreArchivo);
	char *dirPadre = string_substring_until(ruta, largoRuta - largo);
	int parentDir = buscarArchivo(dirPadre);
	osada_file nuevoDir;
	nuevoDir.file_size = 0;
	nuevoDir.state = DIRECTORY;
	strcpy(nuevoDir.fname, nombreArchivo);
	nuevoDir.parent_directory = parentDir;
	nuevoDir.lastmod = consultarTiempo();
	nuevoDir.first_block = 999999;

	miDisco.tablaDeArchivos[pos] = nuevoDir;
	actualizarTablaDeArchivos();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void borrarDirectorio(int posicion){
	miDisco.tablaDeArchivos[posicion].state = DELETED;
	int i;
	for(i = 0; i <= 2047; i++){
		if(miDisco.tablaDeArchivos[i].parent_directory == posicion &&
				miDisco.tablaDeArchivos[i].state == REGULAR){
			borrarArchivo(i);
		}
		else if(miDisco.tablaDeArchivos[i].parent_directory == posicion &&
				miDisco.tablaDeArchivos[i].state == DIRECTORY){
			borrarDirectorio(i);
		}

	}
	actualizarTablaDeArchivos();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void crearArchivo(char *nombreArchivo, int parentDir, int bloqueInicial, int posTablaArchivos){
	osada_file nuevoArchivo;
	nuevoArchivo.file_size = 0;
	nuevoArchivo.first_block = bloqueInicial;
	strcpy(nuevoArchivo.fname, nombreArchivo);
	nuevoArchivo.lastmod = consultarTiempo();
	nuevoArchivo.parent_directory = parentDir;
	nuevoArchivo.state = REGULAR;

	miDisco.tablaDeArchivos[posTablaArchivos] = nuevoArchivo;
	bitarray_set_bit(miDisco.bitmap, bloqueInicial);
	actualizarTablaDeArchivos();
	actualizarBitmap();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
void borrarArchivo(int posicion){
	int aux;
	miDisco.tablaDeArchivos[posicion].state = DELETED;
		int siguienteBloque = miDisco.tablaDeArchivos[posicion].first_block;
		while(siguienteBloque != -1){
			bitarray_clean_bit(miDisco.bitmap, siguienteBloque);
			aux = miDisco.tablaDeAsignaciones[siguienteBloque];
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			siguienteBloque = aux;
		}
	actualizarTablaDeArchivos();
	actualizarBitmap();
	actualizarTablaDeAsignaciones();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void renombrar(char *nombre, int posicion){
	strcpy(miDisco.tablaDeArchivos[posicion].fname, nombre);
	actualizarTablaDeArchivos();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
int calcularBloquesNecesarios(int tamanio){
	int cantidad;
	div_t bloques = div(tamanio, 64);
	if(bloques.rem == 0){
		cantidad = bloques.quot;
	}
	else{
		cantidad = bloques.quot + 1;
	}
return cantidad;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int hayEspacioEnDisco(int unaCantidad){
	int exito = -1;
	int n = 0;
	int finalBitmap = miDisco.header->data_blocks;
	int i;

	for(i = 0; i <= finalBitmap; i++){
		if(!bitarray_test_bit(miDisco.bitmap, i)){
			n++;
		}
	}
	if(n >= unaCantidad){
		exito = 1;
	}

return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
