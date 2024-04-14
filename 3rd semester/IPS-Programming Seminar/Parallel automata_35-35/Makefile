CC = g++
CFLAGS = -pedantic -Wall -Wextra -g
LDLIBS = -lpthread

.PHONY: all

all: aut

aut: aut.c

test: aut
	bash ./test.sh

clean:
	rm aut