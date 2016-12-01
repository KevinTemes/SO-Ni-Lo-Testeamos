/*
 * libreriaPokedexServidor.c
 *
 *  Created on: 7/9/2016
 *      Author: utnso
 */

#include "libreriaPokedexServidor.h"

t_log* log_Servidor;

////////////////////////////////////////////////////////////////////////////////////////////////////////

int buscarArchivo(char *unaRuta){
	int parentDir = 65535;
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
		for(a = 0; separadas[a] != NULL; a++){
			free(separadas[a]);
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
		}

	}

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
char *convertirString(void *buffer, int tamanioRuta){
	char *nuevaRuta = string_new();
	char *puente = string_new();
	puente = (char *)buffer;
	nuevaRuta = string_substring_until(puente, tamanioRuta);
	nuevaRuta[tamanioRuta] = '\0';
	//free(puente);
	return nuevaRuta;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void atenderConexion(void *numeroCliente){
	int unCliente = *((int *) numeroCliente);
	int status = 1;
	int codOp, exito, nuevoTamanio;
	int tamanioRuta, tamanioNombre, tamanioNuevoContenido, tamanioLectura, offset;
	char *ruta = string_new();
	char *contenidoDir = string_new();
	char *nombre = string_new();
	void *buffer, *bufferDir, *nuevoContenido, 	*bufferContenido;;
	void *respuesta;

//	remove("Servidor.log");
//	log_Servidor = log_create("Servidor.log", "libreriaPokedexServidor", false, log_level_from_string("INFO"));

	log_info(log_Servidor, "PokeCliente #%d conectado al servidor", clientesActivos[unCliente].cliente);

	while(status !=0){


		if (status != 0) {
			status = recv(clientesActivos[unCliente].socket, &codOp, sizeof(int), MSG_WAITALL);

			switch(codOp){

			case 0: // .getAttr

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				ruta = convertirString(buffer, tamanioRuta);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				t_getattr archivo = osada_getattr(ruta);
				respuesta = malloc(2 * sizeof(int));
				memcpy(respuesta, &archivo.tipo_archivo, sizeof(int));
				memcpy(respuesta + sizeof(int), &archivo.size, sizeof(int));
				send(clientesActivos[unCliente].socket, respuesta, 2 * sizeof(int), MSG_WAITALL);

				free(respuesta);
				free(buffer);
				free(ruta);


			break;

			case 1: // .readdir (- ls)

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
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
					send(clientesActivos[unCliente].socket, bufferDir, sizeof(int) + tamanio, MSG_WAITALL);
				}
				//*ruta = '\0';
				//*contenidoDir = '\0';
				free(buffer);
				free(bufferDir);
				free(ruta);
				free(contenidoDir);


			break;

			case 2: // .read

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &tamanioLectura, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &offset, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				int j = buscarArchivo(ruta);
				int tamanioArchivo = (int)miDisco.tablaDeArchivos[j].file_size;

				if(tamanioArchivo == 0){
					bufferContenido = malloc(sizeof(int));
					memcpy(bufferContenido, &tamanioArchivo, sizeof(int));
					send(clientesActivos[unCliente].socket, bufferContenido, sizeof(int), MSG_WAITALL);
				}
				else{
					void *contenidoArchivo = osada_read(ruta, tamanioLectura, offset);
					bufferContenido = malloc(tamanioLectura + sizeof(int));
					memcpy(bufferContenido, &tamanioLectura, sizeof(int));
					memcpy(bufferContenido + sizeof(int), contenidoArchivo, tamanioLectura);
					send(clientesActivos[unCliente].socket, bufferContenido, sizeof(int) + tamanioLectura, MSG_WAITALL);
					free(contenidoArchivo);
					free(bufferContenido);
				}
				*ruta = '\0';
				free(ruta);
				free(buffer);
			break;

			case 3: // .create (por ahora supongo que el nombre ya viene en la ruta)

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}

				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_create(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(ruta);

			break;

			case 4: // .write



				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &tamanioNuevoContenido, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				nuevoContenido = malloc(tamanioNuevoContenido);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, nuevoContenido, tamanioNuevoContenido, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &offset, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_write(ruta, nuevoContenido, tamanioNuevoContenido, offset);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(nuevoContenido);
				free(ruta);

			break;

			case 5: // .unlink

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_unlink(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(ruta);

			break;

			case 6: // .mkdir

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_mkdir(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(ruta);

			break;

			case 7: // .rmdir

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_rmdir(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(ruta);

			break;

			case 8: // .rename

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &tamanioNombre, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				void *bufferNombre = malloc(tamanioNombre);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, bufferNombre, tamanioNombre, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				nombre = convertirString(bufferNombre, tamanioNombre);
				exito = osada_rename(ruta, nombre);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(bufferNombre);
				free(ruta);

			break;

			case 9: // .open

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_open(ruta);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(buffer);
				free(ruta);

			break;

			case 10: // .truncate

				status = recv(clientesActivos[unCliente].socket, &tamanioRuta, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				buffer = malloc(tamanioRuta);
				status = recv(clientesActivos[unCliente].socket, buffer, tamanioRuta, MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				status = recv(clientesActivos[unCliente].socket, &nuevoTamanio, sizeof(int), MSG_WAITALL);
				if(status == 0 || status == -1){
					status = 0;
					break;
				}
				ruta = convertirString(buffer, tamanioRuta);
				exito = osada_truncate(ruta, nuevoTamanio);
				send(clientesActivos[unCliente].socket, &exito, sizeof(int), MSG_WAITALL);
				free(ruta);



			}
		}
	}
	//free(bufferContenido);
	//free(bufferDir);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void actualizarTablaDeArchivos(){
	int inicioTabla = ((miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap) * 64);
	memcpy(miDisco.discoMapeado + inicioTabla, &miDisco.tablaDeArchivos, sizeof(osada_file) * 2048);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int primerBloqueBitmapLibre(){
	int pos = -1;

	int bloquesDeDatos = miDisco.header->fs_blocks;
	int inicio = inicioDeDatosEnBloques();

	int i;
	pthread_mutex_lock(&mutex_bloques);
	for(i = inicio; i <= bloquesDeDatos; i++){
		if(!bitarray_test_bit(miDisco.bitmap, i)){
			pos = i;
			bitarray_set_bit(miDisco.bitmap, i);
			break;
		}
	}
	pthread_mutex_unlock(&mutex_bloques);


	return pos;
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
	log_info(log_Servidor, "Recibida solicitud .getattr sobre la ruta %s", unaRuta);

	if (posicion == 65535){
		atributos.tipo_archivo = 2;
		atributos.size = 0;

		log_info(log_Servidor, "Solicitud .getattr sobre la ruta %s completa.", unaRuta);
	}
	else if(posicion == -1){
		atributos.tipo_archivo = 0;
		atributos.size = 0;
	}
	else{
		if(miDisco.tablaDeArchivos[posicion].state == REGULAR){
			atributos.tipo_archivo = 1;
			atributos.size = (int) miDisco.tablaDeArchivos[posicion].file_size;

			log_info(log_Servidor, "Solicitud .getattr sobre la ruta %s completa.", unaRuta);
		}
		else if(miDisco.tablaDeArchivos[posicion].state == DIRECTORY){
			atributos.tipo_archivo = 2;
			atributos.size = 0;

			log_info(log_Servidor, "Solicitud .getattr sobre la ruta %s completa.", unaRuta);
		}
	}



return atributos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
char *osada_readdir(char *unaRuta){

	char *contenidoDir = string_new();
	char *separador = ";";
	int parentBlock = buscarArchivo(unaRuta);
	int numElems = 0;
	int i;

	log_info(log_Servidor, "Solicitud de lectura de directorio (.readdir) sobre la ruta %s", unaRuta);

	if(parentBlock == 999999){
		log_error(log_Servidor, "no existe el directorio o subdirectorio especificado\n");
		exit(0);
	}


	// Busco todos los archivos hijos de ese directorio padre, y me copio el nombre de cada uno
	for (i = 0; i <= 2047; i++){

			if(miDisco.tablaDeArchivos[i].parent_directory == parentBlock
					&& miDisco.tablaDeArchivos[i].state != DELETED){

				if(strlen(miDisco.tablaDeArchivos[i].fname) == 17){
					char *nombreLargo = string_substring(miDisco.tablaDeArchivos[i].fname, 0, 17);
					string_append(&contenidoDir, nombreLargo);
				}
				else{
					string_append(&contenidoDir, miDisco.tablaDeArchivos[i].fname);
				}
				string_append(&contenidoDir, "\0");

				// Concateno los nombres en un string, separados por la variable separador.

				string_append(&contenidoDir, separador);
				string_append(&contenidoDir, "\0");
				numElems++;

			}

		}

	if(numElems == 0){
		string_append(&contenidoDir, "\0");
	}

	log_info(log_Servidor, "Operacion .readdir sobre la ruta %s finalizada.", unaRuta);

return contenidoDir;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_open(char *ruta){

	log_info(log_Servidor, "Solicitud para abrir archivo (.open) %s", ruta);
	int exito = -1;
	t_getattr archivo = osada_getattr(ruta);
	if(archivo.tipo_archivo != 0){
		exito = 0;
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void *osada_read(char *ruta, int tamanioLectura, int offset){

	int i = buscarArchivo(ruta);

	log_info(log_Servidor, "Solicitud de lectura (.read) del archivo %s", ruta);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	if(miDisco.tablaDeArchivos[i].file_size == 0){
		goto terminar;
	}
	void *buffer;

	pthread_mutex_lock(&misMutex[i]);

		buffer = malloc(tamanioLectura);
		int tamanioActualBuffer = 0;
		int inicioDatos = inicioDeDatos();
		void *desplazamiento;
		int datosPendientes = tamanioLectura;
		int bloquesAsaltear = calcularBloquesNecesarios(offset);
		int bloquesSalteados = 0;
		int espacioEnElUltimoBloqueSalteado = hayLugarEnElUltimoBloque(offset);
		int offset_bloque = 64 - espacioEnElUltimoBloqueSalteado;

		while((datosPendientes > 0) && (siguienteBloque != -1)){

			/* -- Me desplazo hasta el offset --*/
			if(bloquesSalteados < bloquesAsaltear){
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
				bloquesSalteados++;
			}

			/*-- Cuando llego al offset, copio el pedazo del bloque inicial --*/
			else if((bloquesSalteados == bloquesAsaltear) && (offset_bloque < 64)){
				desplazamiento = &miDisco.discoMapeado[inicioDatos + (siguienteBloque * 64)
								 + offset_bloque];

				/*-- Leo todos los bytes restantes del bloque --*/
				if(datosPendientes >= espacioEnElUltimoBloqueSalteado){
					memcpy(buffer + tamanioActualBuffer, desplazamiento, espacioEnElUltimoBloqueSalteado);
					datosPendientes -= espacioEnElUltimoBloqueSalteado;
					tamanioActualBuffer += espacioEnElUltimoBloqueSalteado;
				}

				/*-- Leo solo la cantidad de datos pendientes --*/
				else{
					memcpy(buffer + tamanioActualBuffer, desplazamiento, datosPendientes);
					datosPendientes -= datosPendientes;
					tamanioActualBuffer += datosPendientes;

				}
				bloquesSalteados++;
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

			}

			/*-- Leo los bloques restantes --*/
			else{
				desplazamiento = &miDisco.discoMapeado[inicioDatos + (siguienteBloque * 64)];
				if(datosPendientes >= 64){
					memcpy(buffer + tamanioActualBuffer, desplazamiento, 64);
					datosPendientes -= 64;
					tamanioActualBuffer += 64;
				}
				else{
					memcpy(buffer + tamanioActualBuffer, desplazamiento, datosPendientes);
					datosPendientes -= datosPendientes;
					tamanioActualBuffer += datosPendientes;
				}
				siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

			}
		}


	pthread_mutex_unlock(&misMutex[i]);


terminar:
return(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_create(char *ruta){

	log_info(log_Servidor, "Solicitud de creacion (.create) del archivo %s", ruta);
	int exito = 1;

	int existe = buscarArchivo(ruta);
	if(existe != -1){
		goto terminar;
	}

	unsigned int bloqueLibre = primerBloqueBitmapLibre();
	char *nombreArchivo = obtenerNombre(ruta);
	int largo = strlen(nombreArchivo);
	int posTablaArchivos = buscarPosicionLibre();
	if(bloqueLibre >= 0 && posTablaArchivos >= 0 && largo <= 17){
		int largoRuta = strlen(ruta);
		char *dirPadre = string_substring_until(ruta, largoRuta - largo);
		int parentDir = buscarArchivo(dirPadre);
		pthread_mutex_lock(&misMutex[posTablaArchivos]);
		crearArchivo(nombreArchivo, parentDir, bloqueLibre, posTablaArchivos);
		pthread_mutex_unlock(&misMutex[posTablaArchivos]);
		exito = 0;
	}

	if(exito == 0){
		log_info(log_Servidor, "El archivo %s se creo exitosamente", ruta);
	}
	else{
		if(bloqueLibre < 0){
			log_error(log_Servidor, "Error al crear el archivo %s: espacio en disco insuficiente", ruta);
		}
		if(posTablaArchivos < 0){
			log_error(log_Servidor, "Error al crear el archivo %s: se ha alcanzado el número máximo de"
					"archivos soportados por el sistema", ruta);
		}
		if(largo > 17){
			log_error(log_Servidor, "Error al crear el archivo %s: el nombre del archivo no es válido"
					"(longitud superior a 17 caracteres)", ruta);
		}
		log_error(log_Servidor, "No se pudo crear el archivo");
	}

terminar:
return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_write(char *ruta, void *nuevoContenido, int sizeAgregado, int offset){

	log_info(log_Servidor, "Solicitud de escritura (.write) del archivo %s", ruta);
	int exito = -1;
	int i = buscarArchivo(ruta);
	int tamanioActualArchivo = miDisco.tablaDeArchivos[i].file_size;
	int solicitud = hayEspacioEnDisco(tamanioActualArchivo, sizeAgregado, offset);

	if(solicitud < 0){
		goto fin;
	}

	int inicio = inicioDeDatos();

	pthread_mutex_lock(&misMutex[i]);

	// CASO APPEND (OFFSET = TAMANIO ACTUAL DEL ARCHIVO)
	if(tamanioActualArchivo == offset){

		int espacioEnUltimoBloque = hayLugarEnElUltimoBloque(tamanioActualArchivo);
		int siguienteBloque = ultimoBloqueAsignado(i);
		void *desplazamiento;
		int datosCopiados = 0;
		int datosPendientes = sizeAgregado;
		int aux;

		//  SI EL ULTIMO BLOQUE NO ESTA COMPLETO, LO RELLENO

		if(espacioEnUltimoBloque > 0){
			desplazamiento = nuevoContenido + datosCopiados;
			memcpy(&miDisco.discoMapeado[inicio + (siguienteBloque * 64) + (64 - espacioEnUltimoBloque)], desplazamiento, espacioEnUltimoBloque);
			datosCopiados += espacioEnUltimoBloque;
			datosPendientes -= espacioEnUltimoBloque;
		}

		miDisco.tablaDeAsignaciones[siguienteBloque] = primerBloqueBitmapLibre();
		aux = siguienteBloque;
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

		//  Asigno bloque, escribo bloque, repeat, profit

		while(datosPendientes > 0){
			desplazamiento = nuevoContenido + datosCopiados;
			if(datosPendientes >= 64){
				memcpy(&miDisco.discoMapeado[inicio + (siguienteBloque * 64)], desplazamiento, 64);
				datosPendientes -= 64;
				datosCopiados += 64;
			}
			else{
				memcpy(&miDisco.discoMapeado[inicio + (siguienteBloque * 64)], desplazamiento, datosPendientes);
				datosCopiados += datosPendientes;
				datosPendientes = 0;
			}

			bitarray_set_bit(miDisco.bitmap, siguienteBloque);
			aux = siguienteBloque;
			miDisco.tablaDeAsignaciones[siguienteBloque] = primerBloqueBitmapLibre();
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];

		}

		miDisco.tablaDeArchivos[i].file_size = offset + sizeAgregado;
		miDisco.tablaDeAsignaciones[aux] = -1;

		log_info(log_Servidor, "Cantidad de bytes escritos: %d", sizeAgregado);
		log_info(log_Servidor, "Tamanio del archivo despues de escritura: %d", miDisco.tablaDeArchivos[i].file_size);
	}

	// CASO TRUNCATE (Offset en cero, el archivo se achica)

	if(offset == 0 && sizeAgregado < tamanioActualArchivo){
		int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
		void *desplazamiento;
		int datosCopiados = 0;
		int datosPendientes = sizeAgregado;
		int aux;

		// Verifico si el tamaño que me pasan es menor al de un bloque

		while(datosPendientes > 0){
			desplazamiento = nuevoContenido + datosCopiados;
			if (datosPendientes >= 64){
				memcpy(&miDisco.discoMapeado[inicio + (siguienteBloque * 64)], desplazamiento, 64);
				datosPendientes -= 64;
				datosCopiados += 64;
			}
			else{
				memcpy(&miDisco.discoMapeado[inicio + (siguienteBloque * 64)], desplazamiento, datosPendientes);
				datosCopiados += datosPendientes;
				datosPendientes = 0;
			}
			if(datosPendientes > 0){
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
			}
		}

		while(siguienteBloque != -1){
			bitarray_clean_bit(miDisco.bitmap, miDisco.tablaDeAsignaciones[siguienteBloque]);
			aux = miDisco.tablaDeAsignaciones[siguienteBloque];
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			siguienteBloque = aux;
		}

		miDisco.tablaDeArchivos[i].file_size = sizeAgregado;
		log_info(log_Servidor, "Tamanio del archivo despues de la escritura: %d", miDisco.tablaDeArchivos[i].file_size );
	}

	miDisco.tablaDeArchivos[i].lastmod = consultarTiempo();
	actualizarTablaDeArchivos();
	exito = sizeAgregado;

	pthread_mutex_unlock(&misMutex[i]);

fin:
return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_unlink(char *ruta){

	int exito = -1;
	log_info(log_Servidor, "Solicitud de borrado (.unlink) del archivo %s", ruta);
	int i = buscarArchivo(ruta);
	if(i >= -1){
		pthread_mutex_lock(&misMutex[i]);
		borrarArchivo(i);
		pthread_mutex_unlock(&misMutex[i]);

		exito = 0;
	}

	if(exito == 0){
		log_info(log_Servidor, "El archivo %s se ha borrado exitosamente", ruta);
	}
	else{
		log_error(log_Servidor, "Error al borrar el archivo %s", ruta);
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_mkdir(char *ruta){

	int exito = -1;
	log_info(log_Servidor, "Solicitud de creacion (.mkdir) del directorio %s", ruta);
	int posTablaArchivos = buscarPosicionLibre();
	char *nombreDir = obtenerNombre(ruta);
	int largo = strlen(nombreDir);
	nombreDir[largo] = '\0';
	if(posTablaArchivos >= 0 && largo <= 17){
		pthread_mutex_lock(&misMutex[posTablaArchivos]);
		crearDirectorio(nombreDir, ruta, posTablaArchivos);
		pthread_mutex_unlock(&misMutex[posTablaArchivos]);
		exito = 0;

	}

	if(exito == 0){
		log_info(log_Servidor, "El directorio  %s se creo exitosamente", ruta);
	}
	else{
		if(posTablaArchivos < 0){
			log_error(log_Servidor, "Error al crear el directorio: espacio insuficiente en la tabla de archivos");
		}
		if(largo > 17){
			log_error(log_Servidor, "Error al crear el directorio: nombre de directorio inválido (supera los 17 caracteres)");
		}
	}

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rmdir(char *ruta){

	int exito;
	log_info(log_Servidor, "Solicitud de borrado (.rmdir) del directorio %s", ruta);

	int i = buscarArchivo(ruta);
	pthread_mutex_lock(&misMutex[i]);
	borrarDirectorio(i);
	pthread_mutex_unlock(&misMutex[i]);
	exito = 0;
	log_info(log_Servidor, "El directorio %s se ha borrado exitosamente", ruta);

	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_rename(char *ruta, char *nuevoNombre){

	int i;
	int exito = 1;
	log_info(log_Servidor, "Solicitud de renombrado (.rename) del archivo %s", ruta);
	char *nombre = obtenerNombre(nuevoNombre);
	if(strlen(nombre) <= 17){
		i = buscarArchivo(ruta);


		pthread_mutex_lock(&misMutex[i]);

		renombrar(nombre, i);

		pthread_mutex_unlock(&misMutex[i]);

		exito = 0;
	}

	if(exito == 0){
		log_info(log_Servidor, "El archivo  %s se ha renombrado exitosamente."
				" Nuevo nombre: %s", ruta, miDisco.tablaDeArchivos[i].fname);
	}
	else{
		log_error(log_Servidor, "Error al renombrar el archivo %s:"
				" el nombre solicitado es invalido.", ruta);
	}
	return exito;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int osada_truncate(char *ruta, int nuevoTamanio){
	int exito = -1;
	int i = buscarArchivo(ruta);
	int tamanioActual = miDisco.tablaDeArchivos[i].file_size;
	int bloques = calcularBloquesNecesarios(nuevoTamanio);
	int siguienteBloque = miDisco.tablaDeArchivos[i].first_block;
	int contador = 0;
	int aux;

	log_info(log_Servidor, "Solicitud de truncado (.truncado) del archivo %s", ruta);

	pthread_mutex_lock(&misMutex[i]);

/*	if(nuevoTamanio == 0){ // Truncado a cero
		aux = siguienteBloque;
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
		miDisco.tablaDeAsignaciones[aux] = -1;

		while(siguienteBloque != -1){ // Limpio los bloques restantes
			aux = miDisco.tablaDeAsignaciones[siguienteBloque];
			bitarray_clean_bit(miDisco.bitmap, siguienteBloque);
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			siguienteBloque = aux;
		}


		exito = 0;

	} */

	if(tamanioActual > nuevoTamanio){ // Reduzco el tamaño

		while(contador <= bloques){ //Avanzo hasta el último bloque que me va a quedar
			aux = siguienteBloque;
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
			contador++;
		}

		miDisco.tablaDeAsignaciones[aux] = -1;
		while(siguienteBloque != -1){ // Limpio los bloques restantes
			aux = miDisco.tablaDeAsignaciones[siguienteBloque];
			bitarray_clean_bit(miDisco.bitmap, siguienteBloque);
			miDisco.tablaDeAsignaciones[siguienteBloque] = -1;
			siguienteBloque = aux;

		}
		exito = 0;

	}


	else{ // Agrando el archivo

		while(siguienteBloque != -1){
			aux = siguienteBloque;
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
			contador++;
		}

		int bloquesNuevos = bloques - contador;
		int hayLugar = hayBloquesLibres(bloquesNuevos);
		if(hayLugar < 0){
			goto terminar;
		}
		siguienteBloque = aux;

		while(bloquesNuevos != 0){
			miDisco.tablaDeAsignaciones[siguienteBloque] = primerBloqueBitmapLibre();
			aux = siguienteBloque;
			bitarray_set_bit(miDisco.bitmap, siguienteBloque);
			siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
			bloquesNuevos--;
		}

		miDisco.tablaDeAsignaciones[aux] = -1;
		exito = 0;
		terminar:
		exito = -1;
	}

	miDisco.tablaDeArchivos[i].file_size = nuevoTamanio;
	miDisco.tablaDeArchivos[i].lastmod = consultarTiempo();
	actualizarTablaDeArchivos();

	pthread_mutex_unlock(&misMutex[i]);


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
	actualizarTablaDeArchivos();
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
int hayBloquesLibres(int unaCantidad){
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
int inicioDeDatos(){
	int inicio = (miDisco.cantBloques.bloques_header
			+ miDisco.cantBloques.bloques_bitmap
			+ miDisco.cantBloques.bloques_tablaDeArchivos
			+ miDisco.cantBloques.bloques_tablaDeAsignaciones) * 64;

return inicio;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int hayEspacioEnDisco(int tamanioActualArchivo, size_t tamanioAgregado, off_t offset){
	int flag;
	if(tamanioActualArchivo < offset + tamanioAgregado){
		int tamanioRequerido = (offset + tamanioAgregado) - tamanioActualArchivo;
		int bloquesNecesarios = calcularBloquesNecesarios(tamanioRequerido);
		flag = hayBloquesLibres(bloquesNecesarios);
	}
	else{
		flag = 1;
	}
return flag;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int hayLugarEnElUltimoBloque(int unTamanio){
	int res;
	if(unTamanio == 0){ // El archivo está vacío
		res = 64;
	}
	else{
		div_t tamanioEnBloques = div(unTamanio, 64);
		if(tamanioEnBloques.rem == 0){ // El último bloque del archivo está completo
			res = 0;
		}
		else{ // El último bloque del archivo está incompleto
			res = (64 - tamanioEnBloques.rem);
		}
	}

	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int ultimoBloqueAsignado(int filePos){
	int siguienteBloque = miDisco.tablaDeArchivos[filePos].first_block;
	int aux;
	while(siguienteBloque != -1){
		aux = siguienteBloque;
		siguienteBloque = miDisco.tablaDeAsignaciones[siguienteBloque];
	}
return aux;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int inicioDeDatosEnBloques(){
	int inicio = miDisco.cantBloques.bloques_header + miDisco.cantBloques.bloques_bitmap
			+ miDisco.cantBloques.bloques_tablaDeArchivos
			+ miDisco.cantBloques.bloques_tablaDeAsignaciones;

return inicio;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void chequearBitmap(){
	char *root = "/";
	char *contDir = osada_readdir(root);
	if(strlen(contDir) == 0){
		int i;
		int inicio = inicioDeDatosEnBloques();
		int fin = miDisco.header->fs_blocks;
		for(i = inicio; i <= fin; i++){
			bitarray_clean_bit(miDisco.bitmap, i);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
