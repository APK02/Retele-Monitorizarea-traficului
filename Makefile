all:
	g++ client.c -o client
	g++ server.c -o server -lsqlite3 -lpthread
clean:
	rm -f -v client server