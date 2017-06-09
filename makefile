CXX = gcc
SOURCES = src/tas.c src/graphe.c
CFLAGS = -std=c99 -Wall -g -Ofast
LDFLAGS =
.PHONY = clean

all: main

main: $(SOURCES:.c=.o) $(LDFLAGS)

clean: 
	rm ./src/*.o