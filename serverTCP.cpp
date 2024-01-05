#include "elementTCP.h"
#include "serverTCP.h"
#include "readline.h"

serverTCP::serverTCP(){
    signal(SIGCHLD, sig_chld);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // Set listenfd to non-blocking mode
    int flags = fcntl(listenfd, F_GETFL, 0);
    if (flags == -1) { std::cout << "fcntl F_GETFL error" << std::endl; }
    flags |= O_NONBLOCK;
    if (fcntl(listenfd, F_SETFL, flags) == -1) { std::cout << "fcntl F_SETFL error" << std::endl; }
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
bool serverTCP::accept_client(){
    clilen = sizeof(cliaddr);
    if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
        if (errno != EINTR){
            // std::cout << "accept error" << std::endl;
            return false;
            // perror("accept error");
        }
    }
    FD_SET(connfd, &allset);
    if(connfd > maxfd) maxfd = connfd;
    return true;
}
void serverTCP::login_mainloop(){
    sqlServer sqlServer;
    while (true) {
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            //accept
            if(accept_client() == false) continue;
            //讀 name
            Packet packet = receiveData_login(connfd);
            std::cout << "sender: " << packet.sender_name << std::endl;
            //放入 vector
            Player new_player(connfd, LOGINMODE, 0, 0, packet.sender_name);
            players[packet.sender_name] = new_player;
        }
        //看每個 client
        for (auto& player : players){
            sockfd = player.second.sockfd;
            if(FD_ISSET(sockfd, &rset)){
                std::cout << "client: " << sockfd << std::endl;
                //handle every client
                login_handle(sqlServer);
            }
        }
    }
}
void serverTCP::login_handle(sqlServer& sqlServer) {
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
    sqlServer.db_information(login_name, hashed_password);
    //no sha
    // sqlServer sqlServer(login_name, password);

    if (packet.mode_packet == LOGINMODE) {
        if (sqlServer.login_check() == true) {
            //login success
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login success");
            sendData(new_packet, sockfd);
            std ::cout << "login success" << std::endl;
        } else {
            //login fail
            Packet new_packet(LOGINMODE, packet.sender_name, "", 0, 0, "login fail");
            sendData(new_packet, sockfd);
            std ::cout << "login fail" << std::endl;
        }
    } else if (packet.mode_packet == REGISTERMODE) {
        std::cout << "register" << std::endl;
        int register_status = sqlServer.db_register();
        if(register_status == REG_SUCCESS){
            //register success
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register success");
            sendData(new_packet, sockfd);
            std ::cout << "register success" << std::endl;
        }else if(register_status == REG_REPEAT){
            //register fail
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register repeat");
            sendData(new_packet, sockfd);
            std ::cout << "register repeat" << std::endl;
        }else if(register_status == REG_FAIL){
            //register fail
            Packet new_packet(REGISTERMODE, packet.sender_name, "", 0, 0, "register fail");
            sendData(new_packet, sockfd);
            std ::cout << "register fail" << std::endl;
        }
    }
}
void serverTCP::game_mainloop(){
    sqlServer sqlServer;
    while (true) {
        rset = allset;
        n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        //listenfd
        if(FD_ISSET(listenfd, &rset)){
            //accept
            if(accept_client() != false){
                //new client handle
                new_game_handle(sqlServer);
            }
        }
        //看每個 client
        for (auto& player : players){
            sockfd = player.second.sockfd;
            if(FD_ISSET(sockfd, &rset)){
                //handle every client
                game_handle(sqlServer);
            }
        }
        //disconnect list handle
        while(!disconnect_list.empty()){
            auto firstElement = disconnect_list.begin();
            players.erase(firstElement->second);
            disconnect_list.erase(firstElement->first);
        }
    }
}
void serverTCP::game_handle(sqlServer& sqlServer){
    Packet packet = receiveData_game(sockfd);
    std::cout << "sender: " << packet.sender_name << " (" << packet.x_packet << ", " << packet.y_packet << ")" << std::endl;
    std::cout << "player num : " << players.size() << std::endl;

    switch(packet.mode_packet) {
        case MAPMODE: {
            //更新位置
            players[packet.sender_name].x_player = packet.x_packet;
            players[packet.sender_name].y_player = packet.y_packet;
            Packet new_packet(MAPMODE, packet.sender_name, "", packet.x_packet, packet.y_packet, "");
            broadcast_xy(new_packet, sockfd);
            break;
        }
        case REQMODE: {
            int sender_sockfd = players[packet.sender_name].sockfd;
            int receiver_sockfd = players[packet.receiver_name].sockfd;
            int receiver_roomID = players[packet.receiver_name].roomID;
            std::cout << "receiver_sockfd: " << receiver_sockfd << std::endl;
            if(strcmp(packet.message, "first request") == 0){
                Packet new_packet(REQMODE, packet.sender_name, packet.receiver_name, 0, 0, "Connect?");
                if(receiver_roomID != -1){
                    Packet new_packet(REQMODE, packet.sender_name, packet.receiver_name, 0, 0, "Can chat");
                    sendData(new_packet, sender_sockfd);
                    roomList[receiver_roomID].push_back(players[packet.sender_name]);
                    players[packet.sender_name].roomID = receiver_roomID;
                    return;
                }
                sendData(new_packet, receiver_sockfd);
                std::cout << "get first request, send \"Connect?\"" << std::endl;
            }else if(strcmp(packet.message, "Yes") == 0){
                Packet new_packet(REQMODE, packet.sender_name, packet.receiver_name, 0, 0, "Can chat");
                sendData(new_packet, receiver_sockfd);
                std::cout << packet.sender_name << " ==> " << packet.receiver_name << std::endl;
                std::cout << "recv Yes, send \"Can chat\"" << std::endl;
                //加入 roomList 並 broadcast
                getNewRoom(players[packet.sender_name], players[packet.receiver_name]);
            }else if(strcmp(packet.message, "No") == 0){
                Packet new_packet(REQMODE, packet.sender_name, packet.receiver_name, 0, 0, "Can not chat");
                sendData(new_packet, receiver_sockfd);
                std::cout << "recv No, send \"Can not chat\"" << std::endl;
            }
            break;
        }
        case CHATMODE: {
            // int receiver_sockfd = players[packet.receiver_name].sockfd;
            std::cout << "send chat message: " << packet.sender_name << std::endl;
            Packet new_packet(CHATMODE, packet.sender_name, packet.receiver_name, 0, 0, packet.message);
            // sendData(new_packet, receiver_sockfd);
            broadcast_chatroom(new_packet, sockfd);
            // std::cout << " ==> " << packet.receiver_name << std::endl;
            break;
        }
        case ESCMODE: {
            int receiver_sockfd = players[packet.receiver_name].sockfd;
            std::cout << "send esc message: " << packet.sender_name;
            Packet new_packet(ESCMODE, packet.sender_name, packet.receiver_name, 0, 0, packet.message);
            broadcast_chatroom(new_packet, sockfd);
            //清空聊天室
            int roomID = players[packet.sender_name].roomID;
            auto& room = roomList[roomID];
            for (auto it = room.begin(); it != room.end(); /* no increment here */) {
                if (it->playerID == packet.sender_name) {
                    it = room.erase(it);  // erase 返回下一个有效迭代器
                    break;  
                } else {
                    ++it;  // 只在未删除元素时递增迭代器
                }
            }
            if(room.empty()){
                roomList.erase(roomList.begin() + roomID);
                Packet room_packet(ROOMMODE, std::to_string(roomID), "", NOWHERE, NOWHERE, "");
                broadcast_xy(room_packet, -500); 
            }
            players[packet.sender_name].roomID = -1;
            std::cout << " ==> " << packet.receiver_name << std::endl;
            break;
        }
        case TIMEMODE: {
            std::cout << "收到 time mode" << std::endl;
            int self_sockfd = players[packet.sender_name].sockfd;
            //time mode
            sqlServer.addtimelen(packet.sender_name, packet.message);
            //發送個人累積聊天時間
            sendSelfTotalTime(sqlServer, packet.sender_name, self_sockfd);
            //廣播最大聊天時間
            broadcastMaxTime(sqlServer);
            break;
        }
        case JOINMODE: {
            int roomID = atoi(packet.receiver_name);
            players[packet.sender_name].roomID = roomID;
            roomList[roomID].push_back(players[packet.sender_name]);
            Packet new_packet(REQMODE, packet.sender_name, packet.receiver_name, 0, 0, "Can chat");
            sendData(new_packet, players[packet.sender_name].sockfd);
            break;
        }
    }
}
void serverTCP::new_game_handle(sqlServer& sqlServer){
    //init packet
    Packet init_packet(INITMODE, "", "", (float)players.size(), (float)roomList.size(), "");
    sendData(init_packet, connfd);
    //送所有人位置
    for (auto& player : players){
        Packet packet(MAPMODE, player.first, "", player.second.x_player, player.second.y_player, "");
        sendData(packet, connfd);
    }
    //讀 ID
    Packet packet = receiveData_game(connfd);
    broadcast_xy(packet, connfd);
    //放入 vector
    Player new_player(connfd, MAPMODE, packet.x_packet, packet.y_packet, packet.sender_name);
    players[packet.sender_name] = new_player;
    //資料庫放入名字
    sqlServer.db_information(packet.sender_name, "");
    //發送個人累積聊天時間
    sendSelfTotalTime(sqlServer, packet.sender_name, connfd);
    //廣播最大聊天時間
    broadcastMaxTime(sqlServer);
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
void serverTCP::sendData(Packet& packet, int sockfd){
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
        FD_CLR(sockfd, &allset);
        close(sockfd);
        return Packet();
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void serverTCP::remove_player(int sockfd){
    std::cout << "someone disconnected" << std::endl;
    for (auto play_it = players.begin(); play_it != players.end(); /* no increment here */) {
        if (play_it->second.sockfd == sockfd) {
            if(play_it->second.roomID != -1){
                int roomID = play_it->second.roomID;
                auto& room = roomList[roomID];
                Packet ESC_packet(ESCMODE, play_it->first, "", 0, 0, play_it->first + " disconnected\n");
                broadcast_chatroom(ESC_packet, sockfd);
                for (auto it = room.begin(); it != room.end(); /* no increment here */) {
                    if (it->playerID == play_it->first) {
                        it = room.erase(it);  // erase 返回下一个有效迭代器
                        break;  
                    } else {
                        ++it;  // 只在未删除元素时递增迭代器
                    }
                }
                if(room.empty()){
                    roomList.erase(roomList.begin() + roomID);
                    Packet room_packet(ROOMMODE, std::to_string(roomID), "", NOWHERE, NOWHERE, "");
                    broadcast_xy(room_packet, -500); 
                }
            }
            Packet new_packet(MAPMODE, play_it->first, "", NOWHERE, NOWHERE, "");
            std::cout << "broadcast: " << play_it->first << " disconnected" << std::endl;
            broadcast_xy(new_packet, sockfd);
            // play_it = players.erase(play_it);  // erase 返回下一个有效迭代器
            break;  
        } else {
            ++play_it;  // 只在未删除元素时递增迭代器
        }
    }
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
        remove_player(sockfd);
        //add to disconnect list
        disconnect_list[sockfd] = name;
        //close
        shutdown(sockfd, SHUT_RDWR);
        FD_CLR(sockfd, &allset);
        close(sockfd);
        return Packet();
    }
    std::string data = buffer;
    Packet packet = deserialize(data);
    return packet;
}
void serverTCP::broadcast_xy(Packet& packet, int sockfd){
    for (auto& player : players){
        if(player.second.sockfd != sockfd and disconnect_list.find(player.second.sockfd) == disconnect_list.end()){
            sendData(packet, player.second.sockfd);
        }
    }
}

void serverTCP::broadcast_chatroom(Packet& packet, int sockfd){
    int roomID = players[packet.sender_name].roomID;
    for(auto& player : roomList[roomID]){
        if(player.sockfd != sockfd and disconnect_list.find(player.sockfd) == disconnect_list.end()){
            sendData(packet, player.sockfd);
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
void serverTCP::getNewRoom(Player& player1, Player& player2){
    bool findRoom = false;
    static char roomID[10];
    //找空的聊天室
    for(size_t i = 0; i < roomList.size(); i++){
        if(roomList[i].empty()){
            roomList[i].push_back(player1);
            roomList[i].push_back(player2);
            sprintf(roomID, "%d", i);
            findRoom = true;
        }
    }
    //沒有空的聊天室
    if(findRoom == false){
        roomList.push_back(std::vector<Player>{player1, player2});
        sprintf(roomID, "%d", roomList.size() - 1);
    }
    float x_room = (player1.x_player + player2.x_player) / 2;
    float y_room = (player1.y_player + player2.y_player) / 2;
    player1.roomID = atoi(roomID);
    player2.roomID = atoi(roomID);
    std::cout << "Player1: " << player1.playerID <<" Player1 sockfd "<< player1.sockfd << " roomID: " << player1.roomID << std::endl;
    std::cout << "Player2: " << player2.playerID <<" Player2 sockfd "<< player2.sockfd << " roomID: " << player2.roomID << std::endl;
    Packet room_packet(ROOMMODE, roomID, "", x_room, y_room, "");
    broadcast_xy(room_packet, -500); //broadcast 給所有人
}
void serverTCP::broadcastMaxTime(sqlServer& sqlServer){
    std::pair<std::string, float> maxTime = sqlServer.getUserMaxTime();
    char maxTime_char[10];
    sprintf(maxTime_char, "%.1f", maxTime.second);
    Packet new_packet(RANKMODE, maxTime.first, "", 0, 0, maxTime_char);
    broadcast_xy(new_packet, -500); //broadcast 給所有人
}
void serverTCP::sendSelfTotalTime(sqlServer& sqlServer, char* name, int sockfd){
    float totalTime = sqlServer.getSelfTotalTime();
    char totalTime_char[10];
    sprintf(totalTime_char, "%.1f", totalTime);
    Packet new_packet(TIMEMODE, name, "", 0, 0, totalTime_char);
    sendData(new_packet, sockfd);
    std::cout << "send time: " << totalTime_char << std::endl;
}
//======= sqlServer
sqlServer::sqlServer(){
    //connect MySQL
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "gameuser", "Eee3228133@");
    //choose database
    con->setSchema("chatbar");
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
void sqlServer::db_information(std::string new_user_name, std::string new_user_password){
    this->user_name = new_user_name;
    this->user_password = new_user_password;
}
void sqlServer::db_clear(){
    if(res != NULL){
        delete res;
        res = NULL;
    }
    if(prep_stmt != NULL){
        delete prep_stmt;
        prep_stmt = NULL;
    }
}
void sqlServer::db_pswd_select(){
    //query
    prep_stmt = con->prepareStatement("SELECT * FROM user WHERE UserName = ? AND UserPassword = ?");
    prep_stmt->setString(1, user_name);
    prep_stmt->setString(2, user_password);
}
void sqlServer::db_user_select(){
    //query
    std::cout << "Value of con: " << con << std::endl;
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
void sqlServer::db_time_insert(){
    try {
        //insert
        prep_stmt = con->prepareStatement("INSERT INTO timeRecord (UserName, timeLen) VALUES (?, ?)");
        prep_stmt->setString(1, user_name);
        prep_stmt->setString(2, timelen);
        std::cout << "user_name: " << user_name << std::endl;
        std::cout << "timelen: " << timelen << std::endl;
        
        affectedRows = prep_stmt->executeUpdate();
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }
}
int sqlServer::db_register(){
    if(register_check() == true){ return REG_REPEAT; }
    db_user_insert();
    std::cout << "user_name: " << user_name << std::endl;
    std::cout << "user_password: " << user_password << std::endl;
    std::cout << "affectedRows: " << affectedRows << std::endl;
    db_clear();
    if(affectedRows > 0){
        std::cout << "affectedRows > 0" << std::endl;
        return REG_SUCCESS;
    }else{
        return REG_FAIL;
    }
}
void sqlServer::addtimelen(std::string name, char* msg){
    std::string length(msg);
    this->user_name = name;
    this->timelen = length;
    db_time_insert();
    db_clear();
    if(affectedRows > 0){
        std::cout << "add time success" << std::endl;
    }else{
        perror("add time fail");
    }
}
float sqlServer::getSelfTotalTime(){
    std::cout << "getSelfTotalTime" << std::endl;
    float totalTime = 0.0f;
    try {
        //查詢
        prep_stmt = con->prepareStatement("SELECT SUM(timeLen) AS TotalTime FROM timeRecord WHERE UserName = ?");
        prep_stmt->setString(1, user_name);
        res = prep_stmt->executeQuery();
        if (res->next()) {
            totalTime = res->getDouble("TotalTime");
        }
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }
    db_clear();
    return totalTime;
}
std::pair<std::string, float> sqlServer::getUserMaxTime() {
    std::cout << "getUserWithMaxTime" << std::endl;
    std::string maxUserName;
    float maxTotalTime = 0.0f;

    try {
        // 查詢最大時間總和的用戶
        prep_stmt = con->prepareStatement("SELECT UserName, SUM(timeLen) AS TotalTime FROM timeRecord GROUP BY UserName ORDER BY SUM(timeLen) DESC LIMIT 1");
        res = prep_stmt->executeQuery();

        // 獲取結果
        if (res->next()) {
            maxUserName = res->getString("UserName");
            maxTotalTime = res->getDouble("TotalTime");
        }
    } catch (sql::SQLException &e) {
        perror("SQLException");
    }

    db_clear();
    std::cout << "Max User: " << maxUserName << ", Total Time: " << maxTotalTime << std::endl;

    return std::make_pair(maxUserName, maxTotalTime);
}
