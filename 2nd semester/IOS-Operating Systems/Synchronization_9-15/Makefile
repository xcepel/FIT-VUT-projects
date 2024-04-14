# Makefile
# IOS - projekt 2 (synchronizace)
# 30. 4. 2022
# Autor: Katerina Cepelkova, xcepel03
# VUT FIT
#
#

EXECUTABLES = proj2
CC = gcc
CFLAGS = -std=gnu99 -pedantic -Wall -Wextra -Werror
LDLIBS = -pthread

all: $(EXECUTABLES)

zip:
	zip proj2.zip *.c *.h Makefile

clean:
	rm -f $(EXECUTABLES) *.o
