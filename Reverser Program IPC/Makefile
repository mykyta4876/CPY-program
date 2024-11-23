CC = gcc
CFLAGS = -Wall -Wextra

all: reverser controller

reverser: reverser.c
	$(CC) $(CFLAGS) -o reverser reverser.c

controller: controller.c
	$(CC) $(CFLAGS) -o controller controller.c

clean:
	rm -f reverser controller