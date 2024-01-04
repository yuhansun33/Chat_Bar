#ifndef elementTCP_h
#define elementTCP_h

#include "header.h"

using json = nlohmann::json;

struct Packet{
	Packet();
	Packet(int mode, std::string sendstr = "", std::string recvstr = "", float x = 0, float y = 0, std::string msg = "");
	json packet_to_json();
	Packet json_to_packet(json& json_packet);
	int mode_packet; 
	char sender_name[NAMELINE]; 
    char receiver_name[NAMELINE]; 
	float x_packet;
	float y_packet;
	char message[MAXLINE];
	void printPacket();
};

struct Player{
	Player(){};
	Player(int sockfd, int mode, float x, float y, char* ID);
	std::string playerID;
	int sockfd;
	int roomID;
    int mode_player; //MAP , CHAT
	float x_player;
	float y_player;
};

#endif
