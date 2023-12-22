#ifndef header_h
#define header_h

#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdexcept> 
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <unordered_map>
#include <mysql_driver.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


#define EMPTYMODE    -1
#define MAPMODE      0
#define CHATMODE     1
#define REQMODE      2
#define INITMODE     3
#define LOGINMODE    4
#define REGISTERMODE 5

#define CHATDISTANCE        100
#define GAMEPORT            11130
#define LOGINPORT           11131
#define NAMELINE            128
#define MAXLINE             4096
#define LISTENQ             1024
#define SA                  struct sockaddr
#define SERVERIP            "127.0.0.1"
#define SQLPASSWD           "eee3228133@"

#endif