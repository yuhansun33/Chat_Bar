#ifndef serverTCP_h
#define serverTCP_h
#include "header.h"
#include "elementTCP.h"
class sqlServer;
class serverTCP{
    friend class sqlServer;
public:
//share
    serverTCP();
    static void sig_chld(int signo);
    void TCP_connect(int port);
    bool accept_client();
//login server
    void login_mainloop();
    void login_handle(sqlServer& sqlserver);
//game server
    void game_mainloop();
    void turnOnNonBlock();
    void new_game_handle(sqlServer& sqlServer);
    void game_handle(sqlServer& sqlServer);
    void sendData(Packet& packet, int sockfd);
    void broadcast_xy(Packet& packet, int sockfd);
    void broadcast_chatroom(Packet& packet, int sockfd);
    Packet receiveData_login(int sockfd);
    Packet receiveData_game(int sockfd);
    Packet deserialize(std::string& json_string);
    std::string serialize(Packet packet);
    std::string get_player_name(int sockfd);
    void remove_player(int sockfd);
    void getNewRoom(Player& player1, Player& player2);
    void broadcastMaxTime(sqlServer& sqlServer);
    void sendSelfTotalTime(sqlServer& sqlServer, char* name, int sockfd);
    
private:
	int			        n, listenfd, connfd, sockfd;
	socklen_t		    clilen;
	struct sockaddr_in	cliaddr, servaddr;
    int                 maxfd;
    fd_set              rset, allset;

    std::unordered_map<std::string, struct Player> players;
    std::unordered_map<int, std::string> disconnect_list; //name, sockfd
    std::vector<std::vector<Player>> roomList;
};

class sqlServer{
    friend class serverTCP;
public:
    sqlServer();
    ~sqlServer();
    int db_register();
    bool login_check();
    bool register_check();
    void addtimelen(std::string name, char* msg);
    void db_information(std::string new_user_name, std::string new_user_password);
    void db_query();
    void db_clear();
    void db_pswd_select();
    void db_user_select();
    void db_user_insert();
    void db_time_insert();
    float getSelfTotalTime();
    std::pair<std::string, float> getUserMaxTime();
private:
    int affectedRows;
    sql::ResultSet *res;
    std::string user_name;
    std::string user_password;
    std::string timelen;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;
    sql::PreparedStatement *prep_stmt;
};
#endif