ARCH = $(shell getconf LONG_BIT)
LIBSPATH_64 = -L/usr/lib32 -L/lib32
CFLAGS_64 = -m32 -c
LIBSPATH_32 = -L/lib -L/usr/lib -L/usr/local/lib
CFLAGS_32 =  

all: flow2 helloworld sections playback 

flow2:
	$(CC) $(CFLAGS_$(ARCH)) -c $@.c
	../bin/rld $(LIBSPATH_$(ARCH)) -lGL -lSDL -o $@ ../src/runtime/startup_x86.o ./$@.o
	../bin/rld --compress $(LIBSPATH_$(ARCH)) -lGL -lSDL -o $@-compressed ../src/runtime/startup_x86.o ./$@.o

helloworld:
	gcc $(CFLAGS_$(ARCH)) -c $@.c
	../bin/rld $(LIBSPATH_$(ARCH)) -lc -o $@ ../src/runtime/startup_x86.o ./$@.o
	../bin/rld --compress $(LIBSPATH_$(ARCH)) -lc -o $@-compressed ../src/runtime/startup_x86.o ./$@.o

sections:
	gcc $(CFLAGS_$(ARCH)) -c $@.c
	../bin/rld $(LIBSPATH_$(ARCH)) -lc -o $@ ../src/runtime/startup_x86.o ./$@.o

playback:
	gcc $(CFLAGS_$(ARCH)) -c $@.c
	../bin/rld $(LIBSPATH_$(ARCH)) -lSDL -o $@ ../src/runtime/startup_x86.o ./$@.o
	../bin/rld --compress --verbose $(LIBSPATH_$(ARCH)) -lSDL -o $@-compressed ../src/runtime/startup_x86.o ./$@.o

clean:
	rm -fv *.o *-compressed flow2 helloworld sections playback
