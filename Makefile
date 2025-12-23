.PHONY: all build install uninstall clean

all: build

build:
	cmake -S . -B build
	cmake --build build

install: build
	sudo cmake --install build

uninstall:
	sudo cmake --build build --target uninstall

clean:
	rm -rf build