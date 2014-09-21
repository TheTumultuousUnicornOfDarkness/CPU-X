EXEC=cpu-x
CC=gcc
FLAGS=-O
GTKFLAGS=`pkg-config --cflags --libs gtk+-3.0`
ifeq ($(shell [[ -f /usr/include/libcpuid/libcpuid.h ]] && echo true),true)
LIBCPUIDLIB_SO=-l:libcpuid.so
LIBCPUIDLIB_A=-l:libcpuid.a
LIBCPUIDSET=-DLIBCPUID
endif

prefix=/usr/local
bindir=$(prefix)/bin
datadir=$(prefix)/share/$(EXEC)

all: $(EXEC)

$(EXEC): cpu-x.o gui.o
	$(CC) $(GTKFLAGS) $^ -o $@ $(LIBCPUIDLIB_SO) 

$(EXEC)_portable: cpu-x.o gui.o
	$(CC) $(GTKFLAGS) $^ -o $@ $(LIBCPUIDLIB_A) 

cpu-x.o: src/cpu-x.c src/cpu-x.h
	$(CC) $(FLAGS) -c $< $(GTKFLAGS) $(EMBEDSET) $(LIBCPUIDSET)

gui.o: src/gui.c src/cpu-x.h
	$(CC) $(FLAGS) -c $< $(GTKFLAGS) $(EMBEDSET)


.PHONY: install uninstall clean mrproper embed

install:
	install -d $(bindir) $(datadir)
	install -vm755 $(EXEC) $(bindir)
	install -vm644 data/* $(datadir)

uninstall:
	rm -v $(bindir)/$(EXEC)
	rm -Rv $(datadir)

embed:
	sh embed.sh
	EMBEDSET=-DEMBED $(MAKE) $(EXEC)_portable

clean:
	@rm -fv *.o
	@rm -rf embed/

mrproper: clean
	@rm -fv $(EXEC)
	@rm -fv $(EXEC)_portable
