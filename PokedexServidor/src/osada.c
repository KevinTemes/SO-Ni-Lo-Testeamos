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
#include <commons/log.h>
#include <commons/string.h>
#include <sys/mman.h>

archivoOsada* archi;
extern t_log* logs;

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
		log_info(logs,"%d bytes\n",sz);
		log_info(logs,"%d bloques en total\n",bloques);
		(archi)->bloquesTotales= bloques;

	    //header siempre es 1
	    log_info(logs,"1 bloque HEADER\n");
	    (archi)->bloqueHeader=1;

	    //BITMAP ----> N bloques = F/8/OSADA_BLOCK_SIZE
	    int N = bloques/8/OSADA_BLOCK_SIZE;
	    log_info(logs,"%d bloques en el BITMAP\n",N);
	    (archi)->bloquesBitmap=N;

	    //la tabla de archivos siempre tiene 1024 bloques
	    log_info(logs,"1024 bloques en TABLA DE ARCHIVOS\n");
	    (archi)->bloquesTablaArchivos=1024;

	    //TABLA DE ASIGNACIONES ----> A bloques = (F-1-N-1024) * 4 / OSADA_BLOCK_SIZE
	    int A = ((bloques-1-N-1024)*4)/OSADA_BLOCK_SIZE;
	    log_info(logs,"%d bloques en TABLA DE ASIGNACIONES\n",A);
	    (archi)->bloquesTablaAsignaciones=A;

	    //BLOQUES DE DATOS ------> X bloques = F-1-N-1024-A
	    int X = bloques-1-N-1024-A;
	    log_info(logs,"%d BLOQUES DE DATOS\n",X);
	    (archi)->bloquesDeDatos=X;

}

void seekBloques(FILE* archivo,int cantidad){
	fseek(archivo,cantidad*(OSADA_BLOCK_SIZE),SEEK_CUR);
}

int osada(osada_header *head, osada_file *tablaArchivo, int *fd) {

// Antes esto tiraba error de tipos, cambie el osada.h para que reciba punteros
// Porque antes recibia solo las structs

	FILE* archivo;
	if ((archivo = fopen("../disco.bin" , "r")) == NULL) {
		log_error(logs,"No se pudo abrir archivo\n");

		return -99;
			}


	//datos del archivo, no lo pongo en funcion auxiliar porque despues necesito los int
	int sz;


	//recorro el archivo para saber el tamaño
	sz = fsize(archivo);

	//segun el tamaño en bytes del archivo, divido para obtener la cantidad de bloques total
	int bloques = sz/64;


    //BITMAP ----> N bloques = F/8/OSADA_BLOCK_SIZE
    int N = bloques/8/OSADA_BLOCK_SIZE;


    //TABLA DE ASIGNACIONES ----> A bloques = (F-1-N-1024) * 4 / OSADA_BLOCK_SIZE
	int A = ((bloques-1-N-1024)*4)/OSADA_BLOCK_SIZE;


	//BLOQUES DE DATOS ------> X bloques = F-1-N-1024-A
	int X = bloques-1-N-1024-A;


	//leo un header
    fread(head,sizeof(osada_header),1,archivo);

    //esto muestra el header
    log_info(logs,"\n\n----HEADER----\n\n");
    /*int i;
    for(i=0;i<7;i++){
    	char c;
    	c = head->magic_number[i];
    	printf("%c", c);
    }*/

    // Mapeo el archivo en un puntero, con mmap
    fd = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


    log_info(logs, "Identificador: %s\n", head->magic_number);
    log_info(logs,"Version: %d\n",head->version);
    log_info(logs,"Tamaño del FS (en bloques): %d\n",head->fs_blocks);
    log_info(logs,"Tamaño del Bitmap (en bloques): %d\n",head->bitmap_blocks);
    log_info(logs,"Inicio de Tabla de Asignaciones (bloque): %d\n",head->allocations_table_offset);
    log_info(logs,"Tamaño de Datos: %d\n",head->data_blocks);

    int h=0; // despues lo cambiamos, sino tira warning

    //multiplico N bytes del bitmap por el tamaño de un bloque para desplazarme esa cantidad y saltear el bitmap
    //h = fseek(archivo,(N*OSADA_BLOCK_SIZE),SEEK_CUR); seria la funcion equivalente al seekBloques
    //ahorra tiempo, que se yo

    seekBloques(archivo,N);

    if(h!=0){
    	perror("Error con el seek");
    	exit(1);
    }

    fread(tablaArchivo,sizeof(osada_file),1,archivo);

    log_info(logs,"\n\n----TABLA----\n\n");
    log_info(logs,"Estado: %c\n",tablaArchivo->state);
    /*int j;
    for(j=0;j<17;j++){
    	char a;
    	a=tablaArchivo->fname[j];
    	printf("%c",a);
    }*/

    log_info(logs, "Nombre del archivo: %s \n",tablaArchivo->fname);
    log_info(logs,"Bloque Padre: %d\n",tablaArchivo->parent_directory);
    log_info(logs,"Tamaño del Archivo: %d\n",tablaArchivo->file_size);
    log_info(logs,"Fecha de ultima modificacion: %d\n",tablaArchivo->lastmod);
    log_info(logs,"Bloque inicial: %d\n\n",tablaArchivo->first_block);


    fclose(archivo);

	return 0;
}

