#include "header.h"
#include "game.h"
using namespace std;

void sig_chld(int signo){
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}

// Game::handle_client(int sockfd, char* name){
//     int n;
//     char buffer[MAXLINE], sendline[MAXLINE];
//     Player player = players[name];

//     while(true){
//         if(strcmp(player.mode_player, "MAP") == 0){
//             n = Read(sockfd, buffer, MAXLINE);
//             Packet packet;
//             deserialize(packet, buffer);
//             //轉移至 chat mode
//             if(strcmp(packet.mode_packet, "REQUEST") == 0){
//                 if(strcmp(packet.message, "1st request\n") == 0){
//                     snprintf(sendline, sizeof(sendline), "Do you want to connect? (Yes/No)\n");
//                 }else if(strcmp(packet.message, "Yes\n") == 0){
//                     snprintf(sendline, sizeof(sendline), "Can connect.\n");
//                 }else if(strcmp(packet.message, "No\n") == 0){
//                     snprintf(sendline, sizeof(sendline), "Can't connect.\n");
//                 }
//                 Write(players[packet.receiver_name].sockfd, sendline, strlen(sendline));
//                 continue;
//             }
//             //map mode
//             Packet new_packet;
//             new_packet.mode_packet = packet.mode_packet;

//             new_packet.x_packet = packet.x_packet;
//             new_packet.y_packet = packet.y_packet;
//             broadcast_xy(new_packet, sockfd);
//         }else if(player.mode_player == CHAT_MODE){
//             n = Read(sockfd, buffer, MAXLINE);
//             Packet packet;
//             deserialize(packet, buffer);
//             //轉移至 map mode
//             if() player.mode_player = MAP_MODE;
//             //chat mode

//         }
//     }
// }

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
            //送所有人位置
            for (auto& player : game.get_players_map()){
                Packet packet;
                packet.mode_packet = MAPMODE;
                // packet.sender_name = player.first;
                strncpy(packet.sender_name, player.first, sizeof(packet.sender_name) - 1);
                packet.sender_name[sizeof(packet.sender_name) - 1] = '\0';
                packet.x_packet = player.second.x_player;
                packet.y_packet = player.second.y_player;
                game.sendData(packet, connfd);
            }
            //讀 ID
            //name in recvline
            Packet packet = game.receiveData(connfd);
            cout<<"here\n";
            //放入 vector
            Player new_player;
            new_player.sockfd = connfd;
            new_player.mode_player = MAPMODE;
            new_player.x_player = packet.x_packet;
            new_player.y_player = packet.y_packet;
            packet.sender_name[sizeof(packet.sender_name) - 1] = '\0';
            cout << "new player: " << packet.sender_name << endl;
            cout << "(x, y) : (" << packet.x_packet << ", " << packet.y_packet << ")" << endl;
            game.add_player(packet.sender_name, new_player);

            FD_SET(connfd, &allset);
            if(connfd > maxfd) maxfd = connfd;
        }
        //看每個 client
        // int num_players = game.get_player_size();
        // for (const auto& player : game.get_players_map() ){
        //     sockfd = player.second.sockfd;
        //     if(FD_ISSET(sockfd, &rset)){
        //         //handle_client
                
        //     }
        // }


    }

    return 0;
}
