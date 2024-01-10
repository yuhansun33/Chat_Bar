#include "header.h"
#include "serverTCP.h"
using namespace std;

int main() {
    cout << "Game server is running..." << endl;
    if(fork() == 0){
        serverTCP serverTCP;
        serverTCP.TCP_connect(LOGINPORT);
        serverTCP.login_mainloop();
    }else if(fork() > 0){
        serverTCP serverTCP;
        serverTCP.TCP_connect(GAMEPORT);
        serverTCP.game_mainloop();
    }
    return 0;
}
