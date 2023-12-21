#include "elementTCP.h"

Packet::Packet(int mode, std::string sendstr, std::string recvstr, float x, float y, std::string msg){
    mode_packet = mode;
    x_packet = x;
    y_packet = y;
    size_t sender_size = strlen(sendstr.c_str());
    size_t receiver_size = strlen(recvstr.c_str());
    size_t message_size = strlen(msg.c_str());
    strncpy(sender_name, sendstr.c_str(), sender_size);
    strncpy(receiver_name, recvstr.c_str(), receiver_size);
    strncpy(message, msg.c_str(), message_size);
    sender_name[sender_size] = '\0';
    receiver_name[receiver_size] = '\0';
    message[message_size] = '\0';    
}

json Packet::packet_to_json() {
    try {
        json j;
        j["mode_packet"] = mode_packet;
        j["sender_name"] = sender_name;
        j["receiver_name"] = receiver_name;
        j["x_packet"] = x_packet;
        j["y_packet"] = y_packet;
        j["message"] = message;
        return j;
    } catch (const json::exception& e) {
        std::cerr << "JSON 序列化出錯了！: " << e.what() << std::endl;
        return json{};
    }
}

void Packet::printPacket(){
    std::cout << "mode: " << mode_packet << std::endl;
    std::cout << "sender: " << sender_name << std::endl;
    std::cout << "receiver: " << receiver_name << std::endl;
    std::cout << "x: " << x_packet << std::endl;
    std::cout << "y: " << y_packet << std::endl;
    std::cout << "message: " << message << std::endl;
}


Packet Packet::json_to_packet(json& j) {
    Packet p;
    // 進行賦值
    try {
        p.mode_packet = j["mode_packet"];
    } catch (const json::exception& e) {
        std::cerr <<  "mode_packet 反序列化出錯了！: " << e.what() << std::endl;
    }
    try {
        strcpy(p.sender_name, j["sender_name"].get<std::string>().c_str());
    } catch (const json::exception& e) {
        std::cerr <<  "sendname 反序列化出錯了！: " << e.what() << std::endl;
    }
    try {
        strcpy(p.receiver_name, j["receiver_name"].get<std::string>().c_str());
    } catch (const json::exception& e) {
        std::cerr <<  "recvname 反序列化出錯了！: " << e.what() << std::endl;
    }
    try {
        p.x_packet = j["x_packet"];
    } catch (const json::exception& e) {
        std::cerr <<  "x_packet 反序列化出錯了！: " << e.what() << std::endl;
    }
    try {
        p.y_packet = j["y_packet"];
    } catch (const json::exception& e) {
        std::cerr <<  "y_packet 反序列化出錯了！: " << e.what() << std::endl;
    }
    try {
        strcpy(p.message, j["message"].get<std::string>().c_str());
    } catch (const json::exception& e) {
        std::cerr <<  "message 反序列化出錯了！: " << e.what() << std::endl;
    }
    return p;
}

Player::Player(int new_sockfd, int mode, float x, float y){
    sockfd = new_sockfd;
    mode_player = mode;
    x_player = x;
    y_player = y;
}
