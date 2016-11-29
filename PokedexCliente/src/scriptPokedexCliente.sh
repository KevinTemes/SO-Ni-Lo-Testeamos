#!/bin/bash

cd
cd workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexCliente/src

gcc libSockets.c libSockets.h PokedexCliente.c -o pokeCliente -lcommons -lfuse -lpthread -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64

./pokeCliente /home/utnso/workspace/tp-2016-2c-Ni-Lo-Testeamos/PokedexCliente/Debug/montaje/tmp -s -f
