objects := $(patsubst %.c,%.o,$(wildcard *.c))
includes := $(wildcard *.h)

libstubs.a: server_stub.o client_stub.o helper.o mybind.o
	ar r libstubs.a server_stub.o client_stub.o helper.o mybind.o

$(objects): %.o: %.c $(includes)
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a *_app
