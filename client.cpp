#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <SFML/Graphics.hpp>
using namespace std;

struct Player {
    float x;
    float y;
};

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating client socket" << endl;
        return -1;
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // Replace with your server's IP address
    serverAddress.sin_port = htons(53000);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "Error connecting to the server" << endl;
        close(clientSocket);
        return -1;
    }

    cout << "Connected to the server" << endl;

    Player player;
    player.x = 0.0f;
    player.y = 0.0f;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Multiplayer Game Client");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                // Handle player input and update player's position
                if (event.key.code == sf::Keyboard::Left) {
                    player.x -= 5.0f;
                } else if (event.key.code == sf::Keyboard::Right) {
                    player.x += 5.0f;
                } else if (event.key.code == sf::Keyboard::Up) {
                    player.y -= 5.0f;
                } else if (event.key.code == sf::Keyboard::Down) {
                    player.y += 5.0f;
                }
            }
        }

        // Send player's position to the server
        send(clientSocket, &player, sizeof(Player), 0);

        // Receive positions of other players from the server
        Player otherPlayer;
        ssize_t bytesRead = recv(clientSocket, &otherPlayer, sizeof(Player), MSG_DONTWAIT);
        while (bytesRead > 0) {
            // Update other player's position in the game
            // ...

            bytesRead = recv(clientSocket, &otherPlayer, sizeof(Player), MSG_DONTWAIT);
        }

        // Clear the window
        window.clear();

        // Draw game elements, including the player and other players
        // ...

        // Display the contents of the window
        window.display();
    }

    close(clientSocket);

    return 0;
}
