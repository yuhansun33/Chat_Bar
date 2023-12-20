#ifndef elementTCP_h
#define elementTCP_h

#include "header.h"

using json = nlohmann::json;

struct Packet{
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
	int sockfd;
    int mode_player; //MAP , CHAT
	float x_player;
	float y_player;
};

#endif
