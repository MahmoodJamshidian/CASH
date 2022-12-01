SERVER_FILE=server.cpp
CLIENT_FILE=client.cpp
SOURCE_FILE=CASH.hpp
COMPILER_ARGS=-lssl -lcrypto -pthread -Wall

build:
	mkdir -p build
	g++ $(SERVER_FILE) -o build/server $(COMPILER_ARGS)
	g++ $(SOURCE_FILE) -shared -o build/CASH.so