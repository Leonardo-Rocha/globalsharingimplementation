# The compilation

CC=gcc
CFLAGS=-g -std=c99 -pedantic -Wall
LDFLAGS=-lm

SOURCE=$(wildcard src/*.c)
INCLUDES =$(wildcard include/*.h)
MAIN=simpred

all:  $(MAIN)
	@echo Programa compilado com sucesso!


$(MAIN): $(SOURCE)
	
	$(CC) $^ $(CFLAGS) -o $(MAIN) $(LDFLAGS) 

static: $(SOURCE)

	$(CC) $^ $(CFLAGS) -o $(MAIN) -static 

clean:
	rm -f $(MAIN) *.o

#configure:
	#@echo Searching Dependences unsolved... 
	#@./configure