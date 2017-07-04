CXX = gcc
SOURCES = src/tas.c src/graphe.c src/probleme.c src/reoptimisation.c src/knapglpk.c src/2DKPSurrogate/2DKPSurrogate.c src/2DKPSurrogate/combo.c
CFLAGS = -std=c99 -Wall -g -Ofast
LDFLAGS = -lm -lglpk
.PHONY = clean

all: main

main: $(SOURCES:.c=.o) $(LDFLAGS)

clean: 
	rm ./src/*.o
	rm ./src/2DKPSurrogate/*.o