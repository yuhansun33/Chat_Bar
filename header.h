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
#include <SFML/Audio.hpp>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


#define EMPTYMODE       -1
#define MAPMODE         0
#define CHATMODE        1
#define REQMODE         2
#define INITMODE        3
#define LOGINMODE       4
#define REGISTERMODE    5
#define ESCMODE         6
#define TIMEMODE        7
#define ROOMMODE        8
#define RANKMODE        9
#define JOINMODE        10
#define FRIENDMODE      11

#define FRI_REQ_UNQULIF   0
#define FRI_REQ_PENDING   1
#define FRI_REQ_KEYDENY   2
#define FRI_REQ_KEYSENT   3
#define FRI_REQ_REPDENY   5


#define REG_SUCCESS    0
#define REG_REPEAT     1
#define REG_FAIL       2

#define CHATSTATENONE  0
#define CHATSTATERECV  1
#define CHATSTATESEND  2
#define CHATSTATECHAT  3

#define VIRTUALFD           -500
#define NOWHERE             -500         
#define CHATDISTANCE        100
#define INFINDISTANCE       1000000
#define GAMEPORT            11130
#define LOGINPORT           11131
#define NAMELINE            128
#define MAXLINE             4096
#define LISTENQ             1024
#define MOVEDISTANCE        37
#define SA                  struct sockaddr
#define SERVERIP            "25.17.111.88"
#define SQLPASSWD           "eee3228133@"

#endif