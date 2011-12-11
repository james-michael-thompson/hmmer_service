CC=g++
CFLAGS=-I/work/tex/install_dig/include -I/usr/local/include -c -Wall
LDFLAGS=-L/work/tex/install_dig/lib -L/usr/local/lib -lprotobuf -lgflags -lglog -lzmq -g3 -O03 -O03 -O0

proto: hmmer.proto
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
	rm -f hmmer.pb.* *.o hmmer_client hmmer_server shell fork_exec_test

shell.o: shell.cc hmmer.pb.o
	$(CC) $(CFLAGS) shell.cc -o shell.o

shell: shell.o
	$(CC) $(LDFLAGS) hmmer.pb.o shell.o -g0 -O3 -o shell

fork: fork_exec.cc
	$(CC) fork_exec.cc -o fork_exec_test

