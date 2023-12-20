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
    void serialize(Packet packet, char* buffer);
    void deserialize(Packet packet, char* buffer);
    void broadcast_xy(Packet packet, int sockfd);
    void handle_client(int sockfd);
private:
    unordered_map<const char*, struct Player> players;
};

#endif