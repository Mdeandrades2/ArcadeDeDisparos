CC = gcc
CFLAGS = -Wall
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXECUTABLE = programa
RUN = ./$(EXECUTABLE)
all: $(EXECUTABLE)
$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
run: $(EXECUTABLE)
	$(RUN)
clean:
	rm -f $(OBJ) $(EXECUTABLE)
