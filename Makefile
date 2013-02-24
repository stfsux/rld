include Makefile.conf

all: $(PROJECT) $(OBJSRT)

install: $(PROJECT) $(OBJSRT)
	mkdir -p $(PREFIX)/lib/$(PROJECT)
	cp -vr src/runtime/startup_x86.o $(PREFIX)/lib/$(PROJECT)
	cp -vr bin/$(PROJECT) $(PREFIX)/bin/$(PROJECT)
	cp -vr tools/rlds $(PREFIX)/bin/rlds
	chmod ugo+rx $(PREFIX)/bin/rlds

uninstall:
	rm -vf $(PREFIX)/lib/rld/startup_x86.o $(PREFIX)/bin/rlds $(PREFIX)/bin/rld

$(PROJECT): $(OBJS)
	test -d bin || mkdir bin
	$(CC) -g $(LDFLAGS) -o bin/$(PROJECT) $^

%.o: %.c
	$(CC) -g $(CFLAGS) -c $< -o $@

src/runtime/startup_x86.o: src/runtime/startup_x86.asm 
ifeq ($(PLATFORM_X86),yes)
	$(AS_X86) --32 src/runtime/startup_x86.asm -o src/runtime/startup_x86.o
endif

clean:
	rm -vf src/*.o src/platform/*.o src/runtime/*.o bin/$(PROJECT)
	@(cd examples && $(MAKE) clean)

.PHONY: clean
