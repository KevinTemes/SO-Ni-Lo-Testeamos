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

	 t_tiempoBloqueado* tiempo;
	 tiempo= malloc(sizeof(t_tiempoBloqueado));

	 t_calculoTiempo* calculoTiempo;
	 calculoTiempo = malloc(sizeof(t_calculoTiempo));


	 posicionesYDeadlocks = malloc(sizeof(t_posMapaposObjetivoYDeadlocks));

	 t_actualizarPos* posActual;
	 posActual = malloc(sizeof(t_actualizarPos));


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
	 posicionesYDeadlocks->cantMuertes = 0;
	 posicionesYDeadlocks->reintentosActualizados=0;
	 posicionesYDeadlocks->cantDeadlocks=0;
	 tiempo->horasBloqueado = 0;
	 tiempo->minutosBloqueado = 0;
	 tiempo->segundosBloqueado = 0;

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

				 	//log_info(logs,"Entre con la pos %d",posicionesYDeadlocks->pos);

				 	miIP= list_get(ips,posicionesYDeadlocks->pos);
					miPuerto = list_get(puertos,posicionesYDeadlocks->pos);

					//printf("ip %s \n", miIP);
					//printf("puerto %s \n",miPuerto);


					servidor = conectarCliente(miIP, miPuerto);

					//int resultadoEnvio = 0;

					char* mapa = list_get((ent)->hojaDeViaje,posicionesYDeadlocks->pos);

					log_info(logs,"Conectado al Mapa %s. Ingrese el mensaje que desee enviar, o cerrar para salir\n",mapa);

					//////////////// recibo y mando datos al Mapa /////////////////////

					// cuando pase a otro mapa, o lo reinicia, vuelve a arrancar en (0;0)
					posActual->posXInicial=1;
					posActual->posYInicial =1;
					posicionesYDeadlocks->salirDeObjetivos = 0;
					posicionesYDeadlocks->cargarDeNuevoObjetivo=0;

						while((dictionary_get(pokesDeCadaMapa,mapa)!=NULL) && (posicionesYDeadlocks->salirDeObjetivos!=1)){

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

							log_info(logs, "Recibi esta coordenada entera %s",coordPokenest);

							posPokenest = string_split(coordPokenest,";");

							int x = atoi (posPokenest[0]);
							log_info(logs,"Coordenada X pokenest: %d", x);

							int y = atoi (posPokenest[1]);
							log_info(logs,"Coordenada Y pokenest: %d\n", y);

							moverseEnUnaDireccion(posActual, x, y);
							free(coordPokenest);
							free(posPokenest);

							protocAManejar[0]='9';
							send(servidor,protocAManejar,2,0);

							solicitarAtraparPokemon(calculoTiempo,tiempo,mapa);

							if (posicionesYDeadlocks->cargarDeNuevoObjetivo== 0){
								list_add((ent)->pokemonsPorMapaCapturados,caracterPoke);
								log_info(logs,"Capture en el mapa %s a %s y lo agregue a la lista por mapa capturados\n", mapa,caracterPoke);
								dictionary_remove(pokesDeCadaMapa,mapa);
							}else{ // solo si resetea y tiene vidas lo hace
								int i;
								while(dictionary_get(pokesDeCadaMapa,mapa)!=NULL){
									char* losQueQuedaron = dictionary_get(pokesDeCadaMapa,mapa);
									list_add(ent->pokemonsPorMapaCapturados, losQueQuedaron);
									log_info(logs,"Agrego al final de la lista por mapa capturados al poke %s que estaba en el dictionary de objetivos no realizados todavia",losQueQuedaron);
									dictionary_remove(pokesDeCadaMapa,mapa);
								}

								for(i=0;i<list_size(ent->pokemonsPorMapaCapturados);i++){
									char* pokeAMeter = list_get(ent->pokemonsPorMapaCapturados,i);
									log_info(logs,"Agrego este pokemon al diccionario %s para reiniciar el mapa que se encontraba",pokeAMeter);
									log_info(logs, "Lo agregue en la posicion %d del diccionario\n",i);
									dictionary_put(pokesDeCadaMapa,mapa,pokeAMeter); // vuelvo a meter todos los pokemons de ese mapa
								}
								list_clean(ent->pokemonsPorMapaCapturados);

							}

						} // cierro el while de los objetivos

						if(posicionesYDeadlocks->salirDeObjetivos==0){
							copiarMedalla(mapa);
							list_clean(ent->pokemonsPorMapaCapturados);
							close(servidor);
						}

			} // cierro el for de los mapas

		 } while(list_get(ent->hojaDeViaje,posicionesYDeadlocks->pos)!=NULL && ent->cantidadInicialVidas !=0);


	terminarAventura(calculoTiempo,tiempo,horaInicio);


	//libero variables del main
	//free(coordPokenest); invalid free
	//free(posPokenest);
	//free(objetivoDeMapa);
	//free(objetivosMapa);
	//free(cosasMapa);

	//free(horaInicio);
	free(resultado);
	free(protocolo);
	free(protocAManejar);
	free(configEntrenador);

	//libero listas
	list_destroy_and_destroy_elements(ent->hojaDeViaje,free);
	list_destroy_and_destroy_elements(ent->pokemonsPorMapaCapturados,free);
	list_destroy((ent)->listaNivAtrapados);
	list_destroy_and_destroy_elements(ips,free);
	list_destroy_and_destroy_elements(puertos,free);

	//libero dictionarys
	dictionary_destroy_and_destroy_elements(pokesDeCadaMapa,free);

	//libero punteros y struct de entrenador, las listas ya fueron destruidas, no se liberan
	free(ent->nombreEntrenador);
	free(ent->caracter);
	free(ent);

	//libero punteros y struct de pokePiola
	//free(pokePiola->especie);
	//free(pokePiola->nombreMetadata);

	//libero structs de int
	free(calculoTiempo);
	free(posActual);
	free(posicionesYDeadlocks);
	free(tiempo);

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
	log_info(logs,"Me dieron una vida, ahora tengo %d vidas \n",ent->cantidadInicialVidas);
	sleep(1);
}

