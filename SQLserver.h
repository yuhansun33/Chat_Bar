#ifndef sqlserver_h
#define sqlserver_h
#include "header.h"
#include "elementTCP.h"
class sqlServer{
public:
    sqlServer(){};
    void db_connect();
    void db_query();
    void db_clear();
    void db_pswd_select(std::string db_name, std::string password);
    bool login_check(std::string name, std::string password);
private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res
};

#endif