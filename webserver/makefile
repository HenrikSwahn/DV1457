CC=gcc
HDRS=./include
SRC=./src
OBJ=./objects
CFLAGS=-I$(HDRS)

_HEADERS=server.h error.h http.h conf.h
HEADERS=$(patsubst %,$(HDRS)/%,$(_HEADERS)) 

_SOURCE=main.c server.c error.c http.c conf.c
SOURCE=$(patsubst %,$(SRC)/%,$(_SOURCE))

_OBJECTS=main.o server.o error.o http.o conf.o
OBJECTS=$(patsubst %,$(OBJ)/%,$(_OBJECTS))

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJECTS) $(HEADERS)
	gcc -o  $@ $^ $(CFLAGS)

.PHONY: clean

clean: 
	rm -f $(OBJ)/*.o
	rm server
