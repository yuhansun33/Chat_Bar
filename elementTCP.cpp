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
    try {
        Packet p;
        j.at("mode_packet").get_to(p.mode_packet);
        j.at("sender_name").get_to(p.sender_name);
        j.at("receiver_name").get_to(p.receiver_name);
        j.at("x_packet").get_to(p.x_packet);
        j.at("y_packet").get_to(p.y_packet);
        j.at("message").get_to(p.message);
        return p;
    } catch (const json::exception& e) {
        std::cerr << "JSON 反序列化出錯了！: " << e.what() << std::endl;
        return Packet{};
    }
}
