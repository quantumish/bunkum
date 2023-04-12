CFLAGS?=-O2 -g
LIBS:=-lz -lpthread
LIBS = $(wildcard utils/*.c) $(wildcard http/*.c)
OBJ = $(wildcard build/*.o)

.PHONY: all clean run test

all: serv

serv: $(LIBS) http.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

run: serv
	sudo docker build -t bunkum-dev .
	sudo docker run -it -p "8080:8082" bunkum-dev

test: $(LIBS) test.c
	$(CC) $(LIBS) -shared -fPIC $(CFLAGS) -o libbunkum.so
	$(CC) test.c -o test -ldl
	./test libbunkum.so

clean:
	rm serv
