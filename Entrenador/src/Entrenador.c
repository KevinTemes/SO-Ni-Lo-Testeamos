/*
 ============================================================================

 ============================================================================
 Name        : Entrenador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 ============================================================================
 Name        : Entrenador.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "libSockets.h"

int servidor;
char* puntoMontaje;
t_entrenador* ent;
t_posMapaposObjetivoYDeadlocks* posicionesYDeadlocks;
char* protocAManejar;
char* configEntrenador;

#define ATRAPA 1
#define DEADLOCK 3
#define MORI 7
#define SOBREVIVI 0

int main(int argc, char* argv[]){ // PARA EJECUTAR: ./Entrenador Ash /home/utnso/workspace/pokedex
	char* nombreEnt = argv[1];
	puntoMontaje = argv[2];

	// LOGS
	 remove("Entrenador.log");
	 puts("Creando archivo de logueo...\n");
	 logs = log_create("Entrenador.log", "Entrenador", true, log_level_from_string("INFO"));
	 puts("Log Entrenador creado exitosamente \n");

	 //STRUCTS
	 ent = malloc(sizeof(t_entrenador));

	 //si rompe, meterlo adentro del for de cada pokemon
	 t_pokemonDeserializado* pokePiola;
	 pokePiola =malloc(sizeof(t_pokemonDeserializado));

	 t_tiempoBloqueado* tiempo;
	 tiempo= malloc(sizeof(t_tiempoBloqueado));

	 t_calculoTiempo* calculoTiempo;
	 calculoTiempo = malloc(sizeof(t_calculoTiempo));


	 posicionesYDeadlocks = malloc(sizeof(t_posMapaposObjetivoYDeadlocks));

	 //CONFIG
	 configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",puntoMontaje,nombreEnt);


	 if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");

	 if(signal(SIGUSR1, handler)==SIG_ERR){
		 perror ("No se puede cambiar la señal SIGUSR1");
	 }
	 if(signal(SIGTERM, handler)==SIG_ERR){
		 perror("No se puede cambiar la señal SIGTERM");
	 }

	 //VARIABLES USADAS Y CONEXION
	 int cantMapas = list_size((ent)->hojaDeViaje);
	 posicionesYDeadlocks->reintentosActualizados=0;
	 posicionesYDeadlocks->cantDeadlocks=0;

	 char* protocolo = string_new();
	 char* numConcatenado="1";
	 char* coordPokenest;
	 char* horaInicio;
	 char* resultado = malloc(sizeof(int));
	 char* miIP;
	 char* miPuerto;
	 char** posPokenest;

	 string_append(&protocolo,numConcatenado);
	 string_append(&protocolo,ent->caracter);
	 protocAManejar = strdup(protocolo);

	 horaInicio = empezarAventura();

		 do{
			 for(posicionesYDeadlocks->pos = 0;posicionesYDeadlocks->pos<cantMapas;posicionesYDeadlocks->pos++){

					miIP= list_get(ips,posicionesYDeadlocks->pos);
					miPuerto = list_get(puertos,posicionesYDeadlocks->pos);

					//printf("ip %s \n", miIP);
					//printf("puerto %s \n",miPuerto);


					servidor = conectarCliente(miIP, miPuerto);

					int resultadoEnvio = 0;

					char* mapa = list_get((ent)->hojaDeViaje,posicionesYDeadlocks->pos);

					char* mensaje = string_from_format("Conectado al Mapa %s. Ingrese el mensaje que desee enviar, o cerrar para salir\n",mapa);
					log_info(logs,mensaje);
					free(mensaje);

					//////////////// recibo y mando datos al Mapa /////////////////////

					// cuando pase a otro mapa, o lo reinicia, vuelve a arrancar en (1;1)
					int posXInicial =0;
					int posYInicial =0;
					posicionesYDeadlocks->salirDeObjetivos = 0;
					posicionesYDeadlocks->cargarDeNuevoObjetivo=0;

						for(posicionesYDeadlocks->posObjetivo=0;(dictionary_get(pokesDeCadaMapa,mapa)!=NULL) && (posicionesYDeadlocks->salirDeObjetivos!=1);posicionesYDeadlocks->posObjetivo++){

							char* caracterPoke = dictionary_get(pokesDeCadaMapa,mapa);
							//string_append(&protocolo,caracterPoke);


							log_info(logs,"Voy a buscar este pokemon: %s \n", caracterPoke);

							// MANDO: POKENEST + CARACTER
							char carPoke = caracterPoke[0];
							protocAManejar[0]=carPoke;

							send(servidor, protocAManejar, 2, 0);
							//recibo 5 chars, ej: "02;05"
							//coordPokenest= "02;03";

							coordPokenest = (char*)recibirDatos(servidor,5);
							coordPokenest[5] = '\0';

							posPokenest = string_split(coordPokenest,";");

							int x = atoi (posPokenest[0]);
							char* mensaje3 = string_from_format("Coordenada X pokenest: %d\n", x);
							log_info(logs,mensaje3);
							free(mensaje3);

							int y = atoi (posPokenest[1]);
							char* mensaje4 = string_from_format("Coordenada Y pokenest: %d\n", y);
							log_info(logs,mensaje4);
							free(mensaje4);


							moverseEnUnaDireccion(posXInicial, posYInicial, x, y);


							protocAManejar[0]='9';
							send(servidor,protocAManejar,2,0);
							solicitarAtraparPokemon(calculoTiempo,tiempo,pokePiola,mapa);

							// por si se cae
							recv(servidor, (void *)resultado, sizeof(int), 0);
							resultadoEnvio = *((int *)resultado);

							if(resultadoEnvio == 9){
								log_info(logs,"Servidor caído! imposible reconectar. Cerrando...\n");
								exit(0);
							}

							if (posicionesYDeadlocks->cargarDeNuevoObjetivo== 0){
								list_add((ent)->pokemonsPorMapaCapturados,caracterPoke);
								dictionary_remove(pokesDeCadaMapa,mapa);
							}else{
								dictionary_put(pokesDeCadaMapa,mapa,(ent)->pokemonsPorMapaCapturados); // vuelvo a meter todos los pokemons de ese mapa
							}

						} // cierro el for de los objetivos

					if(posicionesYDeadlocks->salirDeObjetivos==0){
						copiarMedalla(mapa);
					}

			} // cierro el for de los mapas

		 } while(list_get(ent->hojaDeViaje,posicionesYDeadlocks->pos)!=NULL);


	terminarAventura(calculoTiempo,tiempo,horaInicio);


	//libero variables del main
	//free(coordPokenest); invalid free
	//free(horaInicio); invalid free
	free(objetivoDeMapa);
	free(objetivosMapa);
	free(posPokenest);
	free(resultado);
	free(protocolo);
	free(protocAManejar);
	free(cosasMapa);
	free(configEntrenador);
	free(nombre);
	free(simbolo);

	//libero listas
	list_destroy_and_destroy_elements(ent->hojaDeViaje,free);
	list_destroy_and_destroy_elements(ent->pokemonsPorMapaCapturados,free);
	list_destroy_and_destroy_elements((ent)->listaNivAtrapados,free);
	list_destroy_and_destroy_elements(ips,free);
	list_destroy_and_destroy_elements(puertos,free);

	//libero dictionarys
	dictionary_destroy_and_destroy_elements(pokesDeCadaMapa,free);


	//libero punteros y struct de entrenador
	free(ent->nombreEntrenador);
	free(ent->caracter);
	free(ent->hojaDeViaje);
	free(ent->pokemonsPorMapaCapturados);
	free(ent->listaNivAtrapados);
	free(ent);

	//libero punteros y struct de pokePiola
	free(pokePiola->especie);
	free(pokePiola->nombreMetadata);
	free(pokePiola);

	//libero struct de tiempo
	free(tiempo);

	//libero struct de calculoTiempo
	free(calculoTiempo);

	//libero struct posicionesYDeadlocks
	free(posicionesYDeadlocks);

	return EXIT_SUCCESS;
}

///////////////////// FUNCIONES DEL ENTRENADOR ///////////////////////////
void handler(int n){
	switch (n) {
		case SIGUSR1:
		reciboUnaVida();
		break;

		case SIGTERM:
		pierdoUnaVida();
		break;
	}
}

void reciboUnaVida(){
	sleep(1);
	ent->cantidadInicialVidas++;
	char* ganeVida = string_from_format("Me dieron una vida, ahora tengo %d vidas \n",ent->cantidadInicialVidas);
	log_info(logs,ganeVida);
	free(ganeVida);
	sleep(1);
}

void pierdoUnaVida(){
	sleep(1);
	morir("senial");
	sleep(1);
}

void* solicitarAtraparPokemon(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,t_pokemonDeserializado* pokePiola, char* mapa){

	char* inicioBloq = temporal_get_string_time();
	char* finBloq;

	pokePiola->protocolo = (int)recibirDatos(servidor,sizeof(int));
	//pokePiola->protocolo=1;

		switch(pokePiola->protocolo){
			case ATRAPA:
				usleep(1000);
				// deserializo el pokemon
				int tamanioEspecie,tamanioNombreMetadata;

				recv(servidor,&tamanioEspecie,sizeof(int),MSG_WAITALL);
				recv(servidor,&tamanioNombreMetadata,sizeof(int),MSG_WAITALL);


				void* bufferEspecie  = malloc(tamanioEspecie);
				void* bufferNombreMetadata  = malloc(tamanioNombreMetadata);
				void* bufferNivel  = malloc(sizeof(int));

				recv(servidor,bufferEspecie,tamanioEspecie, MSG_WAITALL);
				recv(servidor,bufferNombreMetadata,tamanioNombreMetadata, MSG_WAITALL);
				recv(servidor,bufferNivel,sizeof(int), MSG_WAITALL);

				//guardo lo que recibi en mi struct
				pokePiola->especie = (char*)bufferEspecie;
				pokePiola->nombreMetadata = (char*)bufferNombreMetadata;
				pokePiola->nivelPokemon = (int)bufferNivel;

				// para saber si los recibo bien
				log_info(logs,"especie %s",pokePiola->especie);
				log_info(logs,"nombre metadata %s", pokePiola->nombreMetadata);
				log_info(logs,"nivel %d", pokePiola->nivelPokemon);

				list_add((ent)->listaNivAtrapados,pokePiola);
				finBloq = temporal_get_string_time();

				copiarArchivo(mapa,pokePiola->especie,pokePiola->nombreMetadata);
				tiempo = sacarTiempo(calculoTiempo,tiempo,"bloqueado",inicioBloq,finBloq);

				//libero los buffer antes de volver
				free(bufferEspecie);
				free(bufferNombreMetadata);
				free(bufferNivel);
				return tiempo;

			case DEADLOCK:
				posicionesYDeadlocks->cantDeadlocks++;
				int nivelPokeMasFuerte = agarrarPokeConMasNivel((ent)->listaNivAtrapados, pokePiola);

				//serializo mi pokemon
				int tamanioEspecieEnviar = sizeof(char) * strlen(pokePiola->especie);
				//int tamanioNivelEnviar = sizeof(int);

				int tamanioTotal = 3 *sizeof(int) + tamanioEspecieEnviar;

				void* miBuffer = malloc(tamanioTotal);
				// si molesta, la cambio por un int comun que no sea de esa struct y listo
				pokePiola->protocolo = 5;

				//cargo los tamanios
				memcpy(miBuffer, &(pokePiola->protocolo), sizeof(int));
				memcpy(miBuffer + sizeof(int), &tamanioEspecieEnviar, sizeof(int));

				//cargo lo que voy a mandar
				pokePiola->nivelPokemon = nivelPokeMasFuerte;
				memcpy(miBuffer + (3* sizeof(int)), pokePiola->especie, tamanioEspecieEnviar);
				memcpy(miBuffer + (3* sizeof(int)) + tamanioEspecieEnviar, (void*)pokePiola->nivelPokemon, sizeof(int));

				send(servidor,miBuffer,tamanioTotal,0);

				int resolucionDeadlock = (int)recibirDatos(servidor,sizeof(int));
				//int resolucionDeadlock = 7;
				if(resolucionDeadlock==MORI){
					morir("deadlock");
				} else if(resolucionDeadlock==SOBREVIVI){
					log_info(logs,"Sobrevivi al deadlock \n");
				}
				free(miBuffer);
				return posicionesYDeadlocks;
		}

	return NULL;
}


int agarrarPokeConMasNivel(t_list* listaNivAtrapados, t_pokemonDeserializado* pokePiola){

	int ordenarDeMayorAMenor(t_pokemonDeserializado* poke1, t_pokemonDeserializado* poke2){
		return (poke1->nivelPokemon>poke2->nivelPokemon);
	}

	list_sort(listaNivAtrapados,(void*)ordenarDeMayorAMenor);

	int nivelPokeDevuelto = (int)list_get(listaNivAtrapados,0);
	return nivelPokeDevuelto;
}


void terminarAventura(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,char* horaInicio){
	char* horaFin = temporal_get_string_time();
	sacarTiempo(calculoTiempo,tiempo,"aventura",horaInicio,horaFin);

	char* tiempoTotalBloqueado = string_from_format("El tiempo total que paso bloqueado en las pokenests fue: %d:%d:%d:%d", (tiempo)->horasBloqueado,(tiempo)->minutosBloqueado,(tiempo)->segundosBloqueado,(tiempo)->milesimasBloqueado);
	log_info(logs,tiempoTotalBloqueado);
	free(tiempoTotalBloqueado);

	char* deadlock = string_from_format("la cantidad de deadlocks es: %d \n", posicionesYDeadlocks->cantDeadlocks);
	log_info(logs,deadlock);
	free(deadlock);

	return;
}

void* sacarTiempo(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,char* estado,char* horaInicio,char* horaFin){
	//separo hh mm ss mmmm y los guardo en char**, fundamental
		char** inicio = string_n_split(horaInicio,4,":");
		char** fin = string_n_split(horaFin,4,":");

		// convierto a enteros los char*
		calculoTiempo->hInicio = atoi(inicio[0]);
		calculoTiempo->mInicio = atoi(inicio[1]);
		calculoTiempo->sInicio = atoi(inicio[2]);
		calculoTiempo->milInicio = atoi(inicio[3]);

		calculoTiempo->hFin = atoi(fin[0]);
		calculoTiempo->mFin = atoi(fin[1]);
		calculoTiempo->sFin = atoi(fin[2]);
		calculoTiempo->milFin = atoi(fin[3]);

		int horasAventura = calculoTiempo->hFin - calculoTiempo->hInicio;
		int minAventura = calculoTiempo->mFin - calculoTiempo-> mInicio;
		int segAventura = calculoTiempo->sFin - calculoTiempo-> sInicio;
		int milAventura = calculoTiempo->milFin - calculoTiempo-> milInicio;

		if(!strcmp(estado,"bloqueado")){

			(tiempo)->horasBloqueado=(tiempo)->horasBloqueado+horasAventura;
			(tiempo)->minutosBloqueado = (tiempo)->minutosBloqueado + minAventura;
			(tiempo)->segundosBloqueado = (tiempo)->segundosBloqueado + segAventura;
			(tiempo)->milesimasBloqueado = (tiempo)->milesimasBloqueado + milAventura;


			char* mensBloq = string_from_format("El tiempo que paso bloqueado en esta pokenest fue: %d:%d:%d:%d  \n",horasAventura,minAventura,segAventura,milAventura);
			log_info(logs,mensBloq);
			free(mensBloq);
			return tiempo;
		} else if (!strcmp(estado,"aventura")){
			char* aviso = string_from_format("Ahora sos un maestro pokemon, lo lograste a las %s \n", horaFin);
			log_info(logs,aviso);
			free(aviso);

			char* mensTiempo = string_from_format("La aventura duró: %d:%d:%d:%d  \n",horasAventura,minAventura,segAventura,milAventura);
			log_info(logs,mensTiempo);
			free(mensTiempo);
		}

	return NULL;
}



char* empezarAventura(){
	char* horaInicio = temporal_get_string_time();
	char* mensaje = string_from_format("Empezo a las: %s \n", horaInicio);
	log_info(logs, mensaje);
	free(mensaje);
	return horaInicio;
}

void copiarMedalla(char* mapa){

	char* medalla=string_from_format("cp %s/Mapas/%s/medalla-%s.jpg %s/Entrenadores/%s/medallas/medalla-%s.jpg", puntoMontaje, mapa, mapa, puntoMontaje, (ent)->nombreEntrenador,mapa);
	system(medalla);

	char* logueo = string_from_format("Copiada medalla del Mapa %s con exito \n", mapa);
	log_info(logs, logueo);

	free(medalla);
	free(logueo);

return;
}

void copiarArchivo(char* mapa, char* especie, char* especifico){

	char* poke = string_from_format("cp %s/Mapas/%s/PokeNests/%s/%s.dat %s/Entrenadores/%s/Dir\\ de\\ Bill/%s.dat", puntoMontaje, mapa, especie, especifico ,puntoMontaje, (ent)->nombreEntrenador,especifico);
	system(poke);

	char* logueo = string_from_format("Copiada metadata de %s exitosamente \n", especifico);
	log_info(logs, logueo);

	free(poke);
	free(logueo);
	return;
}

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


void moverseEnUnaDireccion(int posXInicial, int posYInicial,int x, int y){
	int cantMovX = x - posXInicial; // cant total mov de x
	int cantMovY = y - posYInicial; // cant total mov de y
	char ultMov = '\0';
	int movDeX = 0; // movimientos que se hicieron de x
	int movDeY = 0; // movimientos que se hicieron de y

	do
	{
		if(cantMovY==0)
			ultMov='y';
		if((cantMovX>0) && (ultMov!='x')){
			protocAManejar[0]='6'; // derecha
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		} else if((cantMovX<0) && (ultMov!='x')){
			protocAManejar[0]='4'; //izquierda
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		}

		if (cantMovX==0) ultMov = 'x';
		if((cantMovY>0) && (ultMov!='y')){
			protocAManejar[0]='2'; // abajo
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovY--;
			movDeY++;
		} else if((cantMovY<0) && (ultMov!='y')){
			protocAManejar[0]='8'; //arriba
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovY--;
			movDeY++;
		}

	} while((cantMovX!=0) || (cantMovY != 0));

	return;
}


void* morir(char* motivo){
	ent->cantidadInicialVidas = ent->cantidadInicialVidas-1;
	if (ent->cantidadInicialVidas>0){
		if(!strcmp(motivo,"deadlock")){
			char* infoDeadlock = string_from_format("Moriste por deadlock, vidas restantes: %d\n", ent->cantidadInicialVidas);
			log_info(logs,infoDeadlock);
			free(infoDeadlock);
			borrarArchivosBill();
			log_info(logs,"Borrados archivos de pokemones del Dir de Bill exitosamente!\n");
			//reconecto al mismo mapa, y reinicio el objetivo siempre desde cero
			log_info(logs,"Reconectandose al mismo mapa que se encontraba \n");
			// empiezo del mapa anterior
			posicionesYDeadlocks->pos = posicionesYDeadlocks->pos -1;
			// no dejo que siga iterando los objetivos
			posicionesYDeadlocks->salirDeObjetivos = 1;
			posicionesYDeadlocks->cargarDeNuevoObjetivo=1;
			//printf("Posicion antes de iterar el for de mapas: %d\n",posicionesYDeadlocks->pos);
			return posicionesYDeadlocks;
			} else if (!strcmp(motivo,"senial")){
				char* infoSenial = string_from_format("Moriste por la senial SIGTERM, vidas restantes: %d\n", ent->cantidadInicialVidas);
				log_info(logs,infoSenial);
				free(infoSenial);
			}
	} else if(ent->cantidadInicialVidas==0){
		if(!strcmp(motivo,"deadlock")){
			log_info(logs,"Perdiste tu ultima vida, fue por deadlock \n");
		} else if (!strcmp(motivo,"senial")){
			log_info(logs,"Perdiste tu ultima vida, fue por SIGTERM \n");
		}
		char respuesta[3];
		char* reintentos = string_from_format("Numero de reintentos realizados hasta el momento: %d\n", ent->reintentos);
		log_info(logs,reintentos);
		free(reintentos);
		do{
			log_info(logs,"Desea reiniciar juego?\n");
			fgets(respuesta, 3, stdin);
			if (string_equals_ignore_case(respuesta,"si")){
				log_info(logs,"Reseteando...\n");
				resetear();
				log_info(logs,"Borradas todas las medallas y todos los pokemons, empezando nueva aventura\n");
				posicionesYDeadlocks->pos = -1;
				posicionesYDeadlocks->salirDeObjetivos = 1;

				list_destroy_and_destroy_elements(ent->hojaDeViaje,free);
				list_destroy_and_destroy_elements(ent->pokemonsPorMapaCapturados,free);
				list_destroy_and_destroy_elements((ent)->listaNivAtrapados,free);
				list_destroy_and_destroy_elements(ips,free);
				list_destroy_and_destroy_elements(puertos,free);

				dictionary_destroy_and_destroy_elements(pokesDeCadaMapa,free);

				if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
					log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
					return NULL;
				}
				posicionesYDeadlocks->reintentosActualizados++;
				ent->reintentos = posicionesYDeadlocks->reintentosActualizados;
				char* nroReintentos = string_from_format("Numero de reintentos realizados: %d",ent->reintentos);
				log_info(logs,nroReintentos);
				free(nroReintentos);
				//aviso al mapa que reinicio desde cero
				protocAManejar[0]='1';
				send(servidor, protocAManejar, 2, 0);
				return posicionesYDeadlocks;
			}else if(string_equals_ignore_case(respuesta,"no")) {
					log_info(logs,"Cerrando programa\n");
					exit(0);
			}else{
					log_info(logs,"Debe ingresar si o no sin importar mayusculas para realizar algo\n");
			}
		}while( !(string_equals_ignore_case(respuesta,"si")) || !(string_equals_ignore_case(respuesta,"no")));
	}
	return NULL;
}


void borrarArchivosBill(){ // Al momento de testear va a decir que no encontro nada si la carpeta estaba vacia
		char* borrarBill = string_from_format("rm -r %s/Entrenadores/%s/Dir\\ de\\ Bill/*",puntoMontaje,ent->nombreEntrenador);
		system(borrarBill);
		free(borrarBill);
}

void borrarMedallas(){
		char* borrarMedallas = string_from_format("rm -r %s/Entrenadores/%s/medallas/*",puntoMontaje,ent->nombreEntrenador);
		system(borrarMedallas);
		free(borrarMedallas);
}


void resetear(){
	borrarMedallas();
	borrarArchivosBill();
}

