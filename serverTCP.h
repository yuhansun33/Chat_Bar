#ifndef serverTCP_h
#define serverTCP_h
#include "header.h"
#include "elementTCP.h"
class serverTCP{
public:
    serverTCP(){};
    void add_player(char* name, struct Player new_player);
    void remove_player(char* name);
    int  get_player_size();
    std::string serialize(Packet packet);
    Packet deserialize(std::string& json_string);
    void sendData(Packet packet, int sockfd);
    Packet receiveData(int sockfd);
    void broadcast_xy(Packet packet, int sockfd);
    std::unordered_map<std::string, struct Player> get_players_map();
    void show_players();
    int get_player_sockfd(char* name);
private:
    std::unordered_map<std::string, struct Player> players;
};

#endif