CC = gcc
CFLAGS = -g -w -O2 -fPIC
INCLUDES = -I/usr/local/include
LIBS = -lm -lpthread -ldl

OBJ = ./obj
SRC = ./src

default:  $(OBJ)/gups_rndm_read.o $(OBJ)/gups_rndm_write.o $(OBJ)/gups_seq_read.o $(OBJ)/gups_seq_write.o

$(OBJ):
	@echo "\nOutput directory $(OBJ) doesn't exist. Creating it!"
	mkdir $@

$(SRC):
	@echo "\nSource directory $(SRC) doesn't exist. Failing!"
	false; 

$(OBJ)/gups_rndm_read.o: $(OBJ)/gups_rndm_read.part $(OBJ)/log.part $(OBJ)/lfsr.part
	@echo "\nLinking all intermediate objects into single binary."
	$(CC) $(CFLAGS)  $(INCLUDES)  $^ -o $(OBJ)/gups_rndm_read.o $(LIBS) -L.

$(OBJ)/gups_rndm_write.o: $(OBJ)/gups_rndm_write.part $(OBJ)/log.part $(OBJ)/lfsr.part
	@echo "\nLinking all intermediate objects into single binary."
	$(CC) $(CFLAGS)  $(INCLUDES)  $^ -o $(OBJ)/gups_rndm_write.o $(LIBS) -L.

$(OBJ)/gups_seq_read.o: $(OBJ)/gups_seq_read.part $(OBJ)/log.part
	@echo "\nLinking all intermediate objects into single binary."
	$(CC) $(CFLAGS)  $(INCLUDES)  $^ -o $(OBJ)/gups_seq_read.o $(LIBS) -L.

$(OBJ)/gups_seq_write.o: $(OBJ)/gups_seq_write.part $(OBJ)/log.part
	@echo "\nLinking all intermediate objects into single binary."
	$(CC) $(CFLAGS)  $(INCLUDES)  $^ -o $(OBJ)/gups_seq_write.o $(LIBS) -L.

DEBUG=0

$(OBJ)/gups_rndm_read.part: $(SRC)/gups.c
	$(CC) $(CFLAGS) -DLOGGING=$(DEBUG) -DPATTERN=0 -DTYPE=0 $(INCLUDES) -c $^ -o $@

$(OBJ)/gups_seq_read.part: $(SRC)/gups.c
	$(CC) $(CFLAGS) -DLOGGING=$(DEBUG) -DPATTERN=1 -DTYPE=0 $(INCLUDES) -c $< -o $@

$(OBJ)/gups_rndm_write.part: $(SRC)/gups.c 
	$(CC) $(CFLAGS) -DLOGGING=$(DEBUG) -DPATTERN=0 -DTYPE=1 $(INCLUDES) -c $^ -o $@

$(OBJ)/gups_seq_write.part: $(SRC)/gups.c
	$(CC) $(CFLAGS) -DLOGGING=$(DEBUG) -DPATTERN=1 -DTYPE=1 $(INCLUDES) -c $< -o $@


$(OBJ)/log.part: $(SRC)/log.c
	$(CC) $(CFLAGS) -DLOGGING=$(DEBUG) $(INCLUDES) -c -o $@ $<

$(OBJ)/%.part: $(SRC)/%.c
	@mkdir -p $(OBJ)
	@echo "\nBuilding intermediate objects."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean_parts clean

clean_parts:
	rm $(OBJ)/*.part

clean:
	rm $(OBJ)/*
