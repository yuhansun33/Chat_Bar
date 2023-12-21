#include "elementTCP.h"
#include "serverTCP.h"
#include "readline.h"

//======= serverTCP
serverTCP::serverTCP(){
    signal(SIGCHLD, sig_chld);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
}
void serverTCP::sig_chld(int signo){
    pid_t   pid;
    int     stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}
void serverTCP::TCP_connect(int port){
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    
    maxfd = listenfd;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
}
void serverTCP::accept_client(){
    clilen = sizeof(cliaddr);
    if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
        if (errno != EINTR){ perror("accept error"); }
    }
}
void serverTCP::login_mainloop(){
    while (true) {
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            //accept
            accept_client();
        }
        //看每個 client
        for (auto& player : players){
            sockfd = player.second.sockfd;
            if(FD_ISSET(sockfd, &rset)){
                //handle every client
                login_handle();
            }
        }
    }
}
void serverTCP::game_mainloop(){
    while (true) {
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            //accept
            accept_client();
            //new client handle
            new_game_handle();
        }
        //看每個 client
        for (auto& player : players){
            sockfd = player.second.sockfd;
            if(FD_ISSET(sockfd, &rset)){
                //handle every client
                game_handle();
            }
        }
    }
}
void serverTCP::game_handle(){
    Packet packet = receiveData(sockfd);
    std::cout << "sender: " << packet.sender_name << " (" << packet.x_packet << ", " << packet.y_packet << ")" << std::endl;
    if(packet.mode_packet == MAPMODE){
        //map mode
        Packet new_packet(MAPMODE, packet.sender_name, "", packet.x_packet, packet.y_packet, "");
        broadcast_xy(new_packet, sockfd);
    }else if(packet.mode_packet == REQMODE){
        //request mode
        int receiver_sockfd = players[packet.receiver_name].sockfd;
        if(packet.message == "1st request\n"){
            Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Connect?\n");
            sendData(new_packet, receiver_sockfd);
        }else if(packet.message == "Yes\n"){
            Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Yes\n");
            sendData(new_packet, receiver_sockfd);
        }else if(packet.message == "No\n"){
            Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "No\n");
            sendData(new_packet, receiver_sockfd);
        }
    }

}
void serverTCP::new_game_handle(){
    //init packet
    Packet init_packet(INITMODE, "", "", (float)players.size(), 0, "");
    sendData(init_packet, connfd);
    //送所有人位置
    for (auto& player : players){
        Packet packet(MAPMODE, player.first, "", player.second.x_player, player.second.y_player, "");
        sendData(packet, connfd);
    }
    //讀 ID
    Packet packet = receiveData(connfd);
    //放入 vector
    Player new_player(connfd, MAPMODE, packet.x_packet, packet.y_packet);
    players[packet.sender_name] = new_player;
    
    FD_SET(connfd, &allset);
    if(connfd > maxfd) maxfd = connfd;
}
Packet serverTCP::deserialize(std::string& json_string){
    Packet packet;
    json j = json::parse(json_string);
    packet = packet.json_to_packet(j);
    return packet;
}
void serverTCP::sendData(Packet packet, int sockfd){
    std::string data = serialize(packet);
    data += "\n";
    if (send(sockfd, data.c_str(), data.size(), 0) == -1) {
        perror("Failed to send data");
    }
}
Packet serverTCP::receiveData(int sockfd){
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int n = Readline(sockfd, buffer, MAXLINE);
    if (n == -1) {
        std::cout << "(server) Failed to receive data" << std::endl;
        perror("Failed to receive data");
    }else if(n == 0){
        std::cout << "client disconnected" << std::endl;
        close(sockfd);
        exit(0);
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void serverTCP::broadcast_xy(Packet packet, int sockfd){
    char buffer[MAXLINE];
    for (auto& player : players){
        if(player.second.sockfd != sockfd){
            //send
            sendData(packet, player.second.sockfd);
        }
    }
}
//======= PlayerList
void PlayerList::add_player(char* name, struct Player new_player){
    std::string name_str(name);
    players[name_str] = new_player;
}
void PlayerList::remove_player(char* name){ players.erase(name); }
int  PlayerList::get_player_size(){ return players.size(); }
std::unordered_map<std::string, struct Player> PlayerList::get_players_map(){ return players; }
void PlayerList::show_players(){
    std::cout << "name\tsockfd\tmode\tx\ty" << std::endl;
    for (auto& player : players){
        std::cout << player.first << "\t";
        std::cout << player.second.sockfd << "\t";
        std::cout << player.second.mode_player << "\t";
        std::cout << player.second.x_player << "\t";
        std::cout << player.second.y_player << std::endl;
    }
}
int PlayerList::get_player_sockfd(char* name){
    std::string name_str(name);
    return players[name_str].sockfd;
}