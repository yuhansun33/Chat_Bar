#include "elementTCP.h"
#include "SQLserver.h"
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
void sqlServer::db_pswd_select(std::string db_name, std::string password){
    //query
    prep_stmt = con->prepareStatement("SELECT * FROM users WHERE username = ? AND password = ?");
    prep_stmt->setString(1, db_name);
    prep_stmt->setString(2, password);
}
bool sqlServer::login_check(std::string db_name, std::string password){
    db_connect();
    db_pswd_select(db_name, password);
    db_query();
    if(res->next()){
        db_clear();
        return true;
    }else{
        db_clear();
        return false;
    }
}