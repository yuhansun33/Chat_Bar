#ifndef header_h
#define header_h
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define MAXMODE     0
#define CHATMODE    1
#define REQMODE     2
#define GAMEPORT    11130
#define MAXLINE     4096
#define LISTENQ     1024
#define SA          struct sockaddr
#define SERVERIP    "127.0.0.1"
#endif