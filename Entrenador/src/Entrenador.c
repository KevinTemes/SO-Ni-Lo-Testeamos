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


t_log* logs;
char* puntoMontaje;
t_entrenador* ent;
t_posMapaposObjetivoYDeadlocks* posicionesYDeadlocks;
char* protocAManejar;

#define ATRAPA 1
#define DEADLOCK 3
#define MORI 7
#define SOBREVIVI 0
#define PACKAGESIZE 10

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
	 char* configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",puntoMontaje,nombreEnt);

	 //para cuando debuggeamos, descomentar lo de abajo y comentar lo de arriba
	 //char* configEntrenador = "/home/utnso/workspace/pokedex/Entrenadores/Red/metadata";

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
	 char* protocolo = string_new();
	 char* numConcatenado="1";
	 string_append(&protocolo,(ent)->caracter);
	 string_append(&protocolo,numConcatenado);
	 protocAManejar = strdup(protocolo);
	 char* coordPokenest;
	 char** posPokenest;
	 char* horaInicio;
	 char *resultado = malloc(sizeof(int));
	 char* miIP;
	 char* miPuerto;
	 t_list* listaNivAtrapados= list_create();
	 t_dictionary* masFuertePokeYNivel = dictionary_create();
	 posicionesYDeadlocks->pos=0;
	 posicionesYDeadlocks->posObjetivo=0;
	 posicionesYDeadlocks->cantDeadlocks=0;

	 //reiniciarHojaDeViaje:
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
					printf("Conectado al Mapa %s. Ingrese el mensaje que desee enviar, o cerrar para salir\n",mapa);


					//////////////// recibo y mando datos al Mapa /////////////////////

					// cuando pase a otro mapa, o lo reinicia, vuelve a arrancar en (0;0)
					int posXInicial =0;
					int posYInicial =0;
					posicionesYDeadlocks->valor = 1;

						for(posicionesYDeadlocks->posObjetivo=0;(dictionary_get(pokesDeCadaMapa,mapa)!=NULL) && (posicionesYDeadlocks->valor!=0);posicionesYDeadlocks->posObjetivo++){
							printf("Posicion del objetivo inicial: %d\n",posicionesYDeadlocks->posObjetivo);
							// MANDO: CARACTER + POKENEST

							char* caracterPoke = dictionary_get(pokesDeCadaMapa,mapa);
							string_append(&protocolo,caracterPoke);


							printf("Voy a buscar este pokemon: %s \n", caracterPoke);

							char carPoke = caracterPoke[0];
							protocAManejar[1]=carPoke;

							send(servidor, protocAManejar, 2, 0);
							//recibo 5 chars, ej: "34;12"
							//coordPokenest = (char*)recibirDatos(servidor,5);

							coordPokenest= "02;03";

							posPokenest = string_split(coordPokenest,";");
							int x = atoi (posPokenest[0]);
							printf("Coordenada X pokenest: %d\n", x);
							int y = atoi (posPokenest[1]);
							printf("Coordenada Y pokenest: %d\n", y);


							moverseEnUnaDireccion(posXInicial, posYInicial, x, y);


							protocAManejar[1]='9'; // Solicitud Atrapar Pokemon
							send(servidor,protocAManejar,2,0);
							solicitarAtraparPokemon(calculoTiempo,tiempo,masFuertePokeYNivel,pokePiola, listaNivAtrapados,mapa);

							// por si se cae
							recv(servidor, (void *)resultado, sizeof(int), 0);
							resultadoEnvio = *((int *)resultado);

							if(resultadoEnvio == 9){
								printf("Servidor caído! imposible reconectar. Cerrando...\n");
								exit(0);
							}

							dictionary_remove(pokesDeCadaMapa,mapa);
						} // cierro el for de los objetivos

					if(!close(servidor)){
						copiarMedalla(mapa);
						close(servidor);
					}
					close(servidor);
			} // cierro el for de los mapas

		 } while((ent->cantidadInicialVidas!=0) && (list_get(ent->hojaDeViaje,posicionesYDeadlocks->pos)!=NULL));


	terminarAventura(calculoTiempo,tiempo,horaInicio);

	list_destroy_and_destroy_elements(ips,free);
	list_destroy_and_destroy_elements(puertos,free);
	//list_destroy_and_destroy_elements(tiempoBloqCadaPokenest,free); invalid free
	dictionary_destroy_and_destroy_elements(masFuertePokeYNivel,free);

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
	free(ent->objetivosPorMapa);
	free(ent->hojaDeViaje);
	free(ent);
	free(pokePiola);
	free(tiempo);
	free(calculoTiempo);
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

