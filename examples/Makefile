
all: flow2 helloworld

flow2:
	gcc -m32 -Os -c flow2.c
	../bin/rld -L/usr/lib32 -lGL -lSDL -o flow2 ../src/runtime/runtime_x86.o ./flow2.o
	../bin/rld --compress -L/usr/lib32 -lGL -lSDL -o flow2-compressed ../src/runtime/runtime_x86.o ./flow2.o

helloworld:
	gcc -m32 -Os -c helloworld.c
	../bin/rld -L/usr/lib32 -lc=/lib32/libc.so.6 -o helloworld ../src/runtime/runtime_x86.o ./helloworld.o
	../bin/rld --compress -L/usr/lib32 -lc=/lib32/libc.so.6 -o helloworld-compressed ../src/runtime/runtime_x86.o ./helloworld.o

clean:
	rm -fv *.o *-compressed flow2 helloworld
