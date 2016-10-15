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

int agarrarPokeConMasNivel(t_list*, t_pokemonDeserializado*);
void terminarAventura(int,char*,int,int,int,int,int,int,int,int);
char* empezarAventura();
void copiarMedalla(char*,char*,t_entrenador*);
void copiarArchivo(char*, char*, char*, t_entrenador*,char*);
void* recibirDatos(int, int);
void moverseEnUnaDireccion(int,int,int,int,char*,int);
void solicitarAtraparPokemon(t_dictionary*,t_pokemonDeserializado*,int, t_list*,char*, char*,t_entrenador*, int,int,int,int,int,int,int,int,int);
void sacarTiempo(char*,char*,char*,int,int,int,int,int,int,int,int);

void mostrarMotivo();
void borrarArchivosBill(t_entrenador*, char*);
void borrarMedallas(t_entrenador*, char*);
int leQuedanVidas(t_entrenador*);
void resetear(t_entrenador*, char*);
void reconectarse(t_entrenador*);
int murioEntrenador(t_entrenador*);
void morir(t_entrenador*, char*);
void reiniciarHojaDeViaje(t_entrenador*);


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


	 //CONFIG
	 char* configEntrenador = string_from_format("%s/Entrenadores/%s/metadata",puntoMontaje,nombreEnt);

	 //para cuando debuggeamos, descomentar lo de abajo y comentar lo de arriba
	 //char* configEntrenador = "/home/utnso/workspace/pokedex/Entrenadores/Red/metadata";

	 if (!leerConfigEnt(configEntrenador,&ent, puntoMontaje)) {
	     log_error(logs,"Error al leer el archivo de configuracion de Metadata Entrenador\n");
	     return 1;
	 }

	 log_info(logs,"Archivo de config Entrenador creado exitosamente!\n");

	 //VARIABLES USADAS Y CONEXION

	 int servidor;
	 int pos;
	 int cantMapas = list_size((ent)->hojaDeViaje);
	 int posObjetivo;

	 int hInicio=0;
	 int mInicio=0;
	 int sInicio=0;
	 int milInicio=0;

	 int hFin=0;
	 int mFin=0;
	 int sFin=0;
	 int milFin=0;

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
	 tiempoBloqCadaPokenest = list_create();

	 horaInicio = empezarAventura();

	 for(pos = 0;pos<cantMapas;pos++){


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
			solicitarAtraparPokemon(masFuertePokeYNivel,pokePiola, cantDeadlocks,listaNivAtrapados,puntoMontaje,mapa, ent,servidor, hInicio, mInicio, sInicio, milInicio,hFin,mFin,sFin,milFin);

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

terminarAventura(cantDeadlocks,horaInicio, hInicio,mInicio,sInicio,milInicio,hFin,mFin,sFin,milFin);

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

return EXIT_SUCCESS;
}

///////////////////// FUNCIONES DEL ENTRENADOR ///////////////////////////
void solicitarAtraparPokemon(t_dictionary* masFuertePokeYNivel,t_pokemonDeserializado* pokePiola,int cantDeadlocks,t_list* listaNivAtrapados,char* puntoMontaje, char* mapa, t_entrenador* ent, int servidor,int hInicio,int mInicio,int sInicio,int milInicio,int hFin,int mFin,int sFin,int milFin){
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
				sacarTiempo("bloqueado",inicioBloq,finBloq, hInicio, mInicio, sInicio, milInicio,hFin,mFin,sFin,milFin);
				//revisar si lo puedo manejar casteandolo o con el char* y fue
				dictionary_put(masFuertePokeYNivel,nivelPoke,especifico);
			break;

			case DEADLOCK:
				cantDeadlocks = cantDeadlocks + 1;
				int nivelPokeAtrapado = agarrarPokeConMasNivel(listaNivAtrapados, pokePiola);
				char* nivelString = string_itoa(nivelPokeAtrapado);
				char* especificoMasFuerte = dictionary_get(masFuertePokeYNivel,nivelString);
				char* pokeMFuerte = string_from_format("%s5;%s;%d",ent->caracter,especificoMasFuerte,nivelPokeAtrapado);

				//revisar ese sizeof
				send(servidor,pokeMFuerte,sizeof(pokeMFuerte),0);
				free(pokeMFuerte);
			break;

			case MORI:

			break;
		}
	}

	free(nivelPoke);
	free(nroRecString);
	free(caracterRecString);
	free(caracYNro);
	return;
}

