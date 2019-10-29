all: daytime_server tls_server use-dlopen hello.so

daytime_serve: daytime_server.c
	$(CC) -o $@ $^

tls_server: tls_server.c
	$(CC) `pkg-config --cflags openssl` -o $@ $^ `pkg-config --libs openssl`
	@echo "Don't forget to create cert.pem and key.pem"

use-dlopen: use-dlopen.c
	$(CC) -o $@ $^ -ldl

hello.so: hello.c
	$(CC) -c -fPIC $^
	ld -G -o hello.so hello.o

clean:
	rm -f tls_server daytime_server use-dlopen hello.so hello.o
