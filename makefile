compile:
	@rm -rf ./build/*

	@node preprocess.js

	@clang \
		-Wall -Wextra -Wpedantic \
		-Wno-unused-parameter -Wno-strict-prototypes \
		--target=wasm32 -nostdlib -fno-builtin \
		-matomics -mbulk-memory \
		-O3 -ffast-math -flto \
		-Wl,--lto-O3 -Wl,--error-limit=0 \
		-Wl,--no-entry -Wl,--export-dynamic \
		-Wl,--import-memory -Wl,--shared-memory -Wl,--initial-memory=98304000 -Wl,--max-memory=98304000 \
		-Wl,-z,stack-size=65536 \
		-o build/game.wasm \
		$(shell find ./src/c -name '*.S') $(shell find ./src/c -name '*.c')

	@npx tsc
	@cp -r ./src/static/* ./build
