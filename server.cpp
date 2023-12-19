#include	"unp.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <unordered_map>
using namespace std;

void sig_chld(int signo){
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}

struct Packet{
	int mode_packet; //MAP , REQUEST, CHAT
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
    unordered_map<const char*, struct Player> players;

};

Game::add_player(char* name, struct Player new_player){ players[name] = new_player; }
Game::remove_player(char* name){ players.erase(name); }
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
Game::handle_client(int sockfd, char* name){
    int n;
    char buffer[MAXLINE], sendline[MAXLINE];
    Player player = players[name];

    while(true){
        if(strcmp(player.mode_player, "MAP") == 0){
            n = Read(sockfd, buffer, MAXLINE);
            Packet packet;
            deserialize(packet, buffer);
            //轉移至 chat mode
            if(strcmp(packet.mode_packet, "REQUEST") == 0){
                if(strcmp(packet.message, "1st request\n") == 0){
                    snprintf(sendline, sizeof(sendline), "Do you want to connect? (Yes/No)\n");
                }else if(strcmp(packet.message, "Yes\n") == 0){
                    snprintf(sendline, sizeof(sendline), "Can connect.\n");
                }else if(strcmp(packet.message, "No\n") == 0){
                    snprintf(sendline, sizeof(sendline), "Can't connect.\n");
                }
                Write(players[packet.receiver_name].sockfd, sendline, strlen(sendline));
                continue;
            }
            //map mode
            Packet new_packet;
            new_packet.mode_packet = packet.mode_packet;

            new_packet.x_packet = packet.x_packet;
            new_packet.y_packet = packet.y_packet;
            broadcast_xy(new_packet, sockfd);
        }else if(player.mode_player == CHAT_MODE){
            n = Read(sockfd, buffer, MAXLINE);
            Packet packet;
            deserialize(packet, buffer);
            //轉移至 map mode
            if() player.mode_player = MAP_MODE;
            //chat mode

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

    int maxfd;
    fd_set rset, allset;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(11130);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);

    maxfd = listenfd;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    Game game;

    while(true){
        rset = allset;
        n = Select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            clilen = sizeof(cliaddr);
            //accept
            if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
                if (errno == EINTR)
                        continue;
                else
                        err_sys("accept error");
            }
            //讀 ID
            //name in recvline
            n = Read(connfd, recvline, MAXLINE);
            //放入 vector
            Player new_player;
            new_player.sockfd = connfd;
            new_player.mode_player = "map";
            new_player.x = 0.0f;
            new_player.y = 0.0f;
            game.add_player(recvline, new_player);
            /
        }



    }

    return 0;
}