void pierdoUnaVida(){
	sleep(1);
	morir("senial");
	sleep(1);
}

void* deserializoPokemon(t_calculoTiempo* calculoTiempo, t_tiempoBloqueado* tiempo, char* mapa, char* inicioBloq, char* finBloq){
	//reservo memoria para mi struct cada vez que entra, sino lo pisaria
	t_pokemonDeserializado* pokePiola;
	pokePiola =malloc(sizeof(t_pokemonDeserializado));

	// deserializo el pokemon, recibo 7 10 Pikachu Pikachu001 33
	int tamanioEspecie,tamanioNombreMetadata;

	recv(servidor,&tamanioEspecie,sizeof(int),MSG_WAITALL);
	recv(servidor,&tamanioNombreMetadata,sizeof(int),MSG_WAITALL);

	void* bufferEspecie  = malloc(tamanioEspecie+1);
	void* bufferNombreMetadata  = malloc(tamanioNombreMetadata+1);

	recv(servidor,bufferEspecie,tamanioEspecie, MSG_WAITALL);
	recv(servidor,bufferNombreMetadata,tamanioNombreMetadata, MSG_WAITALL);

	pokePiola->especie = (char*)bufferEspecie;
	pokePiola->especie[tamanioEspecie] = '\0';

	pokePiola->nombreMetadata = (char*)bufferNombreMetadata;
	pokePiola->nombreMetadata[tamanioNombreMetadata] = '\0';

	recv(servidor, &pokePiola->nivelPokemon,sizeof(int),MSG_WAITALL);

	log_info(logs, "Especie del atrapado: %s",pokePiola->especie);
	log_info(logs, "Especifico del atrapado: %s", pokePiola->nombreMetadata);
	log_info(logs,"Agregue el nivel %d del atrapado \n", pokePiola->nivelPokemon);

	list_add((ent)->listaNivAtrapados,pokePiola);
	int posicion = list_size(ent->listaNivAtrapados);
	t_pokemonDeserializado* meteEnLaLista = list_get((ent->listaNivAtrapados),posicion-1);

	log_info(logs, "Especie que atrape agregada a la lista: %s, importante si llega a haber deadlock",meteEnLaLista->especie);
	log_info(logs, "Especifico que atrape agregado a la lista: %s, importante si llega a haber deadlock", meteEnLaLista->nombreMetadata);
	log_info(logs,"Agregue el nivel que atrape a la lista: %d, importante si llega a haber deadlock \n",meteEnLaLista->nivelPokemon);

	finBloq = temporal_get_string_time();

	copiarArchivo(mapa,pokePiola->especie,pokePiola->nombreMetadata);
	tiempo = sacarTiempo(calculoTiempo,tiempo,"bloqueado",inicioBloq,finBloq);

	//free(bufferEspecie);
	//free(bufferNombreMetadata);
	free(finBloq);
	free(inicioBloq);
	return tiempo;
}