void* solicitarAtraparPokemon(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo, t_dictionary* masFuertePokeYNivel,t_pokemonDeserializado* pokePiola,t_list* listaNivAtrapados, char* mapa){

	char* inicioBloq = temporal_get_string_time();
	char* finBloq;

	//char* operacion = recibirDatos(servidor,21);
	/* podria recibir serializado:
	nroProtocolo especie nombreMetadata nivel sin los ; porque usamos un offset*/

	char* operacion =  "3;Pikachu/Pikachu001;33";
	char** cosasRecibidas = string_n_split(operacion,3,";");

	//el nro del protocolo
	char* nroRecString = cosasRecibidas[0];
	int nroRec = atoi (nroRecString);
	pokePiola->protocolo=nroRec;

	// la ruta de mi poke atrapado
	char* rutaPokeAtrapado = cosasRecibidas[1];
	pokePiola->nombreMetadata = rutaPokeAtrapado;

	//divido la ruta para usarla para copiarArchivo y para mandarla a Mapa
	char** especieYEspecifico = string_split(pokePiola->nombreMetadata,"/");
	//agarro Pikachu001
	char* especifico = especieYEspecifico[1];

	//nivel poke atrapado, hacer el atoi y castearlo a (int*)
	char* nivelPoke = cosasRecibidas[2];
	int nivelInt = atoi(nivelPoke);
	pokePiola->nivelPokemon=nivelInt;

		switch(pokePiola->protocolo){
			case ATRAPA:
				//simulo un tiempo de bloqueado de pokenest por el momento
				usleep(1000);
				list_add(listaNivAtrapados,pokePiola->nivelPokemon);
				finBloq = temporal_get_string_time();
				copiarArchivo(especifico,mapa,pokePiola->nombreMetadata);
				tiempo = sacarTiempo(calculoTiempo,tiempo,"bloqueado",inicioBloq,finBloq);
				//revisar si lo puedo manejar casteandolo o con el char* y fue
				dictionary_put(masFuertePokeYNivel,nivelPoke,especifico);
				return tiempo;

			case DEADLOCK:
				posicionesYDeadlocks->cantDeadlocks++;
				//int nivelPokeAtrapado = agarrarPokeConMasNivel(listaNivAtrapados, pokePiola);
				/*char* nivelString = string_itoa(nivelPokeAtrapado);
				char* especificoMasFuerte = dictionary_get(masFuertePokeYNivel,nivelString);
				char* pokeMFuerte = string_from_format("%s5;%s;%d",ent->caracter,especificoMasFuerte,nivelPokeAtrapado);
				//revisar ese sizeof
				send(servidor,pokeMFuerte,sizeof(pokeMFuerte),0);
				free(pokeMFuerte);

				char* resolucionDeadlock = recibirDatos(servidor,1);*/
				char* resolucionDeadlock = "7";
				int respDeadlock = atoi(resolucionDeadlock);
				if(respDeadlock==MORI){
					morir("deadlock");
				} else if(respDeadlock==SOBREVIVI){
					log_info(logs,"Sobrevivi al deadlock \n");
				}
				return posicionesYDeadlocks;
		}

	free(nivelPoke);
	free(nroRecString);
	return NULL;
}

int agarrarPokeConMasNivel(t_list* listaNivAtrapados, t_pokemonDeserializado* pokePiola){

	int ordenarDeMayorAMenor(t_pokemonDeserializado* niv1, t_pokemonDeserializado* niv2){
		return (niv1->nivelPokemon>niv2->nivelPokemon);
	}

	list_sort(listaNivAtrapados,(void*)ordenarDeMayorAMenor);

	int nivelPokeDevuelto = list_get(listaNivAtrapados,0);
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

void copiarArchivo(char* especifico,char* mapa,char* rutaPokeAtrapado){

	char* poke = string_from_format("cp %s/Mapas/%s/PokeNests/%s.dat %s/Entrenadores/%s/Dir\\ de\\ Bill/%s.dat", puntoMontaje, mapa, rutaPokeAtrapado, puntoMontaje, (ent)->nombreEntrenador,especifico);
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
	char ultMov;
	int movDeX = 0; // movimientos que se hicieron de x
	int movDeY = 0; // movimientos que se hicieron de y

	do
	{
		if(cantMovY==0)
			ultMov='y';
		if((cantMovX>0) && (ultMov!='x')){
			protocAManejar[1]='6'; // derecha
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		} else if((cantMovX<0) && (ultMov!='x')){
			protocAManejar[1]='4'; //izquierda
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'x';
			cantMovX--;
			movDeX++;
		}

		if (cantMovX==0) ultMov = 'x';
		if((cantMovY>0) && (ultMov!='y')){
			protocAManejar[1]='2'; // abajo
			send(servidor, protocAManejar, 2, 0);
			ultMov = 'y';
			cantMovY--;
			movDeY++;
		} else if((cantMovY<0) && (ultMov!='y')){
			protocAManejar[1]='8'; //arriba
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
			posicionesYDeadlocks->valor = 0;
			printf("Posicion antes de iterar el for de mapas: %d\n",posicionesYDeadlocks->pos);
			close(servidor);
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
				posicionesYDeadlocks->valor = 0;
				ent->reintentos = ent->reintentos+1;
				char* nroReintentos = string_from_format("Numero de reintentos realizados: %d",ent->reintentos);
				log_info(logs,nroReintentos);
				free(nroReintentos);
				//aviso al mapa que reinicio desde cero
				protocAManejar[1]='1';
				send(servidor, protocAManejar, 2, 0); // para que sepa que reseteo desde 0 toda mi hoja
				close(servidor);
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

