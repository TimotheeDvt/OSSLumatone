.PHONY: all configure build run clean c b r ccb

all: configure build

# Generates the Ninja build files
configure:
	cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja

# Compiles the project using the generated build files
build: configure
	cmake --build build --config Release -j4

# Builds then launches the editor
run: build
	"./build/OSSLumatoneEditor_artefacts/Release/OSSLumatone Editor"

# Cleans the build directory
clean:
	rm -rf build

# Shorthand aliases
c: configure
b: build
r: run
ccb: clean configure build
