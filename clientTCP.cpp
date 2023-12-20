#include "clientTCP.h"
#include "elementTCP.h"

ClientConnectToServer::ClientConnectToServer() {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        perror("Failed to create socket");
    }
}

ClientConnectToServer::~ClientConnectToServer() {
    close(socketfd);
}

void ClientConnectToServer::serverIPPort(const char* serverIP, int port) {
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    if (connect(socketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Failed to connect to server");
    }
}

void ClientConnectToServer::sendData(Packet& packet) {
    std::string data = packet.packet_to_json().dump();
    // 接下來，使用 socket 發送 data 字串
}

Packet ClientConnectToServer::receiveData() {
    std::string data;
    Packet P;
    json j = json::parse(data);
    P = P.json_to_packet(j);
}


