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

#define ATRAPA 1
#define DEADLOCK 3
#define MORI 7
#define PACKAGESIZE 10

int main(int argc, char* argv[]){ // PARA EJECUTAR: ./Entrenador Ash /home/utnso/workspace/pokedex
	char* nombreEnt = argv[1];
	char* puntoMontaje = argv[2];

	// LOGS
	 remove("Entrenador.log");
	 puts("Creando archivo de logueo...\n");
	 logs = log_create("Entrenador.log", "Entrenador", true, log_level_from_string("INFO"));
	 puts("Log Entrenador creado exitosamente \n");

	 //STRUCTS
	 t_entrenador* ent;
	 ent = malloc(sizeof(t_entrenador));

	 //si rompe, meterlo adentro del for de cada pokemon
	 t_pokemonDeserializado* pokePiola;
	 pokePiola =malloc(sizeof(t_pokemonDeserializado));

	 t_tiempoBloqueado* tiempo;
	 tiempo= malloc(sizeof(t_tiempoBloqueado));

	 t_calculoTiempo* calculoTiempo;
	 calculoTiempo = malloc(sizeof(t_calculoTiempo));

	 //CONFIG
	 char* configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",puntoMontaje,nombreEnt);

	 //para cuando debuggeamos, descomentar lo de abajo y comentar lo de arriba
	 //char* configEntrenador = "/home/utnso/workspace/pokedex/Entrenadores/Red/metadata";

	 if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");

	 usoDeSeniales(ent,puntoMontaje);

	 //VARIABLES USADAS Y CONEXION
	 int pos;
	 int cantMapas = list_size((ent)->hojaDeViaje);
	 int posObjetivo;
	 char* protocolo = string_new();
	 char* numConcatenado="1";
	 string_append(&protocolo,(ent)->caracter);
	 string_append(&protocolo,numConcatenado);
	 char* protocAManejar = strdup(protocolo);
	 char* coordPokenest;
	 char** posPokenest;
	 char* horaInicio;
	 char *resultado = malloc(sizeof(int));
	 char* miIP;
	 char* miPuerto;
	 t_list* listaNivAtrapados= list_create();
	 int cantDeadlocks = 0;
	 t_dictionary* masFuertePokeYNivel = dictionary_create();


	 horaInicio = empezarAventura();

	 do{
		 for(pos = 0;pos<cantMapas;pos++){
		// reconectar el mismo mapa entra aca directo asi no le suma una posicion
		//reconectarMapa:

			miIP= list_get(ips,pos);
			miPuerto = list_get(puertos,pos);

			//printf("ip %s \n", miIP);
			//printf("puerto %s \n",miPuerto);


			servidor = conectarCliente(miIP, miPuerto);

			int resultadoEnvio = 0;

			char* mapa = list_get((ent)->hojaDeViaje,pos);
			printf("Conectado al Mapa %s. Ingrese el mensaje que desee enviar, o cerrar para salir\n",mapa);


			//////////////// recibo y mando datos al Mapa /////////////////////

			// cuando pase a otro mapa, vuelve a arrancar en (0;0)
			int posXInicial =0;
			int posYInicial =0;


			for(posObjetivo=0;dictionary_get(pokesDeCadaMapa,mapa)!=NULL;posObjetivo++){

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


				moverseEnUnaDireccion(posXInicial, posYInicial, x, y, protocAManejar, servidor);


				protocAManejar[1]='9'; // Solicitud Atrapar Pokemon
				send(servidor,protocAManejar,2,0);
				solicitarAtraparPokemon(calculoTiempo,tiempo,masFuertePokeYNivel,pokePiola, cantDeadlocks,listaNivAtrapados,puntoMontaje,mapa, ent,servidor);

				// por si se cae
				recv(servidor, (void *)resultado, sizeof(int), 0);
				resultadoEnvio = *((int *)resultado);

				if(resultadoEnvio == 9){
					printf("Servidor caído! imposible reconectar. Cerrando...\n");
					exit(0);
				}

				dictionary_remove(pokesDeCadaMapa,mapa);
			} // cierro el for de los objetivos


			copiarMedalla(puntoMontaje, mapa, ent);

			close(servidor); // se desconecta el entrenador
		 }
	} while((ent->cantidadInicialVidas!=0) && (list_get(ent->hojaDeViaje,pos)!=NULL));

terminarAventura(calculoTiempo,tiempo,cantDeadlocks,horaInicio);

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

return EXIT_SUCCESS;
}

