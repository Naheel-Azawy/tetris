PREFIX = /usr/bin/
CC = gcc

SRC = tetris.c
HDR = tetris.h
SRC_TUI = tetris_tui.c
SRC_INO = tetris_arduino.ino
SRC_WASM = tetris_wasm.c

all: build/tetris build/tetris-all.c build/tetris.ino tetris_wasm.wasm

build/tetris: $(SRC) $(HDR) $(SRC_TUI)
	mkdir -p build
	$(CC) $(SRC) $(SRC_TUI) -lpthread -o ./build/tetris

build/tetris-all.c: $(SRC) $(HDR) $(SRC_TUI)
	mkdir -p build
	cat $(HDR) $(SRC) $(SRC_TUI) > ./build/tetris-all.c
	sed -i 's/#include "tetris.h"//' ./build/tetris-all.c

tetris_wasm.wasm: $(SRC) $(HDR) $(SRC_WASM)
	mkdir -p build
	cat $(HDR) $(SRC) $(SRC_WASM) > ./build/tetris-all-wasm.c
	sed -i 's/#include "tetris.h"//' ./build/tetris-all-wasm.c
	emcc -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS="['_setup', '_loop', '_get_data_len']" -Wl,--no-entry ./build/tetris-all-wasm.c -o ./tetris_wasm.wasm

build/tetris.ino: $(SRC) $(HDR) $(SRC_INO)
	mkdir -p build
	cat $(HDR) $(SRC) $(SRC_INO) > ./build/tetris.ino
	sed -i 's/#include "tetris.h"//' ./build/tetris.ino

clean:
	rm -rf ./build

install:
	mkdir -p $(DESTDIR)$(PREFIX)
	cp -f ./build/tetris $(DESTDIR)$(PREFIX)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/tetris
