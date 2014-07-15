OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
includes := $(wildcard *.h)

all: sample_server_app fs_client

sample_server_app: libstubs.a sample_server_app.o
	gcc sample_server_app.o -L. -lstubs -o server

fs_client: libstubs.a fs_client.o fs_dummy.o
	gcc fs_client.o -L. -lstubs -o fsclient

$(OBJECTS): %.o: %.c ece454_fs.h ece454rpc_types.h
	gcc -c $< -o $@	
	

libstubs.a: server_stub.o client_stub.o helper.o mybind.o fs_dummy.o
	ar r libstubs.a server_stub.o client_stub.o helper.o mybind.o

clean:
	rm -rf a.out *.o core *.a fs_client *_app
