#include	"unp.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
using namespace std;

void sig_chld(int signo){
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}
struct Packet{
	char type_packet[MAXLINE]; //map , chat
	char name_packet[MAXLINE]; //sender name
	float x_packet;
	float y_packet;
	char message[MAXLINE];
};
struct player{
	int sockfd;
    char name_player[MAXLINE];
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
    vector<struct player> players;

};

Game::add_player(struct player new_player){ players.push_back(new_player); }
Game::remove_player(int sockfd){
    for(int i = 0; i < players.size(); i++){
        if(players[i].sockfd == sockfd){
            players.erase(players.begin() + i);
            break;
        }
    }
}
Game::get_player_size(){ return players.size(); }
Game::serialize(Packet packet, char* buffer){ memcpy(buffer, &packet, sizeof(Packet)); }
Game::deserialize(Packet packet, char* buffer){ memcpy(&packet, buffer, sizeof(Packet)); }
Game::broadcast_xy(Packet packet, int sockfd){
    char buffer[MAXLINE];
    for(int i = 0; i < players.size(); i++){
        if(players[i].sockfd != sockfd){
            //send
            serialize(packet, buffer);
            Write(players[i].sockfd, buffer, sizeof(Packet));
        }
    }
}
Game::handle_client(int sockfd){
    int n;
    char buffer[MAXLINE];

    while(true){
        n = Read(sockfd, buffer, MAXLINE);
        Packet packet;
        deserialize(packet, buffer);

        Packet new_packet;
        new_packet.type_packet = packet.type_packet;

        if(strcmp(packet.type_packet, "map") == 0){
            //send map
            new_packet.x_packet = packet.x_packet;
            new_packet.y_packet = packet.y_packet;
            broadcast_xy(new_packet, sockfd);
        }
        else if(strcmp(packet.type_packet, "chat") == 0){
            //send chat


            //如何聊天結束???
            
        }
        else{
            //error
        }
    }
}

int main() {
    int                 n;
	int			        listenfd, connfd, sockfd;
	socklen_t		    clilen;
	struct sockaddr_in	cliaddr, servaddr;
    char                recvline[MAXLINE], sendline[MAXLINE];
    time_t			    ticks;
    FILE			    *fp;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT + 3);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);

    Game game;

    while(true){
        clilen = sizeof(cliaddr);
        //accept
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                    continue;
            else
                    err_sys("accept error");
        }
        //讀 ID
        n = Read(connfd, recvline[index], MAXLINE);
        //放入 vector
        Player new_player;
        new_player.sockfd = connfd;
        new_player.x = 0.0f;
        new_player.y = 0.0f;
        new_player.name = recvline;
        game.add_player(new_player);
        //fork
        if ( (childpid = Fork()) == 0) {
            Close(listenfd);
            //handle_client
            game.handle_client(connfd);
            exit(0);
        }
        close(connfd);
    }

    return 0;
}
