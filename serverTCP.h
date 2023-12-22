#ifndef serverTCP_h
#define serverTCP_h
#include "header.h"
#include "elementTCP.h"
class serverTCP{
//share
public:
    serverTCP();
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
    void sendData(Packet packet, int sockfd);
    void broadcast_xy(Packet packet, int sockfd);
    Packet receiveData_login(int sockfd);
    Packet receiveData_game(int sockfd);
    Packet deserialize(std::string& json_string);
    std::string serialize(Packet packet);
    std::string get_player_name(int sockfd);
    
private:
	int			        n, listenfd, connfd, sockfd;
	socklen_t		    clilen;
	struct sockaddr_in	cliaddr, servaddr;
    int                 maxfd;
    fd_set              rset, allset;

    std::unordered_map<std::string, struct Player> players;
    std::vector<std::string> disconnect_list;
};

class sqlServer{
    friend class serverTCP;
public:
    sqlServer();
    sqlServer(std::string user_name, std::string user_password);
    ~sqlServer();
    int db_register();
    bool login_check();
    void db_query();
    void db_clear();
    void db_pswd_select();
    void db_user_insert();
private:
    int affectedRows;
    sql::ResultSet *res;
    std::string user_name;
    std::string user_password;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;
    sql::PreparedStatement *prep_stmt;
};
#endif