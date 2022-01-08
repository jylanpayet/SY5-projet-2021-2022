CC=gcc
EXEC=../cassini
CFLAGS=-Wall -g #-Werror
INCLUDES=include
SRC=src/cassini.c src/client.c src/timing-text-io.c
OBJ=$(SRC:.c=.o)
all: $(EXEC)
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< -I $(INCLUDES)
$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm $(OBJ)
