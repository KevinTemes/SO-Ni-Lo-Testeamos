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
	printf("AVISO: cierre inesperando del sistema. Notificando a los clientes y finalizando...\n");
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
		while(nombreArchivo[i] != '\0' && i <= 17){
			i++;
		}
		char *nombre = malloc(i);
		memcpy(nombre, nombreArchivo, i + 1);

		return nombre;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

char* concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);//+1 for the zero-terminator

    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);//+1 to copy the null-terminator
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

int buscarArchivo(char *unaRuta){
	int parentDir = 65535;
	int existeArchivo = 0;
	if(strlen(unaRuta) == 1){
		goto terminar;
	}

	else{
		char **separadas = string_split(unaRuta, "/");
		int a = 0;
		existeArchivo = recorrerDirectorio(separadas[0], parentDir);

		while(separadas[a] != NULL){
			parentDir = recorrerDirectorio(separadas[a], parentDir);
			if(parentDir == -1){
				goto terminar;
			}
			a++;
		}
	}
terminar:
return parentDir;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int recorrerDirectorio(char *nombre, int parentDir){
	int posicion, i;
	posicion = -1;
	char *archivo = nombre;
//	char *fname = string_new();
	if(strcmp(archivo, "")== 0){
			goto finalizar;
		}
	int nameLength = strlen(archivo);

	for(i = 0; i <= 2047; i++){
		if(miDisco.tablaDeArchivos[i].state != DELETED){

			char *fname = getFileName(miDisco.tablaDeArchivos[i].fname);

			if((strncmp(archivo, fname, nameLength) == 0) &&
				((int)miDisco.tablaDeArchivos[i].parent_directory == parentDir)){
					posicion = i;

			}

		}
	}

	finalizar:
//	free(fname);
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
char *convertirRuta(void *buffer, int tamanioRuta){
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
	int codOp;
	int tamanioRuta, tamanioNuevoContenido, tamanioNombre;
	char *ruta = string_new();
	void *contenido;
	char *rutaRecibida;
	void *buffer, *bufferDir;
	void *respuesta;
	t_infoDirectorio contDir;

	printf("%sPokeCliente #%d conectado! esperando solicitudes... \n", KAMARILLO,
				clientesActivos[unCliente].cliente);

	while(status !=0){

		if (status != 0) {
			recv(clientesActivos[unCliente].socket, &codOp, sizeof(int), MSG_WAITALL);

			switch(codOp){

			case 0: // .getAttr

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
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
				ruta = convertirRuta(buffer, tamanioRuta);
				osada_readdirV3(ruta, clientesActivos[unCliente].socket);
				//bufferDir = malloc(contDir.buffer_size);
				//memcpy(bufferDir, &contDir.buffer_size, sizeof(int));
				//memcpy(bufferDir + sizeof(int), contDir.buffer, contDir.buffer_size);
				//send(clientesActivos[unCliente].socket, bufferDir, sizeof(int) + contDir.buffer_size, 0);
				//*ruta = '\0';
				//free(buffer);


			break;

			case 2: // .read

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				void *buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
				void *contenidoArchivo = osada_read(ruta);
				int i = buscarArchivo(ruta);
				int tamanioArchivo = (int)miDisco.tablaDeArchivos[i].file_size;
				buffer = malloc(tamanioArchivo + sizeof(int));
				memcpy(buffer, &tamanioArchivo, sizeof(int));
				memcpy(buffer + sizeof(int), contenidoArchivo, tamanioArchivo);
				send(clientesActivos[unCliente].socket, buffer, sizeof(int) + tamanioArchivo, 0);
				free(buffer);

			break;

			case 3: // .create (por ahora supongo que el nombre ya viene en la ruta)

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
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
				ruta = convertirRuta(buffer, tamanioRuta);
				recv(clientesActivos[unCliente].socket, bufferContenido,
						tamanioNuevoContenido, MSG_WAITALL);
				//int exito = osada_write(ruta, bufferContenido);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				//free(buffer);
				free(bufferContenido);

			break;

			case 5: // .unlink

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
				// int exito = osada_unlink(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 6: // .mkdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
				// int exito = osada_mkdir(ruta);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);

			break;

			case 7: // .rmdir

				recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				buffer = malloc(tamanioRuta);
				recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirRuta(buffer, tamanioRuta);
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
				ruta = convertirRuta(buffer, tamanioRuta);
				char *nombre = (char *) bufferNombre;
				// int exito = osada_rename(ruta, nombre);
				//send(clientesActivos[unCliente].socket, exito, sizeof(int), 0);
				free(buffer);
				free(bufferNombre);

			break;

			}

		}

	}
	free(contenido);
	free(buffer);
	free(bufferDir);

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
	int parentBlock = buscarArchivo(unaRuta);
	int i;

	if(parentBlock == 999999){
		printf("no existe el directorio o subdirectorio especificado\n");
		exit(0);
	}
	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2048; i++){

			if(miDisco.tablaDeArchivos[i].parent_directory == parentBlock){
				char *nombreArchivo = getFileName(miDisco.tablaDeArchivos[i].fname);
				if(strlen(nombreArchivo) > 0){
				string_append(&contenidoDir, nombreArchivo);

				// Concateno los nombres en un string, separados por la variable separador.
				// Después la idea es separarlos y manejarlos como corresponda afuera de esta función

				string_append(&contenidoDir, separador);

				}

			}

		}


return contenidoDir;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////

t_infoDirectorio osada_readdirV2(char *unaRuta){

	t_infoDirectorio contenido;
	void *tmp_ptr;
	int largoNombre;
//	char *nombreArchivo = string_new();
	char *separador = ";";
	int parentBlock = buscarArchivo(unaRuta);
	int i;
	int desplazamiento = 0;
	int first_iteration = -1;

	if(parentBlock == 999999){
		printf("no existe el directorio o subdirectorio especificado\n");
		exit(0);
	}
	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2048; i++){

			if(miDisco.tablaDeArchivos[i].parent_directory == parentBlock){
				char *nombreArchivo = getFileName(miDisco.tablaDeArchivos[i].fname);
				largoNombre = strlen(nombreArchivo);
				if(largoNombre > 0){

					if(first_iteration < 0){
						contenido.buffer = malloc(largoNombre + 1);
						memcpy(contenido.buffer, nombreArchivo, largoNombre);
						memcpy(contenido.buffer + largoNombre, separador, sizeof(char));
						desplazamiento = desplazamiento + largoNombre + 1;
						first_iteration = 1;
					}
					else{
						tmp_ptr = realloc(contenido.buffer, desplazamiento + largoNombre + 1);
						contenido.buffer = tmp_ptr;
						memcpy(contenido.buffer + desplazamiento, nombreArchivo, largoNombre);
						memcpy(contenido.buffer + desplazamiento + largoNombre, separador, sizeof(char));
						desplazamiento = desplazamiento + largoNombre + 1;
					}



				}

			}

		}
	char nullChar[1];
	nullChar[0] = '\0';
	memcpy(contenido.buffer + desplazamiento, &nullChar[0], sizeof(char));
	contenido.buffer_size = desplazamiento + 1;
	free(tmp_ptr);
	tmp_ptr = NULL;

return contenido;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////

void osada_readdirV3(char *unaRuta, int unSocket){

	t_list *listaHorrible = list_create();


	int parent_directory = buscarArchivo(unaRuta);

	int i;

	for(i = 0; i <= 2048; i++){
		if(miDisco.tablaDeArchivos[i].parent_directory == parent_directory){
			list_add(listaHorrible, (void *)i);
		}
	}

	int j = list_size(listaHorrible);

	int k;
	if(!list_is_empty(listaHorrible)){
		for(i = 0; i <= j; i ++){
			k = (int)list_remove(listaHorrible, 0);
			char *nombreArchivo = getFileName(miDisco.tablaDeArchivos[k].fname);
			int largoNombre = strlen(nombreArchivo);
			nombreArchivo[largoNombre] = '\0';
			void *buffer = malloc(sizeof(int) + largoNombre);
			memcpy(buffer, &largoNombre, sizeof(int));
			memcpy(buffer + sizeof(int), nombreArchivo, largoNombre);
			send(unSocket, buffer, sizeof(int) + largoNombre, MSG_WAITALL);
			free(buffer);
		};
	}

	char *finalEnvio = "lopinju";
	int largoFinal = strlen(finalEnvio);
	void *buffer = malloc(sizeof(int) + largoFinal);
	memcpy(buffer, &largoFinal, sizeof(int));
	memcpy(buffer + sizeof(int), finalEnvio, largoFinal);
	send(unSocket, buffer, sizeof(int) + largoFinal, MSG_WAITALL);
	free(buffer);

	list_destroy_and_destroy_elements(listaHorrible, free);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void *osada_read(char *ruta){

	t_log* logRead;
	remove("osada_read.log");
	logs = log_create("osada_read.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));

	int i = buscarArchivo(ruta);
	log_info(logRead, "Comienza la operacion del archivo %s", ruta);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	void *buffer = malloc(miDisco.tablaDeArchivos[i].file_size);
	div_t bloquesOcupados = div(miDisco.tablaDeArchivos[i].file_size, 64);
	int tamanioActualBuffer = 0;
	int inicioDatos = (miDisco.cantBloques.bloques_header
				+ miDisco.cantBloques.bloques_bitmap
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

	log_info(logRead, "Cantidad de bytes leidos: %d", tamanioActualBuffer);
	char *epifania = buffer;
	printf("%s\n", epifania);

	return(buffer);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_create(char *ruta){

	t_log* logCreate;
	remove("osada_create.log");
	logs = log_create("osada_create.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	log_info(logCreate, "Comienza el create del archivo %s", ruta);
	int exito = 0;

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
	if(exito){
	log_info(logCreate, "El archivo se creo exitosamente");
	}
	else{
		log_error(logCreate, "No se pudo crear el archivo");
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_write(char *ruta, void *nuevoContenido, size_t sizeAgregado, off_t offset){

	t_log* logWrite;
	remove("osada_write.log");
	logs = log_create("osada_write.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
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
    			desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
    			memcpy(desplazamiento, buffer + tamanioBuffer, 64);
    			tamanioBuffer += 64;
    			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

    			if (bloquesOriginal.rem  != 0){ // Si queda algun remainder
    				float bloqueAux = bloquesOriginal.rem/10; // Si vos tenes 4.5, .rem te devuelve 5 => 5/10 es 0.5
    				desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
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
	    memcpy(buffer,contenidoOriginal,offset);
	    progresoBuffer += offset;
	    memcpy(buffer + progresoBuffer, nuevoContenido, sizeFinal);
	    //progresoBuffer += sizeAgregado;
		//memcpy(buffer + progresoBuffer, contenidoOriginal, sizeFinal);

    	if(bloquesBitmapLibres() >= bloquesNecesarios.quot){ // Si hay espacio en bitmap
    		while(bloquesCopiados <= bloquesOriginal.quot){ // Va copiando el contenido original por bloques

    			while(miDisco.tablaDeAsignaciones[siguienteBloque] != -1){
    				desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
            		memcpy(desplazamiento, buffer + tamanioBuffer, 64);
            		tamanioBuffer += 64;
            		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

            		if (bloquesOriginal.rem  != 0){ // Si queda algun remainder
            			float bloqueAux = bloquesOriginal.rem/10;
            			desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
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
   					desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
            		memcpy(desplazamiento, buffer + tamanioBuffer, 64);
            		tamanioBuffer += 64;

            		if (bloquesRestantesRem  != 0){ // Si queda algun remainder
            			float bloqueAux = bloquesOriginal.rem/10;
            			desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
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
			for(bloquesEscritos = 0;bloquesEscritos <= redondearDivision(sizeFinal/64); bloquesEscritos++){
				desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
				memcpy(desplazamiento, buffer + tamanioBuffer, 64);
				tamanioBuffer += 64;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

				if(bloquesOriginal.rem  != 0){ // Si queda algun remainder
					float bloqueAux = bloquesOriginal.rem/10;
					desplazamiento = &miDisco.discoMapeado[(inicioDatos / 4) + ((siguienteBloque * 64) / 4)];
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
	logs = log_create("osada_unlink.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito = 0;
	log_info(logUnlink, "Comienza la operacion unlink sobre el archivo %s", ruta);

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
	exito = 1; // hay chances de error? validar
	log_info(logUnlink, "El archivo se ha borrado exitosamente");
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_mkdir(char *ruta, char *nombreDir){
	t_log* logMkdir;
	remove("osada_Mkdir.log");
	logs = log_create("osada_Mkdir.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito = 0;
	log_info(logMkdir, "Comienza la creacion del directorio %s", ruta);
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
				exito = 1;
			}
	if(exito){
		log_info(logMkdir, "El directorio se creo exitosamente");
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rmdir(char *ruta){
	t_log* logRmdir;
	remove("osada_rmdir.log");
	logs = log_create("osada_rmdir.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito;
	log_info(logRmdir, "Comienza la operacion rmdir sobre el directorio %s", ruta);

	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	miDisco.tablaDeArchivos[i].state = '\0';
	actualizarTablaDeArchivos();
	pthread_mutex_unlock(&mutexOsada);
	exito = 1; // hay chances de error? validar
	log_info(logRmdir, "El directorio se ha borrado exitosamente");

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rename(char *ruta, char *nuevoNombre){
	t_log* logRename;
	remove("osada_rename.log");
	logs = log_create("osada_rename.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));
	int exito;
	log_info(logRename, "Comienza la operacion rename del archivo %s", ruta);

	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&mutexOsada);
	iterarNombre(nuevoNombre,miDisco.tablaDeArchivos[i].fname);
	//strcpy(miDisco.tablaDeArchivos[i].fname, nuevoNombre); Si el de arriba funciona borrar esta linea
	actualizarTablaDeArchivos();
	pthread_mutex_unlock(&mutexOsada);

	exito = 1;
	log_info(logRename, "El archivo se ha renombrado exitosamente");
	log_info(logRename, "Nombre actual: %s", miDisco.tablaDeArchivos[i].fname );
	return exito;
}
