#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

struct Player {
    float x;
    float y;
};

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(53000);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding server socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on server socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server is listening on port 53000..." << std::endl;

    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
    if (clientSocket == -1) {
        std::cerr << "Error accepting client connection" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Client connected: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

    Player player;
    player.x = 0.0f;
    player.y = 0.0f;

    while (true) {
        ssize_t bytesRead = read(clientSocket, &player, sizeof(Player));
        if (bytesRead <= 0) {
            std::cout << "Client disconnected: " << inet_ntoa(clientAddress.sin_addr) << std::endl;
            break;
        }

        // Broadcast player's position to other clients
        send(clientSocket, &player, sizeof(Player), 0);
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
