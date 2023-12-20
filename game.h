#ifndef GAME_H
#define GAME_H
#include "header.h"
class Game{
public:
    Game();
    void add_player(char* name, struct Player new_player);
    void remove_player(char* name);
    void get_player_size();
    void serialize(Packet packet, char* buffer);
    void deserialize(Packet packet, char* buffer);
    void broadcast_xy(Packet packet, int sockfd);
    void handle_client(int sockfd);
private:
    unordered_map<const char*, struct Player> players;
};

void Game::add_player(char* name, struct Player new_player){ players[name] = new_player; }
void Game::remove_player(char* name){ players.erase(name); }
void Game::get_player_size(){ return players.size(); }
void Game::serialize(Packet packet, char* buffer){ memcpy(buffer, &packet, sizeof(Packet)); }
void Game::deserialize(Packet packet, char* buffer){ memcpy(&packet, buffer, sizeof(Packet)); }
void Game::broadcast_xy(Packet packet, int sockfd){
    char buffer[MAXLINE];
    for (auto& player : players){
        if(player.second.sockfd != sockfd){
            //send
            serialize(packet, buffer);
            write(player.second.sockfd, buffer, sizeof(Packet));
        }
    }
}

#endif