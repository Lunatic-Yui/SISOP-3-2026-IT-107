all: server client

server: wired.c protocol.h
	gcc -Wall wired.c -o wired

client: navi.c protocol.h
	gcc -Wall navi.c -o navi -lpthread

clean:
	rm -f wired navi history.log
