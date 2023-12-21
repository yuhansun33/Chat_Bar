#include "header.h"
#include "serverTCP.h"
using namespace std;

int main() {
    serverTCP serverTCP;
    cout << "Server is running..." << endl;
    serverTCP.TCP_connect(LOGINPORT);
    serverTCP.login_mainloop();

    return 0;
}
