ARCH = $(shell getconf LONG_BIT)
ifeq ($(ARCH), 64)
LIBSPATH = -L/usr/lib32 -L/lib32
CFLAGS = -m32 -Os -c
else
LIBSPATH = -L/lib -L/usr/lib -L/usr/local/lib
CFLGAS = -Os
endif

all: flow2 helloworld

flow2:
	$(CC) $(CFLAGS) -c $@.c
	../bin/rld $(LIBSPATH) -lGL -lSDL -o $@ ../src/runtime/runtime_x86.o ./$@.o
	../bin/rld --compress $(LIBSPATH) -lGL -lSDL -o $@-compressed ../src/runtime/runtime_x86.o ./$@.o

helloworld:
	gcc $(CFLAGS) -c $@.c
	../bin/rld $(LIBSPATH) -lc -o $@ ../src/runtime/runtime_x86.o ./$@.o
	../bin/rld --compress $(LIBSPATH) -lc -o $@-compressed ../src/runtime/runtime_x86.o ./$@.o

clean:
	rm -fv *.o *-compressed flow2 helloworld
