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

/*  acordarse de chequear si hay que poner -DFUSE_USE_VERSION=27 y -D_FILE_OFFSET_BITS=64
 * como parámetros de compilación */

//-----------------------------------------------------
int pokedexServidor;
int protocolo;
int* pmap_arch;
struct stat archivoStat;
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

	void *leBuffer = malloc(sizePath + sizeProtocolo);

	// Aca tengo que pasar los sizes para poder saber donde termina al path y donde empieza el protocolo
	memcpy(leBuffer,&sizePath, sizeof(int));
	memcpy(leBuffer + sizeof(int), &sizeProtocolo, sizeof(int));

	//Aca les paso el path y protocolo
	memcpy(leBuffer + (2 * sizeof(int)), path, sizePath);
	memcpy(leBuffer + (2 * sizeof(int)) + sizePath , &protocolo, sizeProtocolo);

	send(pokedexServidor,leBuffer, sizePath + sizeProtocolo, 0);
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
	solicitarServidor(path,protocolo);
	int tipoFile = recibirTipoFile();
	memset(stbuf,0,sizeof(struct stat));

	if (tipoFile == 2){ // Es un directorio
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else if (tipoFile == 1){ // Es un archivo regular
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
	}
	else{
		res= -ENOENT;
	}
	return res;
}

/* Implementacion del comando "ls" para fuse*/
static int cliente_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) {
	int res= 0, i=0;
	protocolo = 1;
	solicitarServidor(path,protocolo);
	char* listadoConcatenado = recibirListado(); // Listado de archivos en forma de string
	char** archivos = string_split(listadoConcatenado,";");

	while (archivos[i] != NULL){
		filler(buf, archivos[i], NULL, 0);
		i++;
	}
	if (archivos[i] == NULL){
		res = -ENOENT;
	}
	return res;
}

static int cliente_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { // Fijarse bien esta
	protocolo = 2;
	char* contenido = NULL;
	solicitarServidor(path,protocolo);
	contenido =(char*) recibirContenidoArchivo();
	memcpy(buf,(contenido + offset),size);
	return size;
}

/* Crea un archivo vacio*/
static int cliente_crearArchivo(const char* path, mode_t modo, struct fuse_file_info * fi){ //Por ahora asumimos que el nombre del archivo esta en el path
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
static int cliente_modificarArchivo(const char* path,const char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
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

static int cliente_borrarArchivo(const char* path){
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

static int cliente_renombrar(const char* path, const char* nuevoNombre){
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

static int cliente_crearDirectorio(){
	protocolo = 6;
}

static int cliente_borrarDirectorio(){
	protocolo = 7;
}

static int cliente_duplicarArchivo(const char* pathOrigen, const char* pathDestino){
	protocolo = 9;
}
//--------------------------------------------------------------------------------


static struct fuse_operations cliente_oper = {
		.getattr = cliente_getattr,
		.readdir = cliente_readdir,
		.read = cliente_read,
		.create = cliente_crearArchivo,
		.write = cliente_modificarArchivo,
		.unlink = cliente_borrarArchivo,
		.mkdir = cliente_crearDirectorio,
		.rmdir = cliente_borrarDirectorio,
		.rename = cliente_renombrar,
};


int main(int argc, char *argv[]) {

 return fuse_main(argc, argv, &cliente_oper, NULL );

pokedexServidor = conectarCliente(IP, PUERTO);

/* gilada para el primer checkpoint */

int enviar = 1;
char message[PACKAGESIZE];
char *resultado = malloc(sizeof(int));
int resultadoEnvio = 0;
printf("Conectado al servidor. Bienvenido a la enciclopedia global Pokemon! cerrar para salir\n");

while(enviar != 0){
	fgets(message, PACKAGESIZE, stdin);
	if (!strcmp(message,"cerrar\n")) enviar = 0;
	send(pokedexServidor, message, strlen(message) + 1, 0);
	recv(pokedexServidor, (void *)resultado, sizeof(int), 0);
	resultadoEnvio = *((int *)resultado);

	if(resultadoEnvio == 1) {
		printf("el servidor recibió el mensaje!: %d\n", resultadoEnvio);
		}
	else if(resultadoEnvio == 9){
		printf("Servidor caído! imposible reconectar. Cerrando...\n");
		exit(0);
		}
	}




/* fin gilada para el primer checkpoint */

close(pokedexServidor);

return 0;


}
