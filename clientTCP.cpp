#include "clientTCP.h"
#include "elementTCP.h"
#include "readline.h"

ClientConnectToServer::ClientConnectToServer() {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    flag = fcntl(socketfd, F_GETFL, 0);
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
    Packet packet2 = packet.json_to_packet(j);
    return packet2;
}

void ClientConnectToServer::sendData(Packet& packet) {
    std::string data = serializer(packet);
    data += "\n";
    if (send(socketfd, data.c_str(), data.size(), 0) == -1) {
        perror("Failed to send data");
    }
}

Packet ClientConnectToServer::receiveData() {
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int returncode = readline(socketfd, buffer, MAXLINE);
    if ( returncode == -1) {
        perror("Failed to receive data");
    }else if(returncode == 0){
        std::cout << "server disconnected" << std::endl;
        close(socketfd);
        exit(0);
    }
    std::string data = buffer;
    try {
        Packet packet = deserializer(data);
        return packet;
    } catch (const json::exception& e) {
        std::cerr << "packet 反序列化出錯了！: " << e.what() << std::endl;
        return Packet{};
    }
}

void ClientConnectToServer::turnOnNonBlock() {
    flag |= O_NONBLOCK;
    if(fcntl(socketfd, F_SETFL, flag) < 0) {
        perror("fcntl F_SETFL error");
    }
}

void ClientConnectToServer::turnOffNonBlock() {
    flag &= ~O_NONBLOCK;
    if(fcntl(socketfd, F_SETFL, flag) < 0) {
        perror("fcntl F_SETFL error");
    }
}

Packet ClientConnectToServer::receiveDataNonBlock() {
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int returncode = readline(socketfd, buffer, MAXLINE);
    if (returncode == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            return Packet();
        }
        perror("Failed to receive data");
    }else if(returncode == 0){
        std::cout << "server disconnected" << std::endl;
        close(socketfd);
        exit(0);
    }
    std::string data = buffer;
    try {
        Packet packet = deserializer(data);
        return packet;
    } catch (const json::exception& e) {
        std::cerr << "packet 反序列化出錯了！: " << e.what() << std::endl;
        return Packet{};
    }
}