void* solicitarAtraparPokemon(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo, char* mapa){

	char* inicioBloq = temporal_get_string_time();
	char* finBloq = "\0";

	int protocoloRec;
	recv(servidor,&(protocoloRec),sizeof(int),MSG_WAITALL);

	log_info(logs,"Recibo un 1 porque lo voy a atrapar o un 3 porque hay deadlock, y el protocolo recibido es %d",protocoloRec);

		switch(protocoloRec){
			case ATRAPA:
				usleep(1000);
				deserializoPokemon(calculoTiempo,tiempo,mapa,inicioBloq,finBloq);
				return tiempo;

			case DEADLOCK:
				posicionesYDeadlocks->cantDeadlocks++;

				t_pokemonDeserializado* elMasFuerte = (t_pokemonDeserializado*)agarrarPokeConMasNivel((ent)->listaNivAtrapados);

				//serializo mi pokemon, mando 5 7 Pikachu 33
				char protocolo = '5';
				int tamanioEspecieEnviar = strlen(elMasFuerte->especie);
				int tamanioTotal = sizeof(char) + 2 *sizeof(int) + tamanioEspecieEnviar;

				void* miBuffer = malloc(tamanioTotal);

				//cargo los tamanios
				memcpy(miBuffer, &protocolo, sizeof(char));
				memcpy(miBuffer + sizeof(char), &tamanioEspecieEnviar, sizeof(int));

				//cargo lo que voy a mandar
				memcpy(miBuffer + sizeof(char) +  sizeof(int), elMasFuerte->especie, tamanioEspecieEnviar);
				memcpy(miBuffer + sizeof(char) +  sizeof(int) + tamanioEspecieEnviar, &(elMasFuerte->nivelPokemon), sizeof(int));

				log_info(logs,"Protocolo que mando %c, va a haber pelea por deadlock",protocolo);
				log_info(logs,"tamanio de especie para pelear: %d",tamanioEspecieEnviar);
				log_info(logs,"especie del mas fuerte para pelear %s",elMasFuerte->especie);
				log_info(logs, "nivel del mas fuerte para pelear %d \n",elMasFuerte->nivelPokemon);

				send(servidor,miBuffer,tamanioTotal,0);

				int protocMoriOSobrevivi;

				recv(servidor,&(protocMoriOSobrevivi),sizeof(int),MSG_WAITALL);
				log_info(logs,"Recibo 7 si mori o 0 si sobrevivi, y el protocolo recibido es: %d",protocMoriOSobrevivi);

				if(protocMoriOSobrevivi==MORI){
					morir("deadlock");
					// revisar este free
					free(miBuffer);

				} else if(protocMoriOSobrevivi==SOBREVIVI){

					log_info(logs,"Sobrevivi al deadlock\n");

					int protocAtrapo;
					recv(servidor,&(protocAtrapo),sizeof(int),MSG_WAITALL);
					log_info(logs,"Recibi este protocolo %d, voy a agarrar y deserializar el pokemon que queria",protocAtrapo);

					deserializoPokemon(calculoTiempo,tiempo,mapa,inicioBloq,finBloq);

					return tiempo;
				}

		}

	return NULL;
}


void* agarrarPokeConMasNivel(t_list* listaNivAtrapados){

	int ordenarDeMayorAMenor(t_pokemonDeserializado* poke1, t_pokemonDeserializado* poke2){
		return (poke1->nivelPokemon>poke2->nivelPokemon);
	}

	list_sort(listaNivAtrapados,(void*)ordenarDeMayorAMenor);

	t_pokemonDeserializado* elMasFuerte	= list_get(listaNivAtrapados,0);
	log_info(logs,"especie mas fuerte %s",elMasFuerte->especie);

	if(list_size(listaNivAtrapados)==0){
		log_info(logs,"No hay nada en la lista, chau!");
		return NULL;
	}else{
		log_info(logs, "El mas fuerte es %s y el nivel que tiene es %d \n", elMasFuerte->nombreMetadata, elMasFuerte->nivelPokemon);
	}

	return elMasFuerte;
}


void terminarAventura(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,char* horaInicio){
	char* horaFin = temporal_get_string_time();
	sacarTiempo(calculoTiempo,tiempo,"aventura",horaInicio,horaFin);
	free(horaFin);

	log_info(logs,"El tiempo total que paso bloqueado en las pokenests fue: %d horas, %d minutos, %d segundos\n", (tiempo)->horasBloqueado,(tiempo)->minutosBloqueado,(tiempo)->segundosBloqueado);
	log_info(logs,"La cantidad de deadlocks es: %d \n", posicionesYDeadlocks->cantDeadlocks);
	log_info(logs,"Cantidad de muertes: %d\n",posicionesYDeadlocks->cantMuertes);

	return;
}

