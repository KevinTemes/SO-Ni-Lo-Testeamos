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
//-----------------------------------------------------

t_log *log_Cliente;

//-----------------------------------------------------

/* defines para testear sockets */
#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024

//--------------------------------------------------------------------------------

/* Implementacion de GetAttributes para fuse*/
int cliente_getattr(const char *path, struct stat *stbuf) {

	log_info(log_Cliente, "Operacion .getattr sobre la ruta %s", path);
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
		log_info(log_Cliente, "Operacion .getattr completa. La ruta %s corresponde a un directorio", path);

	}
	else if (respuesta->tipo_archivo == 1){ // Es un archivo regular
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = respuesta->size;
		log_info(log_Cliente, "Operacion .getattr completa. La ruta %s corresponde a un archivo regular.", path);
	}
	else{

	//	log_info(log_Cliente, "Operacion .getattr completa. La ruta %s no existe.", path);
		res= -ENOENT;
	}
	free(leBuffer);
	free(leAnswer);
	return res;
}

/* Implementacion del comando "ls" para fuse*/
static int cliente_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) {

	log_info(log_Cliente, "Operacion .readdir sobre la ruta %s", path);
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
		log_info(log_Cliente, "Operacion .readdir completa. El directorio %s está vacio.", path);
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
				string_append(&archivos[i], "\0");
				filler(buf, archivos[i], NULL, 0);
				i++;
			}

			log_info(log_Cliente, "Operacion .readdir exitosa. Contenidos del directorio %s obtenidos.", path);
		}
		free(leAnswer);
	}


	free(leBuffer);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
