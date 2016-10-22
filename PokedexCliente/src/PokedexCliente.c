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

#define KNORMAL "\x1B[0m"
#define KROJO "\x1B[31m"
#define KVERDE "\x1B[32m"
#define KAMARILLO "\x1B[33m"
#define KAZUL "\x1B[34m"
#define KVIOLETA "\x1B[35m"
#define KCYAN "\x1B[36m"
#define KBLANCO "\x1B[37m"


/* defines para testear sockets */
#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024

/* variables de prueba para probar la implementación de fuse*/
int *pmap_red_metadata;
int *pmap_pueblo_paleta_medalla;
int *pmap_pueblo_paleta_metadata;
int *pmap_pokenest_pikachu_metadata;
int *pmap_pokenest_pikachu_pikachu001_dat;

struct stat redMetadataStat;
struct stat puebloPaletaMedallaStat;
struct stat puebloPaletaMetadataStat;
struct stat pokenestPikachuMetadataStat;
struct stat pokenestPikachu001Stat;


/* Implementacion de GetAttributes para fuse*/
int cliente_getattr(const char *path, struct stat *stbuf) {
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, "/Pokedex") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, "/Pokedex/Entrenadores") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, "/Pokedex/Entrenadores/Red") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Entrenadores/Red/Dir-de-Bill") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Entrenadores/Red/medallas") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Entrenadores/Red/metadata") == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 176; // Modificar el tamaño del buffer de acuerdo al archivo
	}else if(strcmp(path, "/Pokedex/Mapas") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu") == 0){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/metadata") == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 45; // Modificar el tamaño del buffer de acuerdo al archivo
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/Pikachu001.dat") == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 1870; // Modificar el tamaño del buffer de acuerdo al archivo
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/medalla-PuebloPaleta.jpg") == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 31620;
	}else if(strcmp(path, "/Pokedex/Mapas/PuebloPaleta/metadata") == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = 97;
	}else {
		res = -ENOENT;
	}

	return res;
}

/* Implementacion del comando "ls" para fuse*/
static int cliente_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {

	int res = 0;

	if (strcmp(path, "/") == 0) {
		filler(buf, "Pokedex", NULL, 0);
	} else if (strcmp(path, "/Pokedex") == 0) {
		filler(buf, "Entrenadores", NULL, 0);
		filler(buf, "Mapas", NULL, 0);
	} else if (strcmp(path, "/Pokedex/Entrenadores") == 0) {
		filler(buf, "Red", NULL, 0);
	} else if (strcmp(path, "/Pokedex/Mapas") == 0) {
		filler(buf, "PuebloPaleta", NULL, 0);
	} else if (strcmp(path, "/Pokedex/Entrenadores/Red") == 0) {
		filler(buf, "Dir-de-Bill", NULL, 0);
		filler(buf, "medallas", NULL, 0);
		filler(buf, "metadata", NULL, 0);
	}else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta") == 0) {
		filler(buf, "PokeNests", NULL, 0);
		filler(buf, "medalla-PuebloPaleta.jpg", NULL, 0);
		filler(buf, "metadata", NULL, 0);
	}else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests") == 0) {
		filler(buf, "Pikachu", NULL, 0);
	}else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu") == 0) {
		filler(buf, "metadata", NULL, 0);
		filler(buf, "Pikachu001.dat", NULL, 0);
	}else {
		res = -ENOENT;
	}

	return res;
}

/* Implementacion de lectura de archivo*/
static int cliente_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	if (strcmp(path, "/Pokedex/Entrenadores/Red/metadata") == 0) {
		memcpy(buf,((char*)pmap_red_metadata + offset),size);
	} else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/medalla-PuebloPaleta.jpg") == 0) {
		memcpy(buf,((char*)pmap_pueblo_paleta_medalla + offset),size);
	} else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/metadata") == 0) {
		memcpy(buf,((char*)pmap_pueblo_paleta_metadata + offset),size);
	} else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/metadata") == 0) {
		memcpy(buf,((char*)pmap_pokenest_pikachu_metadata + offset),size);
	} else if (strcmp(path, "/Pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/Pikachu001.dat") == 0) {
		memcpy(buf,((char*)pmap_pokenest_pikachu_pikachu001_dat + offset),size);
	}
	return size;
}

