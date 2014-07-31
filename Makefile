OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
includes := $(wildcard *.h)

all: make_rpc server

make_rpc:
	$(MAKE) -C simplified_rpc

server: client-api.a server.o
	gcc server.o -L. -lclient-api -o fsServer 

$(OBJECTS): %.o: %.c ece454_fs.h simplified_rpc/ece454rpc_types.h
	gcc -g -c $< -o $@	
	

client-api.a: simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o fs_manager.o
	ar r libclient-api.a simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o fs_manager.o

clean:
	rm -rf a.out *.o core *.a fs_client *_app server fsclient fsclient_test
	$(MAKE) -C simplified_rpc clean
