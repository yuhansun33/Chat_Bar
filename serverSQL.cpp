#include "elementTCP.h"
#include "serverSQL.h"
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