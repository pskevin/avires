CC = gcc
CFLAGS = -g -w -O2 -fPIC
INCLUDES = -I/usr/local/include
LIBS = -lm -lpthread -ldl

OBJ = ./obj
SRC = ./src

default: $(OBJ)/gups.o

$(OBJ):
	@echo "\nOutput directory $(OBJ) doesn't exist. Creating it!"
	mkdir $@

$(SRC):
	@echo "\nSource directory $(SRC) doesn't exist. Failing!"
	false;

$(OBJ)/gups.o: $(OBJ)/gups.part $(OBJ)/zipf.part $(OBJ)/timer.part # TODO(pskevin): Needs to written more concisely
	@echo "\nLinking all intermediate objects into single binary."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OBJ)/gups.o $^ $(LIBS) -L.
	$(MAKE) clean_parts

$(OBJ)/%.part: $(SRC)/%.c
	@mkdir -p $(OBJ)
	@echo "\nBuilding intermediate objects."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean_parts clean

clean_parts:
	rm $(OBJ)/*.part

clean:
	rm $(OBJ)/*
