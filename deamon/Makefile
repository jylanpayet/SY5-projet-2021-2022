CC=gcc
EXEC=../saturnd
CFLAGS=-Wall -g #-Werror
INCLUDES=include
SRC=src/saturnd.c src/daemon.c
OBJ=$(SRC:.c=.o)
all: $(EXEC)
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< -I $(INCLUDES)
$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm $(OBJ)
