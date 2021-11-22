CC=gcc
EXEC=cassini
CFLAGS=-Wall
INCLUDES=include
SRC=src/*.c
OBJ=$(SRC:.c=.o)
all: $(EXEC)
%.o : %.c
	$(CC) -o $@ -c $< -I $(INCLUDES)
$(EXEC) : $(OBJ)
	$(CC) -o $@ $^

