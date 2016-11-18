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
//////////////////////////////////////////////////////////////////////////////
static int cliente_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

	log_info(logPC, "Peticion de lectura de archivo %s", path);
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
	if(tamanioRespuesta == 0){
		goto terminar;
	}
	void *contenido = malloc(tamanioRespuesta);
	recv(pokedexServidor, contenido, tamanioRespuesta, MSG_WAITALL);
	memcpy(buf, ((char *)contenido + offset), size);



	free(contenido);
	terminar:
	free(leBuffer);
	log_info(logPC,"Cantidad de bytes leidos: %d", tamanioRespuesta);
	return tamanioRespuesta;
}
//////////////////////////////////////////////////////////////////////////////
/* Crea un archivo vacio*/
static int cliente_create(const char* path, mode_t modo, struct fuse_file_info * fi){ //Por ahora asumimos que el nombre del archivo esta en el path

	int res; // 0 para exito y 1 para error
	log_info(logPC, "Peticion de creacion de archivo %s", path);
	protocolo = 3;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + tamanioRuta);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), ruta, tamanioRuta);
	send(pokedexServidor, buffer, (2 * sizeof(int)) + tamanioRuta, MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);

	if (res==0){
		log_info(logPC, "El archivo %s fue creado exitosamente", path);
	}
	else{
		log_error(logPC, "No se pudo crear el archivo %s", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
/* Implementacion de la operacion write sobre un archivo*/
static int cliente_write(const char* path,const char *buf, size_t size, off_t offset, struct fuse_file_info* fi){

	int res = 1;
	log_info(logPC, "Peticion de escritura sobre archivo %s", path);
	protocolo = 4;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + sizeof(size_t)+ tamanioRuta + size + sizeof(off_t));
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), &size, sizeof(size_t));
	memcpy(buffer + (2 * sizeof(int)) + sizeof(size_t), ruta, tamanioRuta);
	memcpy(buffer + (2 * sizeof(int)) + sizeof(size_t) + tamanioRuta, buf, size);
	memcpy(buffer + (2 * sizeof(int)) + sizeof(size_t) + tamanioRuta + size, &offset, sizeof(off_t));

	send(pokedexServidor, buffer, (4 * sizeof(int)) + tamanioRuta + size, MSG_WAITALL);

	recv(pokedexServidor,&res,sizeof(int),MSG_WAITALL);

	if (res==0){
		log_info(logPC, "Archivo %s escrito exitosamente", path);
	}
	else{
		log_error(logPC, "No se pudo escribir el archivo %s", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
static int cliente_unlink(const char* path){

	int res;
	log_info(logPC, "Peticion de operacion borrado de archivo %s", path);
	protocolo = 5;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + tamanioRuta);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), ruta, tamanioRuta);

	send(pokedexServidor, buffer, tamanioRuta + (2 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);

	if (res==0){
		log_info(logPC, "Archivo %s borrado exitosamente", path);
	}
	else{
		log_error(logPC, "No se pudo borrar el archivo %s", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
static int cliente_rename(const char* path, const char* nuevoNombre){

	int res;
	log_info(logPC, "Peticion de operacion (.rename) de archivo %s", path);
	protocolo = 8;
	char *ruta = (char *)path;
	char *nombre = (char *)nuevoNombre;
	int tamanioRuta = strlen(ruta);
	int tamanioNombre = strlen(nombre);
	int package_size = (3 * sizeof(int)) + tamanioRuta + tamanioNombre;
	void *buffer = malloc(package_size);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), &tamanioNombre, sizeof(int));
	memcpy(buffer + (3 * sizeof(int)), ruta, tamanioRuta);
	memcpy(buffer + (3 * sizeof(int)) + tamanioRuta, nombre, tamanioNombre);

	send(pokedexServidor, buffer, package_size, MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);

	if (res==0){
		log_info(logPC, "El archivo %s fue renombrado exitosamente", path);
	}
	else{
		log_info(logPC, "No se pudo renombrar el archivo %s", path);
	}
	return res;
}

// char *ipServidor = getenv("IP_SERVIDOR");
//////////////////////////////////////////////////////////////////////////////
static int cliente_mkdir(const char* path, mode_t mode){

	int res;
	log_info(logPC, "Peticion de creacion de directorio %s", path);
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
		log_info(logPC, "El directorio %s fue creado exitosamente", path);
	}
	else{
		log_error(logPC, "No se pudo crear el directorio %s", path);
	}
	free(buffer);
	return res;

}
//////////////////////////////////////////////////////////////////////////////
static int cliente_rmdir(const char* path){

	int res;
	log_info(logPC, "Peticion de borrado de directorio %s", path);
	protocolo = 7;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + tamanioRuta);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), ruta, tamanioRuta);
	send(pokedexServidor, buffer, (2 * sizeof(int)) + tamanioRuta, MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);

	if (res==0){
		printf("Directorio borrado exitosamente\n");
	}
	else{
		printf("No se pudo borrar el directorio\n");
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
int cliente_truncate(const char * path, off_t offset) {

	int res;
	log_info(logPC, "Peticion de truncado de archivo %s", path);
	int protocolo = 10;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	void *buffer = malloc((2 * sizeof(int)) + sizeof(off_t) + tamanioRuta);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 *sizeof(int)), ruta, tamanioRuta);
	memcpy(buffer + (2 *sizeof(int)) + tamanioRuta, &offset, sizeof(off_t));
	send(pokedexServidor, buffer, (2 * sizeof(int)) + sizeof(off_t) + tamanioRuta, MSG_WAITALL);

	recv(pokedexServidor, &res, sizeof(int), MSG_WAITALL);


	if(res > 0){

	}
	else{

	}

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

	remove("Cliente.log");
	logPC = log_create("Cliente.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));

	pokedexServidor = conectarCliente(IP, PUERTO);

	return fuse_main(argc, argv, &cliente_oper, NULL );


return 0;


}
