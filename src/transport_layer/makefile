all: build build/tx build/rx

build:
	mkdir -p build

COMMON_SRC := ../network_utils/serial.c ../network_utils/adapter.c

build/tx: tx/main.c $(COMMON_SRC)
	gcc tx/main.c $(COMMON_SRC) -o build/tx

build/rx: rx/main.c $(COMMON_SRC)
	gcc rx/main.c $(COMMON_SRC) -o build/rx