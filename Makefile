SERVER_FILE=server.cpp
CLIENT_FILE=client.cpp
SOURCE_FILE=CASH.hpp
COMPILER_ARGS=-lssl -lcrypto -pthread -Wall
ifeq ($(OS),Windows_NT)
EXTENITION_EXE_FILE=.exe
EXTENITION_DLL_FILE=.dll
else
EXTENITION_EXE_FILE=
EXTENITION_DLL_FILE=.so
endif

build:
	mkdir -p build
	g++ $(SERVER_FILE) -o build/server $(EXTENITION_EXE_FILE) $(COMPILER_ARGS)
	g++ $(SOURCE_FILE) -shared -o build/CASH $(EXTENITION_DLL_FILE)