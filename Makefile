OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
includes := $(wildcard *.h)

all: make_rpc server_app fs_client fs_client_test

make_rpc:
	$(MAKE) -C simplified_rpc

server_app: libstubs.a server.o
	gcc server.o -L. -lstubs -o server

fs_client: libstubs.a fs_client.o
	gcc fs_client.o -L. -lstubs -o fsclient

fs_client_test: libstubs.a fs_client_test.o
	gcc fs_client_test.o -L. -lstubs -o fsclient_test

$(OBJECTS): %.o: %.c ece454_fs.h simplified_rpc/ece454rpc_types.h
	gcc -g -c $< -o $@	
	

libstubs.a: simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o fs_manager.o
	ar r libstubs.a simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o fs_manager.o

clean:
	rm -rf a.out *.o core *.a fs_client *_app server fsclient fsclient_test
	$(MAKE) -C simplified_rpc clean
