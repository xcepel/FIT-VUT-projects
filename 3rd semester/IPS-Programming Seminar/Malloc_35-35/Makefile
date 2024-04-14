CFLAGS=-std=gnu99 -Wall -Wextra -g
#CFLAGS=-std=gnu99 -Wall -Wextra -g -DNDEBUG
UNAME_S := $(shell uname -s)
test_mmal: mmal.o test_mmal.o
	cc -o $@ $^
test: test_mmal testrun
testrun:
ifeq ($(UNAME_S),Linux)
		@if setarch `uname -m` -R true 2>/dev/null; then setarch `uname -m` -R ./test_mmal; else ./test_mmal; fi
else
		./test_mmal
endif
mmal.o: mmal.c mmal.h
test_mmal.o: test_mmal.c mmal.h
clean:
	-rm mmal.o test_mmal.o test_mmal
