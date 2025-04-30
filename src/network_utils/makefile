all: build build/tx build/rx build/ping-client build/ping-server build/flood-client build/flood-server build/listen

COMMON_SRC :=	adapter.c	\
				serial.c	\

build:
	mkdir -p build

build/tx: tx.c $(COMMON_SRC)
	gcc tx.c $(COMMON_SRC) -o build/tx

build/rx: rx.c $(COMMON_SRC)
	gcc rx.c $(COMMON_SRC) -o build/rx

build/ping-client: ping_client.c $(COMMON_SRC)
	gcc ping_client.c $(COMMON_SRC) -o build/ping-client

build/ping-server: ping_server.c $(COMMON_SRC)
	gcc ping_server.c $(COMMON_SRC) -o build/ping-server

build/flood-client: flood_client.c $(COMMON_SRC)
	gcc flood_client.c $(COMMON_SRC) -o build/flood-client

build/flood-server: flood_server.c $(COMMON_SRC)
	gcc flood_server.c $(COMMON_SRC) -o build/flood-server

build/listen: listen.c $(COMMON_SRC)
	gcc listen.c $(COMMON_SRC) -o build/listen