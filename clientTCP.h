#ifndef CLIENT_TCP_H
#define CLIENT_TCP_H
#include "header.h"
#include "elementTCP.h"

class ClientConnectToServer {
    private:
        int socketfd;
        struct sockaddr_in serverAddr;
    public:
        ClientConnectToServer();
        ~ClientConnectToServer();
        void serverIPPort(const char* serverIP, int port);
        void sendData(Packet& data);
        Packet receiveData();
};

#endif 
