#include "header.h"
#include "serverTCP.h"
using namespace std;

int main() {
    cout << "Server is running..." << endl;
    serverTCP serverTCP;
    serverTCP.TCP_connect(LOGINPORT);
    serverTCP.login_mainloop();

    return 0;
}
