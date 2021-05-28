PREFIX = /usr/bin/
CC = gcc

LICENSE  = LICENSE_HEADER
SRC      = tetris.c
HDR      = tetris.h
SRC_TUI  = tetris_tui.c
SRC_INO  = tetris_arduino.ino
SRC_WASM = tetris_wasm.c

all:     tui tui-all ino web
tui:     build/tetris
tui-all: build/tetris-all.c
ino:     build/tetris.ino
web:     index.html

build/tetris: $(SRC) $(HDR) $(SRC_TUI)
	mkdir -p build
	$(CC) $(SRC) $(SRC_TUI) -lpthread -o ./build/tetris

build/tetris-all.c: $(SRC) $(HDR) $(SRC_TUI)
	mkdir -p build
	cat $(LICENSE) $(HDR) $(SRC) $(SRC_TUI) > ./build/tetris-all.c
	sed -i 's/#include "tetris.h"//' ./build/tetris-all.c

./build/tetris_wasm.wasm: $(SRC) $(HDR) $(SRC_WASM)
	mkdir -p build
	cat $(LICENSE) $(HDR) $(SRC) $(SRC_WASM) > ./build/tetris_wasm_all.c
	sed -i 's/#include "tetris.h"//' ./build/tetris_wasm_all.c
	emcc -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS="['_setup', '_loop', '_get_data_len']" -Wl,--no-entry ./build/tetris_wasm_all.c -o ./build/tetris_wasm.wasm

index.html: ./build/tetris_wasm.wasm tetris_wasm_ui.js tetris_wasm_ui.html
	cat $(LICENSE)                    > ./build/tetris_wasm_all.js
	echo 'const tetris_asm = atob(`' >> ./build/tetris_wasm_all.js
	base64 ./build/tetris_wasm.wasm  >> ./build/tetris_wasm_all.js
	echo '`);'                       >> ./build/tetris_wasm_all.js
	cat tetris_wasm_ui.js            >> ./build/tetris_wasm_all.js
	sed -e '/SCRIPT/ {r ./build/tetris_wasm_all.js' -e 'd}' tetris_wasm_ui.html > index.html

build/tetris.ino: $(SRC) $(HDR) $(SRC_INO)
	mkdir -p build
	cat $(LICENSE) $(HDR) $(SRC) $(SRC_INO) > ./build/tetris.ino
	sed -i 's/#include "tetris.h"//' ./build/tetris.ino

clean:
	rm -rf ./build

install:
	mkdir -p $(DESTDIR)$(PREFIX)
	cp -f ./build/tetris $(DESTDIR)$(PREFIX)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/tetris