void* sacarTiempo(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,char* estado,char* horaInicio,char* horaFin){
	//separo hh mm ss mmmm y los guardo en char**, fundamental
		char** inicio = string_n_split(horaInicio,4,":");
		char** fin = string_n_split(horaFin,4,":");

		// convierto a enteros los char*
		calculoTiempo->hInicio = atoi(inicio[0])*3600;
		calculoTiempo->mInicio = atoi(inicio[1])*60;
		calculoTiempo->sInicio = atoi(inicio[2]);
		calculoTiempo->milInicio = atoi(inicio[3])/1000;

		calculoTiempo->hFin = atoi(fin[0])*3600;
		calculoTiempo->mFin = atoi(fin[1])*60;
		calculoTiempo->sFin = atoi(fin[2]);
		calculoTiempo->milFin = atoi(fin[3])/1000;

		int totalSegInicio = calculoTiempo->hInicio + calculoTiempo ->mInicio + calculoTiempo->milInicio + calculoTiempo->sInicio;
		int totalSegFin = calculoTiempo->hFin + calculoTiempo ->mFin + calculoTiempo->milFin + calculoTiempo->sFin;

		int tiempoQueTardo = totalSegFin - totalSegInicio;

		div_t divHorasCocienteResto;
		divHorasCocienteResto = div(tiempoQueTardo,3600);
		int horasAventura = divHorasCocienteResto.quot;

		div_t divMinutosCocienteResto;
		divMinutosCocienteResto = div(divHorasCocienteResto.rem,60);
		int minAventura = divMinutosCocienteResto.quot;

		int segAventura = divMinutosCocienteResto.rem /1;

		if(!strcmp(estado,"bloqueado")){

			(tiempo)->horasBloqueado=(tiempo)->horasBloqueado+horasAventura;
			(tiempo)->minutosBloqueado = (tiempo)->minutosBloqueado + minAventura;
			(tiempo)->segundosBloqueado = (tiempo)->segundosBloqueado + segAventura;

			log_info(logs,"El tiempo que paso bloqueado en esta pokenest fue %d horas, %d minutos, %d segundos \n",horasAventura,minAventura,segAventura);

			return tiempo;
		} else if (!strcmp(estado,"aventura")){
			log_info(logs,"Ahora sos un maestro pokemon, lo lograste a las %s \n", horaFin);
			log_info(logs,"La aventura duró %d horas, %d minutos, %d segundos  \n",horasAventura,minAventura,segAventura);
		}

		string_iterate_lines(inicio, (void*) free);
		string_iterate_lines(fin, (void*)free);

		free(inicio);
		free(fin);

	return NULL;
}


char* empezarAventura(){
	char* horaInicio = temporal_get_string_time();
	log_info(logs,"Empezo a las: %s \n", horaInicio);
	return horaInicio;
}

void copiarMedalla(char* mapa){

	char* medalla=string_from_format("cp %s/Mapas/%s/medalla-%s.jpg %s/Entrenadores/%s/medallas/medalla-%s.jpg", puntoMontaje, mapa, mapa, puntoMontaje, (ent)->nombreEntrenador,mapa);
	system(medalla);

	log_info(logs,"Copiada medalla del Mapa %s con exito \n", mapa);

	free(medalla);
	return;
}

void copiarArchivo(char* mapa, char* especie, char* especifico){

	char* poke = string_from_format("cp %s/Mapas/%s/PokeNests/%s/%s.dat %s/Entrenadores/%s/Dir\\ de\\ Bill/%s.dat", puntoMontaje, mapa, especie, especifico ,puntoMontaje, (ent)->nombreEntrenador,especifico);
	system(poke);

	log_info(logs,"Copiada metadata de %s exitosamente \n", especifico);

	free(poke);
	return;
}

void* recibirDatos(int conexion, int tamanio){
	void* mensaje=(void*)malloc(tamanio+1);
	int bytesRecibidos = recv(conexion, mensaje, tamanio, MSG_WAITALL);
	if (bytesRecibidos != tamanio) {
		perror("Error al recibir el mensaje\n");
		free(mensaje);
		char* adios=string_new();
		string_append(&adios,"0\0");
		return adios;}
	return mensaje;
}


