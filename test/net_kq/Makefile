CC=gcc44
SVR_TARGET=server
CLI_TARGET=client
CFLAGS=-g
OPTIMIZE=-O2
LFLAGS=-lmcl

LIBRARY=-L../../lib
INCLUDES=-I../../include/ -I./ -I/user/local/include

all:bin after_process

server.o: server.c
	$(CC) $(CFLAGS) $(MAKEARGS) $(INCLUDES) $(OPTIMIZE) -o $@ -c $<
client.o: client.c
	$(CC) $(CFLAGS) $(MAKEARGS) $(INCLUDES) $(OPTIMIZE) -o $@ -c $<

bin:server.o client.o
	$(CC)  server.o -o $(SVR_TARGET) $(LIBRARY) $(LFLAGS)
	$(CC)  client.o -o $(CLI_TARGET) $(LIBRARY) $(LFLAGS)

after_process:
	-rm -f *.o

clean:
	-rm -f $(SVR_TARGET)
	-rm -f $(CLI_TARGET)

