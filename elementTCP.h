#ifndef elementTCP_h
#define elementTCP_h

#include "header.h"

using json = nlohmann::json;

struct Packet{
	Packet(){mode_packet = EMPTYMODE;};
	Packet(int mode, std::string sendstr = NULL, std::string recvstr = NULL, float x = 0, float y = 0, std::string msg = NULL);
	json packet_to_json();
	Packet json_to_packet(json& json_packet);
	int mode_packet; //MAP, REQUEST, CHAT
	char sender_name[NAMELINE]; //sender name
    char receiver_name[NAMELINE]; //receiver name
	float x_packet;
	float y_packet;
	char message[MAXLINE];
};

struct Player{
	Player(){};
	Player(int sockfd, int mode, float x, float y);
	int sockfd;
    int mode_player; //MAP , CHAT
	float x_player;
	float y_player;
};

#endif
