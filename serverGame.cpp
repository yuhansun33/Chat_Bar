#include "header.h"
#include "serverTCP.h"
using namespace std;

int main() {
    serverTCP serverTCP;
    cout << "Game server is running..." << endl;
    serverTCP.TCP_connect(GAMEPORT);
    serverTCP.game_mainloop();
    return 0;
}
