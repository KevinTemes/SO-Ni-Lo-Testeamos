/*
 ============================================================================
 Name        : SistemaOsada.c
 Author      : Nicobile
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "osada.h"


//saca el tamaño en bits del archivo
int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //volver a la posicion original
    return sz;
}

void datos(FILE *archivo){
	int sz;


	    //recorro el archivo para saber el tamaño
		sz = fsize(archivo);

		//segun el tamaño en bytes del archivo, divido para obtener la cantidad de bloques total
		int bloques = sz/64;

		//muestro cuantos bytes y bloques tiene el archivo (aunque no lo crean me sirve para ser feliz)
		printf("%d bytes\n",sz);
		printf("%d bloques en total\n",bloques);

	    //header siempre es 1
	    printf("1 bloque HEADER\n");

	    //BITMAP ----> N bloques = F/8/OSADA_BLOCK_SIZE
	    int N = bloques/8/OSADA_BLOCK_SIZE;
	    printf("%d bloques en el BITMAP\n",N);

	    //la tabla de archivos siempre tiene 1024 bloques
	    printf("1024 bloques en TABLA DE ARCHIVOS\n");

	    //TABLA DE ASIGNACIONES ----> A bloques = (F-1-N-1024) * 4 / OSADA_BLOCK_SIZE
	    int A = ((bloques-1-N-1024)*4)/OSADA_BLOCK_SIZE;
	    printf("%d bloques en TABLA DE ASIGNACIOINES\n",A);

	    //BLOQUES DE DATOS ------> X bloques = F-1-N-1024-A
	    int X = bloques-1-N-1024-A;
	    printf("%d BLOQUES DE DATOS\n",X);
}

void seekBloques(FILE* archivo,int cantidad){
	fseek(archivo,cantidad*(OSADA_BLOCK_SIZE),SEEK_CUR);
}

int main() {
	FILE* archivo;
	if ((archivo = fopen("/home/utnso/workspace/SistemaOsada/archivoEjemplo.txt" , "r")) == NULL) {
					printf("No se pudo abrir archivo\n");
					return -99;
			}


	//datos del archivo, no lo pongo en funcion auxiliar porque despues necesito los int


	int sz;


	//recorro el archivo para saber el tamaño
	sz = fsize(archivo);

	//segun el tamaño en bytes del archivo, divido para obtener la cantidad de bloques total
	int bloques = sz/64;

	//muestro cuantos bytes y bloques tiene el archivo
	printf("%d bytes\n",sz);
	printf("%d bloques en total\n",bloques);

    //header siempre es 1
	printf("1 bloque HEADER\n");

    //BITMAP ----> N bloques = F/8/OSADA_BLOCK_SIZE
    int N = bloques/8/OSADA_BLOCK_SIZE;
	printf("%d bloques en el BITMAP\n",N);

	//la tabla de archivos siempre tiene 1024 bloques
	printf("1024 bloques en TABLA DE ARCHIVOS\n");

    //TABLA DE ASIGNACIONES ----> A bloques = (F-1-N-1024) * 4 / OSADA_BLOCK_SIZE
	int A = ((bloques-1-N-1024)*4)/OSADA_BLOCK_SIZE;
	printf("%d bloques en TABLA DE ASIGNACIOINES\n",A);

	//BLOQUES DE DATOS ------> X bloques = F-1-N-1024-A
	int X = bloques-1-N-1024-A;
    printf("%d BLOQUES DE DATOS\n",X);













    osada_header head;
    osada_file tablaArchivo;

    //leo un header
    fread(&head,sizeof(osada_header),1,archivo);

    //esto muestra el header
    printf("\n\n----HEADER----\n\n");
    int i=0;
    for(i;i<7;i++){
    	char c;
    	c = head.magic_number[i];
    	printf("%c",c);
    }
    printf("\nVersion: %d\n",head.version);
    printf("Tamaño del FS (en bloques): %d\n",head.fs_blocks);
    printf("Tamaño del Bitmap (en bloques): %d\n",head.bitmap_blocks);
    printf("Inicio de Tabla de Asignaciones (bloque): %d\n",head.allocations_table_offset);
    printf("Tamaño de Datos: %d\n",head.data_blocks);

    int h;
    //multiplico N bytes del bitmap por el tamaño de un bloque para desplazarme esa cantidad y saltear el bitmap
    h = fseek(archivo,(N*OSADA_BLOCK_SIZE),SEEK_CUR);
    //seekBloques(archivo,N);
    if(h!=0){
    	perror("Error con el seek");
    	exit(1);
    }

    fread(&tablaArchivo,sizeof(osada_file),1,archivo);

    printf("\n\n----TABLA----\n\n");
    printf("Estado: %d\n",tablaArchivo.state);
    int j=0;
    printf("Nombre del archivo: ");
    for(j;j<17;j++){
    	char j;
    	j=tablaArchivo.fname[i];
    	printf("%c",j);
    }
    printf("\nBloque Padre: %d\n",tablaArchivo.parent_directory);
    printf("Tamaño del Archivo: %d\n",tablaArchivo.file_size);
    printf("Fecha de ultima modificacion: %d\n",tablaArchivo.lastmod);
    printf("Bloque inicial: %d\n",tablaArchivo.first_block);


    fclose(archivo);

	return 0;
}

