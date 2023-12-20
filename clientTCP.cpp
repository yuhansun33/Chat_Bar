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

std::string ClientConnectToServer::serializer(Packet& packet) {
    std::string data = packet.packet_to_json().dump();
    return data;
}

Packet ClientConnectToServer::deserializer(std::string& json_string) {
    Packet packet;
    json j = json::parse(json_string);
    packet = packet.json_to_packet(j);
    return packet;
}

void ClientConnectToServer::sendData(Packet& packet) {
    std::string data = serializer(packet);
    if (send(socketfd, data.c_str(), data.size(), 0) == -1) {
        perror("Failed to send data");
    }
}

Packet ClientConnectToServer::receiveData() {
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int returncode = recv(socketfd, buffer, MAXLINE, 0);
    if ( returncode == -1) {
        perror("Failed to receive data");
    }else if(returncode == 0){
        std::cout << "server disconnected" << std::endl;
        close(socketfd);
        exit(0);
    }
    std::string data = buffer;
    Packet packet = deserializer(data);
    return packet;
}


