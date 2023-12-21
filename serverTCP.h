#ifndef serverTCP_h
#define serverTCP_h
#include "header.h"
#include "elementTCP.h"
class serverTCP{
//share
public:
    serverTCP(){};
    static void sig_chld(int signo);
    void TCP_connect(int port);
    void accept_client();
//login server
    void login_mainloop();
    void login_handle();
//game server
    void game_mainloop();
    void new_game_handle();
    void game_handle();
    std::string serialize(Packet packet);
    Packet deserialize(std::string& json_string);
    void sendData(Packet packet, int sockfd);
    Packet receiveData(int sockfd);
    void broadcast_xy(Packet packet, int sockfd);
    
private:
	int			        n, listenfd, connfd, sockfd;
	socklen_t		    clilen;
	struct sockaddr_in	cliaddr, servaddr;
    int                 maxfd;
    fd_set              rset, allset;

    std::unordered_map<std::string, struct Player> players;
};

class PlayerList{
    friend class serverTCP;
public:
    PlayerList(){};
    void add_player(char* name, struct Player new_player);
    void remove_player(char* name);
    int  get_player_size();
    std::unordered_map<std::string, struct Player> get_players_map();
    void show_players();
    int get_player_sockfd(char* name);
private:
    std::unordered_map<std::string, struct Player> players;
};

#endif