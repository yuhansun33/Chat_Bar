#include "elementTCP.h"
#include "serverTCP.h"
#include "readline.h"

void serverTCP::add_player(char* name, struct Player new_player){
    std::string name_str(name);
    players[name_str] = new_player;
}
void serverTCP::remove_player(char* name){ players.erase(name); }
int  serverTCP::get_player_size(){ return players.size(); }
std::string serverTCP::serialize(Packet packet){
    std::string data = packet.packet_to_json().dump();
    return data;
}
Packet serverTCP::deserialize(std::string& json_string){
    Packet packet;
    json j = json::parse(json_string);
    packet = packet.json_to_packet(j);
    return packet;
}
void serverTCP::sendData(Packet packet, int sockfd){
    std::string data = serialize(packet);
    data += "\n";
    if (send(sockfd, data.c_str(), data.size(), 0) == -1) {
        perror("Failed to send data");
    }
}
Packet serverTCP::receiveData(int sockfd){
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int n = Readline(sockfd, buffer, MAXLINE);
    if (n == -1) {
        std::cout << "(server) Failed to receive data" << std::endl;
        perror("Failed to receive data");
    }else if(n == 0){
        std::cout << "client disconnected" << std::endl;
        close(sockfd);
        exit(0);
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void serverTCP::broadcast_xy(Packet packet, int sockfd){
    char buffer[MAXLINE];
    for (auto& player : players){
        if(player.second.sockfd != sockfd){
            //send
            sendData(packet, player.second.sockfd);
        }
    }
}
std::unordered_map<std::string, struct Player> serverTCP::get_players_map(){ return players; }
void serverTCP::show_players(){
    std::cout << "name\tsockfd\tmode\tx\ty" << std::endl;
    for (auto& player : players){
        std::cout << player.first << "\t";
        std::cout << player.second.sockfd << "\t";
        std::cout << player.second.mode_player << "\t";
        std::cout << player.second.x_player << "\t";
        std::cout << player.second.y_player << std::endl;
    }
}
int serverTCP::get_player_sockfd(char* name){
    std::string name_str(name);
    return players[name_str].sockfd;
}