CC = g++
CFLAGS = -Wall
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all: default

default: default.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)

client_move: client_move.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)

test: test.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)

login: login.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)

server: server.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)


clean:
	rm -f test