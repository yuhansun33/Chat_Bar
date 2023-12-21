#ifndef sqlserver_h
#define sqlserver_h
#include "header.h"
#include "elementTCP.h"
class sqlServer{
public:
    sqlServer(){};
    sqlServer(std::string user_name, std::string user_password);
    void db_connect();
    void db_query();
    void db_clear();
    void db_pswd_select();
    bool login_check();
    void db_user_insert();
    bool db_register();
private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;
    int affectedRows;
    std::string user_name;
    std::string user_password; 
};

#endif