/* Esta función se usa para indicarle a fuse con qué función de las nuestras "sobreeescribir" cada solicitud
 * que haga el filesystem */
static struct fuse_operations cliente_oper = {
		.getattr = cliente_getattr,
		.readdir = cliente_readdir,
		.read = cliente_read,
};

int main(int argc, char *argv[]) {

/* Campos de prueba para testear el filesystem con fuse. Claramente están hardcodeados, después hay
 * que implementar la solución dinámica...*/


// char *ipServidor = getenv("IP_SERVIDOR");

// metadata de Red
int fd_red_metadata;
fd_red_metadata= open("/home/utnso/workspace/pokedex/Entrenadores/Red/metadata",O_RDWR);
fstat(fd_red_metadata,&redMetadataStat);
pmap_red_metadata= mmap(0, redMetadataStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_red_metadata, 0);

// metadata de Pueblo Paleta
int fd_puebloPaleta_metadata;
fd_puebloPaleta_metadata= open("/home/utnso/workspace/pokedex/Mapas/PuebloPaleta/metadata",O_RDWR);
fstat(fd_puebloPaleta_metadata,&puebloPaletaMetadataStat);
pmap_pueblo_paleta_metadata= mmap(0, puebloPaletaMetadataStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_puebloPaleta_metadata, 0);

// medalla de Pueblo Paleta
int fd_medallaPuebloPaleta;
fd_medallaPuebloPaleta= open("/home/utnso/workspace/pokedex/Mapas/PuebloPaleta/medalla-PuebloPaleta.jpg",O_RDWR);
fstat(fd_medallaPuebloPaleta,&puebloPaletaMedallaStat);
pmap_pueblo_paleta_medalla= mmap(0, puebloPaletaMedallaStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_medallaPuebloPaleta, 0);

// metadata de la PokeNest Pikachu
int fd_PikachuMetadata;
fd_PikachuMetadata = open("/home/utnso/workspace/pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/metadata",O_RDWR);
fstat(fd_PikachuMetadata, &pokenestPikachuMetadataStat);
pmap_pokenest_pikachu_metadata = mmap(0, pokenestPikachuMetadataStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_PikachuMetadata, 0);

// instancia Pikachu001.dat de la PokeNest Pikachu
int fd_Pikachu001Dat;
fd_Pikachu001Dat = open("/home/utnso/workspace/pokedex/Mapas/PuebloPaleta/PokeNests/Pikachu/Pikachu001.dat",O_RDWR);
fstat(fd_Pikachu001Dat, &pokenestPikachu001Stat);
pmap_pokenest_pikachu_metadata = mmap(0, pokenestPikachu001Stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_Pikachu001Dat, 0);


/* Para testear la parte de FUSE, descomentar la próxima linea, y comentar  lo que viene después.*/

// return fuse_main(argc, argv, &cliente_oper, NULL );

int servidor;
servidor = conectarCliente(IP, PUERTO);

/* gilada para el primer checkpoint */
int enviar = 1;
char message[PACKAGESIZE];
char *resultado = malloc(sizeof(int));
int resultadoEnvio = 0;
printf("Conectado al servidor. Bienvenido a la enciclopedia global Pokemon! ingrese el mensaje que desee enviar, o cerrar para salir\n");

while(enviar != 0){
	fgets(message, PACKAGESIZE, stdin);
	if (!strcmp(message,"cerrar\n")) enviar = 0;
	send(servidor, message, strlen(message) + 1, 0);
	recv(servidor, (void *)resultado, sizeof(int), 0);
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

close(servidor);

return 0;


}
