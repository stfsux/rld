include Makefile.conf

all: $(PROJECT) $(OBJSRT)

$(PROJECT): $(OBJS)
	$(CC) -g $(LDFLAGS) -o bin/$(PROJECT) $^

%.o: %.c
	$(CC) -g $(CFLAGS) -c $< -o $@

src/runtime/runtime_x86.o: src/runtime/runtime_x86.asm 
ifeq ($(PLATFORM_X86),yes)
	$(AS_X86) --32 src/runtime/runtime_x86.asm -o src/runtime/runtime_x86.o
endif

clean:
	rm -vf src/*.o src/platform/*.o src/runtime/*.o bin/$(PROJECT)

.PHONY: clean
