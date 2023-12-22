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
            //讀 name
            Packet packet = receiveData_login(connfd);
            std::cout << "sender: " << packet.sender_name << std::endl;
            //放入 vector
            Player new_player(connfd, LOGINMODE, 0, 0);
            players[packet.sender_name] = new_player;
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
    Packet packet = receiveData_login(sockfd);
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
    //no sha
    // sqlServer sqlServer(login_name, password);

    if (packet.mode_packet == LOGINMODE) {
        if (sqlServer.login_check() == true) {
            //login success
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login success");
            sendData(new_packet, sockfd);
            std ::cout << "success" << std::endl;
        } else {
            //login fail
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login fail");
            sendData(new_packet, sockfd);
            std ::cout << "fail" << std::endl;
        }
    } else if (packet.mode_packet == REGISTERMODE) {
        std::cout << "register" << std::endl;
        if(sqlServer.db_register() == true){
            //register success
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register success");
            sendData(new_packet, sockfd);
            std ::cout << "success" << std::endl;
        }else{
            //register fail
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register fail");
            sendData(new_packet, sockfd);
            std ::cout << "fail" << std::endl;
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
        //disconnect list handle
        while(!disconnect_list.empty()){
            players.erase(disconnect_list.back());
            disconnect_list.pop_back();
        }
    }
}
void serverTCP::game_handle(){
    Packet packet = receiveData_game(sockfd);
    std::cout << "sender: " << packet.sender_name << " (" << packet.x_packet << ", " << packet.y_packet << ")" << std::endl;
    std::cout << "player num : " << players.size() << std::endl;
    if(packet.mode_packet == MAPMODE){
        //map mode
        Packet new_packet(MAPMODE, packet.sender_name, "", packet.x_packet, packet.y_packet, "");
        broadcast_xy(new_packet, sockfd);
    }else if(packet.mode_packet == REQMODE){
        //request mode
        int receiver_sockfd = players[packet.receiver_name].sockfd;
        if(strcmp(packet.sender_name, packet.receiver_name) == 0){
            Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Connect?\n");
            sendData(new_packet, receiver_sockfd);
        }else if(strcmp(packet.message, "Yes\n") == 0){
            Packet new_packet(REQMODE, packet.receiver_name, packet.sender_name, 0, 0, "Yes\n");
            sendData(new_packet, receiver_sockfd);
        }else if(strcmp(packet.message, "No\n") == 0){
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
    Packet packet = receiveData_game(connfd);
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
Packet serverTCP::receiveData_login(int sockfd){
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int n = Readline(sockfd, buffer, MAXLINE);
    if (n == -1) {
        std::cout << "(server) Failed to receive data" << std::endl;
        perror("Failed to receive data");
    }else if(n == 0){
        std::cout << "client disconnected" << std::endl;
        close(sockfd);
        return Packet();
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
Packet serverTCP::receiveData_game(int sockfd){
    char buffer[MAXLINE];
    bzero(buffer, MAXLINE);
    int n = Readline(sockfd, buffer, MAXLINE);
    if (n == -1) {
        std::cout << "(server) Failed to receive data" << std::endl;
        perror("Failed to receive data");
    }else if(n == 0){
        std::string name = get_player_name(sockfd);
        std::cout << name << " disconnected" << std::endl;
        //broadcast
        Packet new_packet(MAPMODE, name.c_str(), "", -500, -500, "");
        broadcast_xy(new_packet, sockfd);
        //add to disconnect list
        disconnect_list.push_back(name);
        //close
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        return Packet();
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void serverTCP::broadcast_xy(Packet packet, int sockfd){
    for (auto& player : players){
        if(player.second.sockfd != sockfd){
            //send
            sendData(packet, player.second.sockfd);
        }
    }
}
std::string serverTCP::get_player_name(int sockfd){
    for (auto& player : players){
        if(player.second.sockfd == sockfd){
            return player.first;
        }
    }
    return NULL;
}
//======= sqlServer
sqlServer::sqlServer(){
    //connect MySQL
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "gameuser", "Eee3228133@");
    //choose database
    con->setSchema("chatbar");
}
sqlServer::sqlServer(std::string user_name, std::string user_password){
    sqlServer();
    this->user_name = user_name;
    this->user_password = user_password;
}
sqlServer::~sqlServer(){
    delete con;
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
    std::cout << "db_clear1" << std::endl;
    if(res != NULL) delete res;
    std::cout << "db_clear2" << std::endl;
    if(prep_stmt != NULL) delete prep_stmt;
    std::cout << "db_clear3" << std::endl;
}
void sqlServer::db_pswd_select(){
    //query
    prep_stmt = con->prepareStatement("SELECT * FROM user WHERE UserName = ? AND UserPassword = ?");
    prep_stmt->setString(1, user_name);
    prep_stmt->setString(2, user_password);
}
void sqlServer::db_user_select(){
    //query
    prep_stmt = con->prepareStatement("SELECT * FROM user WHERE UserName = ?");
    prep_stmt->setString(1, user_name);
}
bool sqlServer::login_check(){
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
bool sqlServer::register_check(){
    db_user_select();
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
        prep_stmt = con->prepareStatement("INSERT INTO user (UserName, UserPassword) VALUES (?, ?)");
        prep_stmt->setString(1, user_name);
        prep_stmt->setString(2, user_password);
        std::cout << "user_name: " << user_name << std::endl;
        std::cout << "user_password: " << user_password << std::endl;
        
        affectedRows = prep_stmt->executeUpdate();
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }
}
int sqlServer::db_register(){
    if(register_check() == true){ return register_repeat; }
    db_user_insert();
    std::cout << "user_name: " << user_name << std::endl;
    std::cout << "user_password: " << user_password << std::endl;
    std::cout << "affectedRows: " << affectedRows << std::endl;
    db_clear();
    if(affectedRows > 0){
        std::cout << "affectedRows > 0" << std::endl;
        return register_success;
    }else{
        return register_fail;
    }
}