int agarrarPokeConMasNivel(t_list* listaNivAtrapados, t_pokemonDeserializado* pokePiola){

	int ordenarDeMayorAMenor(t_pokemonDeserializado* niv1, t_pokemonDeserializado* niv2){
		return (niv1->nivelPokemon>niv2->nivelPokemon);
	}

	list_sort(listaNivAtrapados,(void*)ordenarDeMayorAMenor);

	int nivelPokeDevuelto = list_get(listaNivAtrapados,0);
	return nivelPokeDevuelto;
}


void terminarAventura(int cantDeadlocks,char* horaInicio, int hInicio,int mInicio,int sInicio,int milInicio,int hFin,int mFin,int sFin,int milFin){
	char* horaFin = temporal_get_string_time();
	sacarTiempo("aventura",horaInicio,horaFin, hInicio, mInicio, sInicio, milInicio,hFin,mFin,sFin,milFin);

	char* deadlocks = string_from_format("la cantidad de deadlocks es: %d \n", cantDeadlocks);
	log_info(logs,deadlocks);
	free(deadlocks);

	return;
}

void sacarTiempo(char* estado,char* horaInicio,char* horaFin,int hInicio,int mInicio, int sInicio,int milInicio,int hFin,int mFin,int sFin,int milFin){
	//separo hh mm ss mmmm y los guardo en char**, fundamental
		char** inicio = string_n_split(horaInicio,4,":");
		char** fin = string_n_split(horaFin,4,":");

		// convierto a enteros los char*
		hInicio = atoi(inicio[0]);
		mInicio = atoi(inicio[1]);
		sInicio = atoi(inicio[2]);
		milInicio = atoi(inicio[3]);

		hFin = atoi(fin[0]);
		mFin = atoi(fin[1]);
		sFin = atoi(fin[2]);
		milFin = atoi(fin[3]);

		int horasAventura = hFin - hInicio;
		int minAventura = mFin - mInicio;
		int segAventura = sFin - sInicio;
		int milAventura = milFin - milInicio;

		if(!strcmp(estado,"bloqueado")){
			t_tiempoBloqCadaPokenest* tBloqPoke;
			tBloqPoke = malloc(sizeof(t_tiempoBloqCadaPokenest));

			tBloqPoke->horas=horasAventura;
			tBloqPoke->minutos=minAventura;
			tBloqPoke->segundos=segAventura;
			tBloqPoke->milesimas=milAventura;

			//le agrego la estructura entera, y despues ya la libero y me manejo con la lista
			list_add(tiempoBloqCadaPokenest,tBloqPoke);
			free(tBloqPoke);

			char* mensBloq = string_from_format("El tiempo que paso bloqueado fue: %d:%d:%d:%d  \n",horasAventura,minAventura,segAventura,milAventura);
			log_info(logs,mensBloq);
			free(mensBloq);
		} else if (!strcmp(estado,"aventura")){
			char* aviso = string_from_format("Ahora sos un maestro pokemon, lo lograste a las %s \n", horaFin);
			log_info(logs,aviso);
			free(aviso);

			char* mensTiempo = string_from_format("La aventura duró: %d:%d:%d:%d  \n",horasAventura,minAventura,segAventura,milAventura);
			log_info(logs,mensTiempo);
			free(mensTiempo);
		}

	return;
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

int murioEntrenador(t_entrenador *ent){ // Poner return 1 para habilitar muerte de entrenador

	//if(murioPorDeadlock(ent) || murioPorSIGTERM(ent) || murioPorKill(ent)){ //Mapa informa de esto
	//	return 1;
	//}
	return 0;
}


void morir(t_entrenador* ent, char* puntoMontaje){
	mostrarMotivo();
	borrarArchivosBill(ent, puntoMontaje);
	if (leQuedanVidas(ent)){
		ent->cantidadInicialVidas = ent->cantidadInicialVidas-1; //Hacer que persista en metadatas de entrenador tambien
		reconectarse(ent); // Ver bien como llevar a cabo esto
	}
	else {
	char respuesta[3];
	printf("Numero de reintentos: %d\n", ent->reintentos);
	printf("Desea reiniciar juego?\n");
	fgets(respuesta, 3, stdin);
    if (!strcmp(respuesta,"si")){
    	printf("Reseteando...\n");
    	 ent->reintentos = ent->reintentos+1;
    	 resetear(ent, puntoMontaje);
    	 reconectarse(ent);
    	}
     else {
    	 puts("Cerrando programa");
    	 exit(0);
     	 }
	}
	}

void mostrarMotivo(){
	printf("Motivo de muerte: HARDCODEADO\n");
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

int leQuedanVidas(t_entrenador* ent){
	return ent->cantidadInicialVidas;
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
