CFLAGS?= -g -std=c99
LIBS:=-lz -lpthread -lunwind -lunwind-ptrace -lunwind-generic
CLIBS = $(wildcard utils/*.c) $(wildcard http/*.c)
OBJ = $(wildcard build/*.o)

.PHONY: all clean run test

all: serv

serv: $(CLIBS) http.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -O3

run: serv
	sudo docker build -t bunkum-dev .
	sudo docker run -it -p "8080:8082" bunkum-dev

test: $(CLIBS) test.c
	$(CC) $(CLIBS) testutils.c -shared -fPIC $(CFLAGS) -o libbunkum.so $(LIBS) -DTEST
	$(CC) test.c -o test -ldl
	./test libbunkum.so

static:
	$(CC) $(CLIBS) testutils.c -shared -fPIC $(CFLAGS) -o bunkum.o $(LIBS) -DTEST
	ar rvs bunkum.a bunkum.o

clean:
	rm serv
