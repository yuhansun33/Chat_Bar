#include "header.h"
#include "game.h"
using namespace std;

void sig_chld(int signo){
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
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
	servaddr.sin_port        = htons(GAMEPORT);

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    signal(SIGCHLD, sig_chld);

    maxfd = listenfd;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    Game game;

    while(true){
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            clilen = sizeof(cliaddr);
            //accept
            if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
                if (errno == EINTR){
                    continue;
                }else{
                    perror("accept error");
                }
            }
            //init packet
            Packet init_packet(INITMODE, "", "", (float)game.get_player_size(), 0, "");
            game.sendData(init_packet, connfd);
            //送所有人位置
            for (auto& player : game.get_players_map()){
                Packet packet(MAPMODE, player.first, "", player.second.x_player, player.second.y_player, "");
                game.sendData(packet, connfd);
            }
            //讀 ID
            Packet packet = game.receiveData(connfd);
            //放入 vector
            Player new_player(connfd, MAPMODE, packet.x_packet, packet.y_packet);
            game.add_player(packet.sender_name, new_player);
            
            FD_SET(connfd, &allset);
            if(connfd > maxfd) maxfd = connfd;
        }
        cout << "player num : " << (float)game.get_player_size() << endl;
        //看每個 client
        for (const auto& player : game.get_players_map() ){
            sockfd = player.second.sockfd;
            // cout << "2)sockfd: " << sockfd << "player name: " << player.first << endl;
            if(FD_ISSET(sockfd, &rset)){
                //handle_client
                Packet packet = game.receiveData(sockfd);
                cout << "sender: " << packet.sender_name << " (" << packet.x_packet << ", " << packet.y_packet << ")" << endl;
                if(packet.mode_packet == MAPMODE){
                    //map mode
                    Packet new_packet(MAPMODE, packet.sender_name, "", packet.x_packet, packet.y_packet, "");
                    game.broadcast_xy(new_packet, sockfd);
                }else if(packet.mode_packet == REQMODE){
                    //request mode
                    int receiver_sockfd = game.get_player_sockfd(packet.receiver_name);
                    if(packet.message == "1st request\n"){
                        Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Connect?\n");
                        game.sendData(new_packet, receiver_sockfd);
                    }else if(packet.message == "Yes\n"){
                        Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Can connect.\n");
                        game.sendData(new_packet, receiver_sockfd);
                    }else if(packet.message == "No\n"){
                        Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Can't connect.\n");
                        game.sendData(new_packet, receiver_sockfd);
                    }
                }
            }
        }


    }

    return 0;
}
