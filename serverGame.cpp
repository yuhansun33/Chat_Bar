#include "header.h"
#include "serverTCP.h"
using namespace std;

int main() {
    cout << "Game server is running..." << endl;
    serverTCP serverTCP;
    serverTCP.TCP_connect(GAMEPORT);
    serverTCP.game_mainloop();

    return 0;
}
