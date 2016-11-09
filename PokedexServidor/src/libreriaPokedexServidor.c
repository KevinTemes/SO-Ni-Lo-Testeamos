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
		if (*dst != basura){
			dst++;
		}
	}
	*dst = '\0';
}
////////////////////////////////////////////////////////////////////////////////////////////////////////


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

////////////////////////////////////////////////////////////////////////////////////////////////////////

int buscarArchivo(char *unaRuta){
	char **separadas = string_split(unaRuta, "/");
	int a = 0;
	int parentDir = 65535;
	while(separadas[a] != NULL){
		parentDir = recorrerDirectorio(separadas[a], parentDir);
		a++;
	}
return parentDir;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int recorrerDirectorio(char *nombre, int parentDir){
	int posicion, i;
	char *archivo = nombre;
	int nameLength = strlen(archivo);
	char *fname = string_new();
	for(i = 0; i <= 2048; i++){
		fname = getFileName(miDisco.tablaDeArchivos[i].fname);
		if((strncmp(archivo, fname, nameLength) == 0) &&
				((int)miDisco.tablaDeArchivos[i].parent_directory == parentDir)){
			posicion = i;
		}
	}


	return posicion;
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
				int tipoArchivo = osada_getAttr(ruta);
				send(clientesActivos[unCliente].socket, tipoArchivo, sizeof(int), 0);
				free(buffer);

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

			case 3: // .create (por ahora supungo que el nombre ya viene en la ruta)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_crearArchivo(ruta);
				// send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 4: // .write


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

			case 5: // .unlink

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_unlink(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 6: // .mkdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_mkdir(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 7: // .rmdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = (char *) buffer;
				// int exito = osada_rmdir(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 8: // .rename


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
void actualizarBitmap(){
	int inicioBitmap = (miDisco.cantBloques.bloques_header * 64) / 4;
	int tamanioBitmap = miDisco.cantBloques.bloques_bitmap * 64;
	memcpy(miDisco.discoMapeado + inicioBitmap, miDisco.bitmap, tamanioBitmap);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarTablaDeArchivos(){
	int inicioTabla = ((miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap) * 64)
			/ 4;
	memcpy(miDisco.discoMapeado + inicioTabla, &miDisco.tablaDeArchivos, sizeof(osada_file) * 2048);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarTablaDeAsignaciones(){
	int inicioTabla = ((miDisco.cantBloques.bloques_header +miDisco.cantBloques.bloques_bitmap +
			miDisco.cantBloques.bloques_tablaDeArchivos) * 64) / 4;
	int tamanioTabla = miDisco.cantBloques.bloques_tablaDeAsignaciones * 64;
	memcpy(miDisco.discoMapeado + inicioTabla, miDisco.tablaDeAsignaciones, tamanioTabla);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int redondearDivision(unsigned int dividendo, unsigned int divisor){
	return (dividendo + (divisor / 2) / divisor);
}

unsigned int calcularBloquesNecesarios(osada_file archivo){
	unsigned int resultado = redondearDivision(archivo.file_size,64);
	return resultado;
}

unsigned int bloquesBitmapLibres(osada_file archivo){

	int nroBloque, bloquesLibres = 0;
	for( nroBloque = 0 ; nroBloque <= bitarray_get_max_bit(miDisco.bitmap); nroBloque++){

		if(bitarray_test_bit(miDisco.bitmap, nroBloque) == 0){
			bloquesLibres++;
		}
	}
	return (bloquesLibres >= calcularBloquesNecesarios(archivo)) ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int primerBloqueBitmapLibre(){
	unsigned int nroBloque = 0;
	while(bitarray_test_bit(miDisco.bitmap,nroBloque) == 1){
		nroBloque++;
	}
	return nroBloque;
}

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
	for (i=0; i <=2048; i++){
		if(miDisco.tablaDeArchivos[i].state == '\0'){
			miDisco.tablaDeArchivos[i] = archivo;
			posAsignada = i;
			break;
		}
	}
	return posAsignada;
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
	div_t bloquesOcupados = div(miDisco.tablaDeArchivos[i].file_size, 64);
	int tamanioActualBuffer = 0;
	int inicioDatos = (miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap
				+ miDisco.cantBloques.bloques_tablaDeArchivos
				+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;
	void *desplazamiento;

	if(bloquesOcupados.rem == 0){
		while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
			desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
			memcpy(buffer + tamanioActualBuffer, desplazamiento, 64);
			tamanioActualBuffer += 64;
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

		}
	}
	else{
		int bloquesCopiados = 0;
		while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
			if(bloquesCopiados <= bloquesOcupados.quot){
				desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) +
													   ((siguienteBloque * 64) / 4)];
				memcpy(buffer + tamanioActualBuffer,desplazamiento, 64);
				tamanioActualBuffer += 64;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				bloquesCopiados++;
			}
			else{
				desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) +
																	   ((siguienteBloque * 64) / 4)];
				memcpy(buffer + tamanioActualBuffer, desplazamiento, bloquesOcupados.rem);
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				bloquesCopiados++;
			}
		}
	}

	char *epifania = buffer;
	printf("%s\n", epifania);

	return(buffer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_create(char *ruta){
	int exito = -1;
	unsigned int BloqueLibre = primerBloqueBitmapLibre();
	unsigned int BloqueAsignacionesLibre = primerBloqueTablaAsignacionesLibre(); // Implementar

	osada_file archivoNuevo;
	llenarEstructuraNuevo(archivoNuevo, ruta, BloqueAsignacionesLibre);
	int posTablaArchivos = asignarPosicionTablaArchivos(archivoNuevo); // Revisar orden


	if (bloquesBitmapLibres(archivoNuevo)){
		char* touch = string_from_format("touch %s", ruta);
		int exito = system(touch); // Revisar, touch no te genera osada_file

		bitarray_set_bit(miDisco.bitmap, BloqueLibre);
		actualizarBitmap();

		if(posTablaArchivos>=0){
			iterarNombre(archivoNuevo.fname,miDisco.tablaDeArchivos[posTablaArchivos].fname);
			miDisco.tablaDeArchivos[posTablaArchivos].state = '\1';
			miDisco.tablaDeArchivos[posTablaArchivos].file_size = archivoNuevo.file_size;
			miDisco.tablaDeArchivos[posTablaArchivos].first_block = archivoNuevo.first_block; // Deberia ser con tabla de asginaciones
			miDisco.tablaDeArchivos[posTablaArchivos].lastmod = archivoNuevo.lastmod;
			miDisco.tablaDeArchivos[posTablaArchivos].parent_directory = archivoNuevo.parent_directory;
			actualizarTablaDeArchivos();
			actualizarTablaDeAsignaciones();
		}
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_write(char *ruta, void *nuevoContenido, off_t offset){
	int exito;
	exito = 1;


	int posicion = buscarArchivo(ruta);

	miDisco.tablaDeArchivos[posicion].lastmod = consultarTiempo();
	int sizeOriginal = miDisco.tablaDeArchivos[posicion].file_size;

	void* contenidoOriginal = osada_read(ruta);

	if (offset > strlen(contenidoOriginal)){ // Nada mas se agrega algo al archivo

		int sizeFinal = sizeOriginal + (sizeof(char) * strlen(nuevoContenido)); // sera asi?
		int quedaEspacioEnBitmap = bloquesBitmapLibres > calcularBloquesNecesarios(miDisco.tablaDeArchivos[posicion]);
		int quedaEspacioEnAsignaciones =
		exito = !(quedaEspacioEnBitmap && quedaEspacioEnAsignaciones);

		char* contenidoSobreescrito = string_new();
		string_append(contenidoSobreescrito, contenidoOriginal);
		string_append(contenidoSobreescrito, nuevoContenido);

		memcpy(contenidoOriginal, contenidoSobreescrito, sizeFinal);
		miDisco.tablaDeArchivos[posicion].lastmod = consultarTiempo();
		miDisco.tablaDeArchivos[posicion].file_size = sizeFinal;
		actualizarBitmap();
	//	actualizarAsignaciones();
	}


	if(bloquesBitmapLibres(miDisco.tablaDeArchivos[posicion])){ // Puede entrar todo en el bitmap

		int siguienteBloque = miDisco.tablaDeArchivos[posicion].first_block;
			void *buffer = malloc(miDisco.tablaDeArchivos[posicion].file_size);
			int tamanioActualBuffer = 0;

			while(miDisco.tablaDeAsignaciones[siguienteBloque] != 65535){
				copiarBloque(buffer, siguienteBloque, tamanioActualBuffer);
				tamanioActualBuffer += 64;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

			}

	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_unlink(char *ruta){
	int exito;
	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	miDisco.tablaDeArchivos[i].state = '\0';
	actualizarTablaDeArchivos();
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	while(siguienteBloque != -1){
		bitarray_clean_bit(miDisco.bitmap, siguienteBloque);
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
	}
	actualizarBitmap();
	pthread_mutex_unlock(&mutexOsada);
	exito = 0; // hay chances de error? validar

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_mkdir(char *ruta, char *nombreDir){
	int exito = 1;
	unsigned int bloqueAsignacionesLibre = primerBloqueTablaAsignacionesLibre();
	osada_file directorioNuevo;
	int posTablaArchivos = asignarPosicionTablaArchivos(directorioNuevo);

	llenarEstructuraNuevo(directorioNuevo, ruta, bloqueAsignacionesLibre);

	if(posTablaArchivos>=0){
				iterarNombre(directorioNuevo.fname,miDisco.tablaDeArchivos[posTablaArchivos].fname);
				miDisco.tablaDeArchivos[posTablaArchivos].state = '\2';
				miDisco.tablaDeArchivos[posTablaArchivos].file_size = directorioNuevo.file_size;
				miDisco.tablaDeArchivos[posTablaArchivos].first_block = directorioNuevo.first_block;
				miDisco.tablaDeArchivos[posTablaArchivos].lastmod = directorioNuevo.lastmod;
				miDisco.tablaDeArchivos[posTablaArchivos].parent_directory = directorioNuevo.parent_directory;
				actualizarTablaDeArchivos();
				actualizarTablaDeAsignaciones();
				exito = 0;
			}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rmdir(char *ruta){
	int exito;
	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	miDisco.tablaDeArchivos[i].state = '\0';
	actualizarTablaDeArchivos();
	pthread_mutex_unlock(&mutexOsada);
	exito = 0; // hay chances de error? validar

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rename(char *ruta, char *nuevoNombre){
	int exito;
	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	iterarNombre(nuevoNombre,miDisco.tablaDeArchivos[i].fname);
	//strcpy(miDisco.tablaDeArchivos[i].fname, nuevoNombre); Si el de arriba funciona borrar esta linea
	actualizarTablaDeArchivos();
	pthread_mutex_unlock(&mutexOsada);
	exito = 0;

	return exito;
}
