#!/bin/bash

cd
cd workspace/tp-2016-2c-Ni-Lo-Testeamos/Entrenador/src

gcc Entrenador.c libSockets.c libSockets.h -o entrenador -lcommons -lfuse -lpthread

./entrenador Ash /home/utnso/workspace/pokedex/01-base

