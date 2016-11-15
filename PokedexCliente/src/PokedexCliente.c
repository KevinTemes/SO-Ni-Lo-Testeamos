/*
 ============================================================================
 Name        : PokedexCliente.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netdb.h>
#include <unistd.h>
#include "libSockets.h"

typedef struct{
	int tipo_archivo;
	int size;
}t_getattr;

/*  acordarse de chequear si hay que poner -DFUSE_USE_VERSION=27 y -D_FILE_OFFSET_BITS=64
 * como parámetros de compilación */

//-----------------------------------------------------
int pokedexServidor;
int protocolo;
int* pmap_arch;
struct stat archivoStat;
//-----------------------------------------------------

#define DEFAULT_FILE_CONTENT "Hello World!\n"

//-----------------------------------------------------

/* defines para testear sockets */
#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024

//--------------------------------------------------------------------------------
void* recibirDatos(int conexion, int tamanio){
	void* mensaje=(void*)malloc(tamanio);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios=string_new();
		string_append(&adios,"0\0");
		return adios;}
	return mensaje;
}

void solicitarServidor(const char* path, int protocolo){ // Este va para los casos que se envie un path y que el servidor tenga que diferenciar que hay que hacer por un protocolo

	int sizePath = (sizeof (char) * strlen(path));
	int sizeProtocolo = sizeof(int);

	void *leBuffer = malloc(sizePath + (2 * sizeof(int)));

	// Aca tengo que pasar los sizes para poder saber donde termina al path y donde empieza el protocolo
	memcpy(leBuffer, &protocolo, sizeof(int));
	memcpy(leBuffer + sizeof(int), &sizePath, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), path, sizePath);

	send(pokedexServidor,leBuffer, sizePath + (2 * sizeof(int)), MSG_WAITALL);
	free(leBuffer); // No se si esto va
}

void solicitarModificacionServidor(const char* path,const char* objetivo, int protocolo){
	// Por medio del protocolo el servidor sabe que tiene que modificar

	int sizePath = (sizeof (char) * strlen(path));
	int sizeObjetivo = (sizeof (char) * strlen(objetivo));
	int sizeProtocolo = sizeof (int);

	void *leBuffer = malloc (sizePath + sizeObjetivo + sizeProtocolo);

	memcpy(leBuffer, &sizePath, sizeof(int));
	memcpy(leBuffer + sizeof(int), &sizeObjetivo, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), &sizeProtocolo, sizeof(int));

	memcpy(leBuffer + (3 * sizeof(int)),&path, sizePath);
	memcpy(leBuffer + (3 * sizeof(int))+ sizePath, &objetivo, sizeObjetivo);
	memcpy(leBuffer + (3 * sizeof(int))+ sizePath + sizeObjetivo, &protocolo, sizeProtocolo);

	send(pokedexServidor, leBuffer, sizePath + sizeObjetivo + sizeProtocolo, 0);
	free(leBuffer);
}

int recibirTipoFile(){
	int tipoFile = 0;
	int sizeTipoFile = sizeof(int);

	recv(pokedexServidor, &sizeTipoFile, sizeof(int), MSG_WAITALL);
	void*bufferTipoFile = malloc(sizeTipoFile);
	recv(pokedexServidor, bufferTipoFile, sizeTipoFile, 0);
	tipoFile = (int) bufferTipoFile;

	free(bufferTipoFile);
	return tipoFile;
}

char* recibirListado(){
	char* listadoConcatenado = NULL;
	int sizeListadoConcatenado = (sizeof(char)* strlen(listadoConcatenado));

	recv(pokedexServidor, &sizeListadoConcatenado, sizeof(int),MSG_WAITALL );
	void*bufferListado = malloc(sizeListadoConcatenado);
	recv(pokedexServidor,bufferListado,sizeListadoConcatenado,0);
	listadoConcatenado = (char*) bufferListado;

	free(bufferListado);
	return listadoConcatenado;
}