///////////////////// FUNCIONES DEL ENTRENADOR ///////////////////////////
void  usoDeSeniales(t_entrenador* ent, char* puntoMontaje){

void reciboUnaVida(){
	ent->cantidadInicialVidas++;
}

void pierdoUnaVida(){
	morir(ent,puntoMontaje,"senial");
}

signal(SIGUSR1, reciboUnaVida);
signal(SIGTERM, pierdoUnaVida);
}

void* solicitarAtraparPokemon(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo, t_dictionary* masFuertePokeYNivel,t_pokemonDeserializado* pokePiola,int cantDeadlocks,t_list* listaNivAtrapados,char* puntoMontaje, char* mapa, t_entrenador* ent, int servidor){
	//pongo el tiempo para despues restarlo y saber cuando estuve bloqueado
	char* inicioBloq = temporal_get_string_time();
	char* finBloq;

	//char* operacion = recibirDatos(servidor,21);
	/* podria recibir serializado:
	caracter nro especie nombreMetadata nivel sin los ; porque usamos un offset*/


	char* operacion =  "@1;Pikachu/Pikachu001;33";
	char** cosasRecibidas = string_n_split(operacion,3,";");

	//manejo el caracter y el nro por un lado aca
	char* caracYNro = strdup(cosasRecibidas[0]);

	char caracterRec = caracYNro[0];
	char* caracterRecString = string_from_format("%c", caracterRec);
	pokePiola->caracter=caracterRecString;

	char nroRecChar = caracYNro[1];
	char* nroRecString = string_from_format("%c",nroRecChar);
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

	if(!strcmp(ent->caracter,pokePiola->caracter)){
		switch(pokePiola->protocolo){
			case ATRAPA:
				//simulo un tiempo de bloqueado de pokenest por el momento
				usleep(1000);
				list_add(listaNivAtrapados,pokePiola->nivelPokemon);
				finBloq = temporal_get_string_time();
				copiarArchivo(especifico, puntoMontaje,mapa,ent,pokePiola->nombreMetadata);
				tiempo = sacarTiempo(calculoTiempo,tiempo,"bloqueado",inicioBloq,finBloq);
				//revisar si lo puedo manejar casteandolo o con el char* y fue
				dictionary_put(masFuertePokeYNivel,nivelPoke,especifico);
				return tiempo;


			case DEADLOCK:
				cantDeadlocks = cantDeadlocks + 1;
				int nivelPokeAtrapado = agarrarPokeConMasNivel(listaNivAtrapados, pokePiola);
				char* nivelString = string_itoa(nivelPokeAtrapado);
				char* especificoMasFuerte = dictionary_get(masFuertePokeYNivel,nivelString);
				char* pokeMFuerte = string_from_format("%s5;%s;%d",ent->caracter,especificoMasFuerte,nivelPokeAtrapado);

				//revisar ese sizeof
				send(servidor,pokeMFuerte,sizeof(pokeMFuerte),0);
				free(pokeMFuerte);
				return cantDeadlocks;


			case MORI:
				morir(ent,puntoMontaje,"deadlock");
			break;
		}
	}

	free(nivelPoke);
	free(nroRecString);
	free(caracterRecString);
	free(caracYNro);
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


void terminarAventura(t_calculoTiempo* calculoTiempo,t_tiempoBloqueado* tiempo,int cantDeadlocks,char* horaInicio){
	char* horaFin = temporal_get_string_time();
	sacarTiempo(calculoTiempo,tiempo,"aventura",horaInicio,horaFin);

	char* tiempoTotalBloqueado = string_from_format("El tiempo total que paso bloqueado en las pokenests fue: %d:%d:%d:%d", (tiempo)->horasBloqueado,(tiempo)->minutosBloqueado,(tiempo)->segundosBloqueado,(tiempo)->milesimasBloqueado);
	log_info(logs,tiempoTotalBloqueado);
	free(tiempoTotalBloqueado);

	char* deadlocks = string_from_format("la cantidad de deadlocks es: %d \n", cantDeadlocks);
	log_info(logs,deadlocks);
	free(deadlocks);

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

void copiarMedalla(char* puntoMontaje, char* mapa, t_entrenador* ent){

	char* medalla=string_from_format("cp %s/Mapas/%s/medalla-%s.jpg %s/Entrenadores/%s/medallas/medalla-%s.jpg", puntoMontaje, mapa, mapa, puntoMontaje, (ent)->nombreEntrenador,mapa);
	system(medalla);

	char* logueo = string_from_format("Copiada medalla del Mapa %s con exito \n", mapa);
	log_info(logs, logueo);

	free(medalla);
	free(logueo);

return;
}

void copiarArchivo(char* especifico, char* puntoMontaje,char* mapa,t_entrenador* ent,char* rutaPokeAtrapado){

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


void moverseEnUnaDireccion(int posXInicial, int posYInicial,int x, int y, char* protocAManejar, int servidor){
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

//ENGANCHAR ESTAS FUNCIONES

void morir(t_entrenador* ent, char* puntoMontaje, char* motivo){
		ent->cantidadInicialVidas = ent->cantidadInicialVidas-1;
		if (ent->cantidadInicialVidas>0){
			if(!strcmp(motivo,"deadlock")){
				log_info(logs,"Moriste por deadlock\n");
				borrarArchivosBill(ent, puntoMontaje);
				close(servidor);
				reconectarse(ent); // Ver bien como llevar a cabo esto, no esta
				} else if (!strcmp(motivo,"senial")){
						log_info(logs,"Moriste por la senial SIGTERM\n");
				}
		} else {
			char respuesta[3];
			char* reintentos = string_from_format("Numero de reintentos realizados hasta el momento: %d\n", ent->reintentos);
			log_info(logs,reintentos);
			free(reintentos);
			log_info(logs,"Desea reiniciar juego?\n");
			fgets(respuesta, 3, stdin);
			if (!strcmp(respuesta,"si")){
				printf("Reseteando...\n");
				ent->reintentos = ent->reintentos+1;
				resetear(ent, puntoMontaje);
			}else if(!strcmp(respuesta,"no")) {
				log_info(logs,"Cerrando programa\n");
				exit(0);
			}
		}
}


void borrarArchivosBill(t_entrenador* ent, char* puntoMontaje){ // Al momento de testear va a decir que no encontro nada si la carpeta estaba vacia
		char* borrarBill = string_from_format("rm -r %s/Entrenadores/%s/Dir\\ De\\ Bill/*",puntoMontaje,ent->nombreEntrenador);
		system(borrarBill);
		free(borrarBill);
}

void borrarMedallas(t_entrenador* ent, char* puntoMontaje){
		char* borrarMedallas = string_from_format("rm -r %s/Entrenadores/%s/medallas/*",puntoMontaje,ent->nombreEntrenador);
		system(borrarMedallas);
		free(borrarMedallas);
}


void resetear(t_entrenador* ent, char* puntoMontaje){
	//reiniciarHojaDeViaje(ent); //Falta implementar, como seria esto?
	borrarMedallas(ent, puntoMontaje);
	borrarArchivosBill(ent, puntoMontaje);
}

void reiniciarHojaDeViaje(t_entrenador *entrenador){

}

void reconectarse(t_entrenador* ent){
	return;
}