static int cliente_open(const char *path, struct fuse_file_info *file){

	log_info(log_Cliente, "Operacion .open del archivo %s", path);
	int res = 1;
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

	if(exito != 0){
		res = -ENOENT;
		log_error(log_Cliente, "Error al abrir el archivo %s. El archivo no existe o es invalido.", path);
	}
	else{
		log_info(log_Cliente, "Archivo %s abierto correctamente.", path);
		res = exito;
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////
static int cliente_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {


	log_info(log_Cliente, "Operacion de lectura (.read) del archivo %s", path);
	protocolo = 2;
	char* ruta = string_new();
	ruta =(char*) path;
	int tamanioRuta = sizeof(char) * strlen(ruta);
	int tamanioLectura = (int) size;
	int off = (int) offset;
	int tamanioRespuesta;

	void *leBuffer = malloc((4 * sizeof(int)) + tamanioRuta);
	memcpy(leBuffer, &protocolo, sizeof(int));
	memcpy(leBuffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(leBuffer + (2 * sizeof(int)), ruta, tamanioRuta);
	memcpy(leBuffer + (2 * sizeof(int)) + tamanioRuta, &tamanioLectura, sizeof(int));
	memcpy(leBuffer + (3 * sizeof(int)) + tamanioRuta, &off, sizeof(int));
	send(pokedexServidor, leBuffer, tamanioRuta + (4 * sizeof(int)), MSG_WAITALL);

	recv(pokedexServidor, &tamanioRespuesta, sizeof(int), MSG_WAITALL);
	if(tamanioRespuesta == 0){
		goto terminar;
	}
	void *contenido = malloc(tamanioRespuesta);
	recv(pokedexServidor, contenido, tamanioRespuesta, MSG_WAITALL);
	memcpy(buf, ((char *)contenido), size);



	free(contenido);
	terminar:
	free(leBuffer);
	log_info(log_Cliente, "Operacion de lectura finalizada. Leidos %d bytes del archivo %s", tamanioRespuesta, path);
	return tamanioRespuesta;
}
//////////////////////////////////////////////////////////////////////////////
/* Crea un archivo vacio*/
static int cliente_create(const char* path, mode_t modo, struct fuse_file_info * fi){


	int res; // 0 para exito y 1 para error
	log_info(log_Cliente, "Solicitud de creacion del archivo %s", path);
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
		log_info(log_Cliente, "El archivo %s fue creado exitosamente", path);
	}
	else{
		log_error(log_Cliente, "Error al crear el archivo %s. Espacio insuficiente en disco o nombre de archivo inválido", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
/* Implementacion de la operacion write sobre un archivo*/
static int cliente_write(const char* path,const char *buf, size_t size, off_t offset, struct fuse_file_info* fi){

	int res = 1;
	log_info(log_Cliente, "Peticion de escritura sobre archivo %s", path);
	protocolo = 4;
	char *ruta = (char *)path;
	int tamanioRuta = strlen(ruta);
	int off = offset;
	int buf_size = size;
	void *buffer = malloc((4 * sizeof(int)) + tamanioRuta + buf_size);
	memcpy(buffer, &protocolo, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(buffer + (2 * sizeof(int)), &buf_size, sizeof(int));
	memcpy(buffer + (3 * sizeof(int)), ruta, tamanioRuta);
	memcpy(buffer + (3 * sizeof(int)) + tamanioRuta, buf, buf_size);
	memcpy(buffer + (3 * sizeof(int)) + tamanioRuta + buf_size, &off, sizeof(int));

	send(pokedexServidor, buffer, (4 * sizeof(int)) + tamanioRuta + size, MSG_WAITALL);

	recv(pokedexServidor,&res,sizeof(int),MSG_WAITALL);

	if (res==size){
		log_info(log_Cliente, "Archivo %s escrito exitosamente. %d bytes fueron escritos", path, buf_size);
	}
	else{
		log_error(log_Cliente, "No se pudo escribir el archivo %s (espacio en disco insuficiente?)", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
static int cliente_unlink(const char* path){

	int res;
	log_info(log_Cliente, "Peticion de operacion borrado del archivo %s", path);
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
		log_info(log_Cliente, "Archivo %s borrado exitosamente", path);
	}
	else{
		log_error(log_Cliente, "Error al intentar borrar el archivo %s", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
static int cliente_rename(const char* path, const char* nuevoNombre){

	int res;
	log_info(log_Cliente, "Peticion de operacion (.rename) de archivo %s", path);
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
		log_info(log_Cliente, "El archivo %s fue renombrado exitosamente", path);
	}
	else{
		log_info(log_Cliente, "No se pudo renombrar el archivo %s. Nombre de archivo inválido", path);
	}
	return res;
}

// char *ipServidor = getenv("IP_SERVIDOR");
//////////////////////////////////////////////////////////////////////////////
static int cliente_mkdir(const char* path, mode_t mode){

	int res;
	log_info(log_Cliente, "Peticion de creacion de directorio %s", path);
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
		log_info(log_Cliente, "El directorio %s fue creado exitosamente", path);
	}
	else{
		log_error(log_Cliente, "No se pudo crear el directorio %s", path);
	}
	free(buffer);
	return res;

}
//////////////////////////////////////////////////////////////////////////////
static int cliente_rmdir(const char* path){

	int res;
	log_info(log_Cliente, "Peticion de borrado de directorio %s", path);
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
		log_info(log_Cliente, "Directorio %s borrador correctamente", path);
	}
	else{
		log_error(log_Cliente, "No se pudo borrar el directorio %s", path);
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////
int cliente_truncate(const char * path, off_t offset) {

	log_info(log_Cliente, "Truncado del archivo %s exitoso.", path);
	return 0;
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
	log_Cliente = log_create("Cliente.log", "PokedexCliente", true, log_level_from_string("INFO"));

	char *SERVER_IP = getenv("SERVER_IP");
	char *SERVER_PUERTO = getenv("SERVER_PUERTO");

	pokedexServidor = conectarCliente(SERVER_IP, SERVER_PUERTO);

	log_info(log_Cliente, "PokeCliente conectado al servidor. Aguardando peticiones....");


	return fuse_main(argc, argv, &cliente_oper, NULL );


return 0;


}
