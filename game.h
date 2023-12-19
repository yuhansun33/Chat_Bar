#include "header.h"
#ifndef GAME_H
#define GAME_H

struct Packet{
	int mode_packet; //MAP, REQUEST, CHAT
	char sender_name[MAXLINE]; //sender name
    char receiver_name[MAXLINE]; //receiver name
	float x_packet;
	float y_packet;
	char message[MAXLINE];
};

struct Player{
	int sockfd;
    int mode_player; //MAP , CHAT
	float x_player;
	float y_player;
};

class Game{
public:
    Game();
    void add_player(struct player new_player);
    void remove_player(int sockfd);
    void get_player_size();
    void serialize(Packet packet, char* buffer);
    void deserialize(Packet packet, char* buffer);
    void broadcast_xy(Packet packet, int sockfd);
    void handle_client(int sockfd);
private:
    unorder_map<const char*, struct Player> players;
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