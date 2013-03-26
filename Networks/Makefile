all: Server/server.o Client/client.o
	gcc -Wall Server/server.o -o Server/server 
	gcc -Wall Client/client.o -o Client/client
server: Server/server.o
	gcc -Wall Server/server.c -o Server/server.o 
client: Client/client.o
	gcc -Wall Client/client.c -o Client/client.o 
clean: 
	rm -f Server/server Server/server.o Client/client Client/client.o
