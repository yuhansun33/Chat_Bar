#include "elementTCP.h"
#include "game.h"


void Game::add_player(char* name, struct Player new_player){ players[name] = new_player; }
void Game::remove_player(char* name){ players.erase(name); }
int  Game::get_player_size(){ return players.size(); }
std::string Game::serialize(Packet packet){
    std::string data = packet.packet_to_json().dump();
    return data;
}
Packet Game::deserialize(std::string& json_string){
    Packet packet;
    json j = json::parse(json_string);
    packet = packet.json_to_packet(j);
    return packet;
}
void Game::sendData(Packet packet, int sockfd){
    std::string data = serialize(packet);
    write(sockfd, data.c_str(), data.size());
}
Packet Game::receiveData(int sockfd){
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    read(sockfd, buffer, MAXLINE);
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void Game::broadcast_xy(Packet packet, int sockfd){
    char buffer[MAXLINE];
    for (auto& player : players){
        if(player.second.sockfd != sockfd){
            //send
            serialize(packet);
            write(player.second.sockfd, buffer, sizeof(Packet));
        }
    }
}
std::unordered_map<char*, struct Player> Game::get_players_map(){ return players; }