void* moverseEnUnaDireccion(t_actualizarPos* posActual,int x, int y){
	int cantMovFaltantesX = x - posActual->posXInicial;
	int cantMovFaltantesY = y - posActual->posYInicial;

	//actualizar la pos x inicial, e y inicial

	char ultMov = '\0';
	int movDeX = 0; // movimientos que se hicieron de x
	int movDeY = 0; // movimientos que se hicieron de y

	do
	{
		if(cantMovFaltantesY==0)
			ultMov='y';
		if((cantMovFaltantesX>0) && (ultMov!='x')){
			protocAManejar[0]='6'; // derecha
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovFaltantesX--;
			movDeX++;
		} else if((cantMovFaltantesX<0) && (ultMov!='x')){
			protocAManejar[0]='4'; //izquierda
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovFaltantesX++;
			movDeX++;
		}

		if (cantMovFaltantesX==0) ultMov = 'x';
		if((cantMovFaltantesY>0) && (ultMov!='y')){
			protocAManejar[0]='2'; // abajo
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovFaltantesY--;
			movDeY++;
		} else if((cantMovFaltantesY<0) && (ultMov!='y')){
			protocAManejar[0]='8'; //arriba
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovFaltantesY++;
			movDeY++;
		}

	} while((cantMovFaltantesX != 0) || (cantMovFaltantesY != 0));

	posActual->posXInicial = x;
	posActual->posYInicial = y;

	return posActual;
}


void resetearDeCero (){

	char respuesta[3];
	log_info(logs,"Numero de reintentos realizados hasta el momento: %d", ent->reintentos);
	do{
		log_info(logs,"Desea reiniciar juego?");
		fgets(respuesta, 3, stdin);
		if (string_equals_ignore_case(respuesta,"si")){
			log_info(logs,"Reseteando...\n");
			resetear();
			log_info(logs,"Borradas todas las medallas y todos los pokemons, empezando nueva aventura");

			list_destroy_and_destroy_elements(ent->hojaDeViaje,free);
			list_destroy_and_destroy_elements(ent->pokemonsPorMapaCapturados,free);
			list_destroy((ent)->listaNivAtrapados);
			list_destroy_and_destroy_elements(ips,free);
			list_destroy_and_destroy_elements(puertos,free);

			dictionary_destroy_and_destroy_elements(pokesDeCadaMapa,free);

			if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
				log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador");
			}

			log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");
			posicionesYDeadlocks->reintentosActualizados++;
			ent->reintentos = posicionesYDeadlocks->reintentosActualizados;
			log_info(logs,"Numero de reintentos realizados: %d \n",ent->reintentos);

			posicionesYDeadlocks->pos = -1;
			posicionesYDeadlocks->salirDeObjetivos = 1;

			}else if(string_equals_ignore_case(respuesta,"no")) {
				log_info(logs,"Cerrando programa\n");
				exit(0);
			}else{
				log_info(logs,"Debe ingresar si o no sin importar mayusculas para realizar algo\n");
			}

	}while( !(string_equals_ignore_case(respuesta,"si")) || !(string_equals_ignore_case(respuesta,"no")));

}


void morir(char* motivo){
	ent->cantidadInicialVidas = ent->cantidadInicialVidas-1;
	posicionesYDeadlocks->cantMuertes = posicionesYDeadlocks->cantMuertes+1;
	if (ent->cantidadInicialVidas>0){
		if(!strcmp(motivo,"deadlock")){
			log_info(logs,"Moriste por deadlock, vidas restantes: %d", ent->cantidadInicialVidas);
			borrarArchivosBill();
			log_info(logs,"Borrados archivos de pokemones del Dir de Bill exitosamente!");
			//reconecto al mismo mapa, y reinicio el objetivo siempre desde cero
			log_info(logs,"Reconectandose al mismo mapa que se encontraba\n");
			// empiezo del mapa anterior
			posicionesYDeadlocks->pos = posicionesYDeadlocks->pos -1;
			// no dejo que siga iterando los objetivos
			posicionesYDeadlocks->salirDeObjetivos = 1;
			posicionesYDeadlocks->cargarDeNuevoObjetivo=1;
			//printf("Posicion antes de iterar el for de mapas: %d\n",posicionesYDeadlocks->pos);
			close(servidor);
			} else if (!strcmp(motivo,"senial")){
				log_info(logs,"Moriste por la senial SIGTERM, vidas restantes: %d\n", ent->cantidadInicialVidas);
			}
	} else if(ent->cantidadInicialVidas==0){
		close(servidor);
		if(!strcmp(motivo,"deadlock")){
			log_info(logs,"Perdiste tu ultima vida, fue por deadlock");
			resetearDeCero();
		} else if (!strcmp(motivo,"senial")){
			log_info(logs,"Perdiste tu ultima vida, fue por SIGTERM");
			resetearDeCero();
		}

	}
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

