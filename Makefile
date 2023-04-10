CFLAGS?=-O2 -g
LIBS:=-lz -lpthread
SOURCES = $(wildcard utils/*.c) $(wildcard http/*.c) http.c
OBJ = $(wildcard build/*.o)

.PHONY: all clean run

all: serv

serv: $(SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

run: serv
	sudo docker build -t bunkum-dev .
	sudo docker run -it -p "8080:8082" bunkum-dev

clean:
	rm serv