char* recibirContenidoArchivo(){ // recibe contenido de archivo en buffer void
	void* contenido = NULL;
	int sizeContenido = (sizeof(char)* strlen(contenido));

	recv(pokedexServidor, &sizeContenido, sizeof(int), MSG_WAITALL);
	void*bufferContenido = malloc(sizeContenido);
	recv(pokedexServidor,bufferContenido,sizeContenido,0);
	contenido = bufferContenido;

	free(bufferContenido);
	return contenido;
}

int recibirEstadoOperacion(){
	int resultado = 1;
	int sizeResultado = (sizeof(int));

	recv(pokedexServidor, &sizeResultado, sizeof(int), MSG_WAITALL);
	void*bufferResultado = malloc(sizeResultado);
	recv(pokedexServidor,bufferResultado, sizeResultado,0);
	resultado = (int) bufferResultado;

	free(bufferResultado);
	return resultado;
}

//--------------------------------------------------------------------------------

/* Implementacion de GetAttributes para fuse*/
int cliente_getattr(const char *path, struct stat *stbuf) {
	int res= 0;
	protocolo = 0;
	int sizePath = (sizeof (char) * strlen(path));
	t_getattr *respuesta;

	void *leBuffer = malloc(sizePath + (2 * sizeof(int)));
	void *leAnswer = malloc(2 * sizeof(int));

	// Aca tengo que pasar los sizes para poder saber donde termina al path y donde empieza el protocolo
	memcpy(leBuffer, &protocolo, sizeof(int));
	memcpy(leBuffer + sizeof(int), &sizePath, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), path, sizePath);

	send(pokedexServidor,leBuffer, sizePath + (2 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, leAnswer, 2 * sizeof(int), MSG_WAITALL);

	respuesta = (t_getattr *) leAnswer;

//	int tipoFile = recibirTipoFile();
	memset(stbuf,0,sizeof(struct stat));

	if (respuesta->tipo_archivo == 2){ // Es un directorio
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if (respuesta->tipo_archivo == 1){ // Es un archivo regular
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = respuesta->size;
	}
	else{

		res= -ENOENT;
	}
	free(leBuffer);
	free(leAnswer);
	return res;
}

/* Implementacion del comando "ls" para fuse*/
static int cliente_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) {
	int res= 0, i=0;
	protocolo = 1;
	int sizePath = (sizeof (char) * strlen(path));
	char *listadoConcatenado = string_new();
	void *leBuffer = malloc(sizePath + (2 * sizeof(int)));
	int leTamanio;

	// Aca tengo que pasar los sizes para poder saber donde termina al path y donde empieza el protocolo
	memcpy(leBuffer, &protocolo, sizeof(int));
	memcpy(leBuffer + sizeof(int), &sizePath, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), path, sizePath);

	send(pokedexServidor,leBuffer, sizePath + (2 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, &leTamanio, sizeof(int), MSG_WAITALL);

	if(leTamanio == 0){
		res = -ENOENT;
	}

	else{
		void *leAnswer = malloc(leTamanio);
		recv(pokedexServidor, leAnswer, leTamanio, MSG_WAITALL);


		char* listado = (char *) leAnswer;
		listadoConcatenado = string_substring_until(listado, leTamanio - 1);
		char** archivos = string_split(listadoConcatenado,";");

		if (archivos[i] == NULL){
			res = -ENOENT;
			}

		else{
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			while (archivos[i] != NULL){
				filler(buf, archivos[i], NULL, 0);
				i++;
			}
		}
		free(leAnswer);
	}


	free(leBuffer);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static int cliente_open(const char *path, struct fuse_file_info *file){
	int res = 0;
	int protocolo = 9;
	char *ruta = (char *)path;
	int tamanio = strlen(ruta);
	int exito;
	void *buffer = malloc((2 * sizeof(int)) + tamanio);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), ruta, tamanio);

	send(pokedexServidor,buffer, tamanio + (2 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, &exito, sizeof(int), MSG_WAITALL);

	if(exito == 0){
		res = -ENOENT;
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static int cliente_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { // Fijarse bien esta
	protocolo = 2;
	char* ruta = string_new();
	ruta =(char*) path;
	int tamanioRuta = sizeof(char) * strlen(ruta);
	int tamanioRespuesta;

	void *leBuffer = malloc((2 * sizeof(int)) + tamanioRuta);
	memcpy(leBuffer, &protocolo, sizeof(int));
	memcpy(leBuffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), ruta, tamanioRuta);
	send(pokedexServidor, leBuffer, tamanioRuta + (2 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, &tamanioRespuesta, sizeof(int), MSG_WAITALL);
	void *contenido = malloc(tamanioRespuesta);
	recv(pokedexServidor, contenido, tamanioRespuesta, MSG_WAITALL);
	memcpy(buf, ((char *)contenido + offset), size);


	free(leBuffer);
	free(contenido);
	return tamanioRespuesta;
}

/* Crea un archivo vacio*/
static int cliente_create(const char* path, mode_t modo, struct fuse_file_info * fi){ //Por ahora asumimos que el nombre del archivo esta en el path
	int res = 1; // 0 para exito y 1 para error
	protocolo = 3;
	solicitarServidor(path,protocolo);
	res = recibirEstadoOperacion();
	if (res==0){
		printf("Archivo creado exitosamente\n");
	}
	else{
		printf("No se pudo crear el archivo\n");
	}
	return res;
}

/* Implementacion de la operacion write sobre un archivo*/
static int cliente_write(const char* path,const char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
	int res = 1;
	protocolo = 4;
	solicitarModificacionServidor(path,buf,protocolo);
	res = recibirEstadoOperacion();
	if (res==0){
		printf("Archivo escrito exitosamente\n");
	}
	else{
		printf("No se pudo escribir el archivo\n");
	}
	return res;
}

static int cliente_unlink(const char* path){
	int res = 1;
	protocolo = 5;
	solicitarServidor(path,protocolo);
	res = recibirEstadoOperacion();
	if (res==0){
		printf("Archivo borrado exitosamente\n");
	}
	else{
		printf("No se pudo borrar el archivo\n");
	}
	return res;
}

static int cliente_rename(const char* path, const char* nuevoNombre){
	int res=1;
	protocolo = 8;
	solicitarModificacionServidor(path,nuevoNombre,protocolo);
	res = recibirEstadoOperacion();
	if (res==0){
		printf("El archivo fue renombrado exitosamente\n");
	}
	else{
		printf("No se pudo renombrar el archivo\n");
	}
	return res;
}


// char *ipServidor = getenv("IP_SERVIDOR");

static int cliente_mkdir(const char* path, mode_t mode){
	int res;
	protocolo = 6;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + tamanioRuta);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), ruta, tamanioRuta);
	send(pokedexServidor, buffer, (2 * sizeof(int)) + tamanioRuta, MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);

	if (res==0){
		printf("El archivo fue creado exitosamente\n");
	}
	else{
		printf("No se pudo crear el archivo\n");
	}
	return res;

}

static int cliente_rmdir(const char* path){
	int res = 1;
	protocolo = 7;
	solicitarServidor(path,protocolo);
		res = recibirEstadoOperacion();
		if (res==0){
			printf("Directorio borrado exitosamente\n");
		}
		else{
			printf("No se pudo borrar el directorio\n");
		}
		return res;
}

int cliente_truncate(const char * path, off_t offset) {
	int res = 0;
	// "falsa" implementación de truncate para que .write no rompa los quinotos
	return res;
}

//--------------------------------------------------------------------------------


static struct fuse_operations cliente_oper = {
		.getattr = cliente_getattr,
		.readdir = cliente_readdir,
		.open = cliente_open,
		.read = cliente_read,
		.create = cliente_create,
		.write = cliente_write,
		.unlink = cliente_unlink,
		.mkdir = cliente_mkdir,
		.rmdir = cliente_rmdir,
		.rename = cliente_rename,
		.truncate = cliente_truncate,
};


int main(int argc, char *argv[]) {

	pokedexServidor = conectarCliente(IP, PUERTO);

	return fuse_main(argc, argv, &cliente_oper, NULL );


return 0;


}
