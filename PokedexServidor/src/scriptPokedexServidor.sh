#!/bin/bash

cd
cd workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/src

gcc PokedexServidor.c PokedexServidor.h libreriaPokedexServidor.c libreriaPokedexServidor.h libSockets.c libSockets.h osada.h -o pokeServidor -lcommons -lpthread

./pokeServidor /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexServidor/tranki.bin
