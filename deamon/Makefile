CC=gcc
EXEC=../saturnd
CFLAGS=-Wall -g #-Werror
INCLUDES=../common
SRC=src/saturnd.c src/daemon.c ../client/src/timing-text-io.c
OBJ=$(SRC:.c=.o)
all: $(EXEC)
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< -I $(INCLUDES)
$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm $(OBJ)
