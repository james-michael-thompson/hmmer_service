CC=g++
CFLAGS=-I/usr/local/include -c -Wall -pedantic
LDFLAGS=-L/usr/local/lib -lprotobuf -lgflags -lglog -lzmq -g0 -O3

proto :
	protoc --cpp_out=. hmmer.proto

hmmer.pb.o: proto hmmer.pb.cc
	$(CC) $(CFLAGS) hmmer.pb.cc

hmmer_client.o: hmmer.pb.o hmmer_client.cc
	$(CC) $(CFLAGS) hmmer_client.cc

hmmer_server.o: hmmer.pb.o hmmer_server.cc
	$(CC) $(CFLAGS) hmmer_server.cc

hmmer_client: hmmer_client.o
	$(CC) $(LDFLAGS) hmmer.pb.o hmmer_client.o -g0 -O3 -o hmmer_client

hmmer_server: hmmer_server.o
	$(CC) $(LDFLAGS) hmmer.pb.o hmmer_server.o -g0 -O3 -o hmmer_server

all: hmmer_client hmmer_server
	

clean:
	rm -f hmmer.pb.* *.o hmmer_client hmmer_server shell_test

shell_test: shell.cc
	$(CC) shell.cc -o shell_test

