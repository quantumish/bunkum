CFLAGS?=-O2 -g
LIBS:=-lz -lpthread
SOURCES = $(wildcard utils/*.c) $(wildcard http/*.c) http.c
OBJ = $(wildcard build/*.o)

.PHONY: all clean

all: serv

serv: $(SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm serv
