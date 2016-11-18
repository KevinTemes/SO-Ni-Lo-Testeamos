#!/bin/bash

cd
cd workspace/tp-2016-2c-Ni-Lo-Testeamos/Mapa/src

gcc Mapa.c libreriaMapa.c libreriaMapa.h libSockets.c libSockets.h -o mapa -lcommons -lfuse -lnivel-gui -lcurses -lpthread -lpkmn-battle

./mapa GrisCompleta /home/utnso/workspace/pokedex
