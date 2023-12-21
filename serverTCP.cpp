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
    FD_SET(connfd, &allset);
    if(connfd > maxfd) maxfd = connfd;
}
void serverTCP::login_mainloop(){
    while (true) {
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            //accept
            accept_client();
            std::cout << "new client: " << connfd << std::endl;
        }
        //看每個 client
        for (auto& player : players){
            sockfd = player.second.sockfd;
            if(FD_ISSET(sockfd, &rset)){
                std::cout << "client: " << sockfd << std::endl;
                //handle every client
                login_handle();
            }
        }
    }
}
void serverTCP::login_handle() {
    Packet packet = receiveData(sockfd);
    std::string login_name(packet.sender_name);

    // SHA-1 hashing for the password
    std::string password(packet.receiver_name);
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);

    // Convert the hashed password to a hexadecimal string
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    std::string hashed_password = ss.str();
    sqlServer sqlServer(login_name, hashed_password);

    if (packet.mode_packet == LOGINMODE) {
        if (sqlServer.login_check() == true) {
            //login success
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login success\n");
            sendData(new_packet, sockfd);
            std ::cout << "login success" << std::endl;
        } else {
            //login fail
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login fail\n");
            sendData(new_packet, sockfd);
            std ::cout << "login fail" << std::endl;
        }
    } else if (packet.mode_packet == REGISTERMODE) {
        if(sqlServer.db_register() == true){
            //register success
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register success\n");
            sendData(new_packet, sockfd);
        }else{
            //register fail
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register fail\n");
            sendData(new_packet, sockfd);
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
}
std::string serverTCP::serialize(Packet packet){
    std::string data = packet.packet_to_json().dump();
    return data;
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
//======= sqlServer
sqlServer::sqlServer(std::string user_name, std::string user_password){
    this->user_name = user_name;
    this->user_password = user_password;
}
void sqlServer::db_connect(){
    //connect MySQL
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "eee3228133@");
    //choose database
    con->setSchema("chatbar");
}
void sqlServer::db_query(){    
    try {
        //查詢
        res = prep_stmt->executeQuery();
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }
}
void sqlServer::db_clear(){
    if(res != NULL) delete res;
    if(prep_stmt != NULL) delete prep_stmt;
    delete con;
}
void sqlServer::db_pswd_select(){
    //query
    prep_stmt = con->prepareStatement("SELECT * FROM users WHERE username = ? AND password = ?");
    prep_stmt->setString(1, user_name);
    prep_stmt->setString(2, user_password);
}
bool sqlServer::login_check(){
    db_connect();
    db_pswd_select();
    db_query();
    if(res->next()){
        db_clear();
        return true;
    }else{
        db_clear();
        return false;
    }
}
void sqlServer::db_user_insert(){
    try {
        //insert
        prep_stmt = con->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)");
        prep_stmt->setString(1, user_name);
        prep_stmt->setString(2, user_password);
        
        affectedRows = prep_stmt->executeUpdate();
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }
}
bool sqlServer::db_register(){
    db_connect();
    if(affectedRows > 0){
        db_user_insert();
        db_clear();
        return true;
    }else{
        db_clear();
        return false;
    }
}