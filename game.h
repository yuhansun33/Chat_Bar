#ifndef game_h
#define game_h
#include "header.h"
#include "elementTCP.h"
class Game{
public:
    Game();
    void add_player(char* name, struct Player new_player);
    void remove_player(char* name);
    int  get_player_size();
    std::string serialize(Packet packet);
    Packet deserialize(std::string& json_string);
    void sendData(Packet packet, int sockfd);
    Packet receiveData(int sockfd);
    void broadcast_xy(Packet packet, int sockfd);
    // void handle_client(int sockfd);
    std::unordered_map<char*, struct Player> get_players_map();
private:
    std::unordered_map<char*, struct Player> players;
};

#endif