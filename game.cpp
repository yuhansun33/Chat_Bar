#include "game.h"

void Game::add_player(char* name, struct Player new_player){ players[name] = new_player; }
void Game::remove_player(char* name){ players.erase(name); }
int Game::get_player_size(){ return players.size(); }
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