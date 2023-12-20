#include "elementTCP.h"

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


Packet Packet::json_to_packet(json& j) {

    Packet p;
    // 進行賦值
    p.mode_packet = j["mode_packet"];
    strcpy(p.sender_name, j["sender_name"].get<std::string>().c_str());
    strcpy(p.receiver_name, j["receiver_name"].get<std::string>().c_str());
    p.x_packet = j["x_packet"];
    p.y_packet = j["y_packet"];
    strcpy(p.message, j["message"].get<std::string>().c_str());
    return p;

}

