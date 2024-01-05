#include "header.h"
#include "clientTCP.h"
#include "elementTCP.h"

bool Login(ClientConnectToServer& client, std::string& username, std::string& password, bool& firstConnection) {
    if(username.empty() || password.empty()) {
        std::cout << "Login failed" << std::endl;
        return false;
    }
    std::cout << "Login" << std::endl;
    Packet packet(LOGINMODE, username, password);
    std::cout << "Login: " << username << " Passwd: " << password << std::endl;
    if(firstConnection){
        client.sendData(packet);
        firstConnection = false;
    }
    client.sendData(packet);
    username.clear();
    password.clear();
    Packet packet2 = client.receiveData();
    if (packet2.mode_packet == LOGINMODE && strcmp(packet2.message, "login success") == 0) {
        std::cout << "Login success!" << std::endl;
        execlp("./main", "./main", packet2.sender_name, (char*)NULL);
        return true;
    } else {
        std::cout << "Login failed!" << std::endl;
        return false;
    }
}

int Register(ClientConnectToServer& ClientConnectToServer, std::string& username, std::string& password, bool& firstConnection) {
    if(username.empty() || password.empty()) {
        std::cout << "Register failed" << std::endl;
        return 0;
    }
    std::cout << "Register" << std::endl;
    Packet packet(REGISTERMODE, username, password);

    if(firstConnection){
        ClientConnectToServer.sendData(packet);
        firstConnection = false;
    }
    ClientConnectToServer.sendData(packet);
    username.clear();
    password.clear();
    Packet packet2 = ClientConnectToServer.receiveData();
    std::cout << packet2.message << std::endl;
    if (packet2.mode_packet == REGISTERMODE && strcmp(packet2.message, "register success") == 0) {
        std::cout << "Register success!" << std::endl;
        return 1;
    } else if (packet2.mode_packet == REGISTERMODE && strcmp(packet2.message, "register repeat") == 0){
        std::cout << "Register repeat!" << std::endl;
        return 0;
    } else {
        std::cout << "Register failed!" << std::endl;
        return -1;
    }
}



int main() {
    // 創建窗口
    ClientConnectToServer client;
    client.serverIPPort(SERVERIP, LOGINPORT);

    sf::RenderWindow window(sf::VideoMode(550, 300), "Login Screen");
    window.setFramerateLimit(60);

    // 登入介面元素
    sf::Color GoodGreen(189, 183, 107);
    sf::Color brown(205, 133, 63);
    sf::Color wheat(255, 231, 186);
    sf::Font font;
    if (!font.loadFromFile("Assets/Fonts/login_font.ttf")) {
        return -1;
    }

    sf::Sprite background;
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Assets/Pictures/login.png")) {
        return -1;
    }
    background.setTexture(backgroundTexture);
    background.setScale(0.3f, 0.3f);
    background.setPosition(0, 0);
    sf::Text usernameLabel("Username:", font, 23);
    usernameLabel.setPosition(50, 50);
    sf::Text passwordLabel("Password:", font, 23);
    passwordLabel.setPosition(50, 120);

    sf::RectangleShape usernameBox(sf::Vector2f(300, 30));
    usernameBox.setPosition(150, 50);
    usernameBox.setFillColor(sf::Color::White);

    sf::RectangleShape passwordBox(sf::Vector2f(300, 30));
    passwordBox.setPosition(150, 120);
    passwordBox.setFillColor(sf::Color::White);

    sf::RectangleShape loginButton(sf::Vector2f(100, 40));
    loginButton.setPosition(320, 200);
    loginButton.setFillColor(brown);

    sf::RectangleShape registerButton(sf::Vector2f(110, 40));
    registerButton.setPosition(100, 200);
    registerButton.setFillColor(GoodGreen);

    sf::Text loginButtonText("Login", font, 25);
    loginButtonText.setPosition(345, 205);
    loginButtonText.setFillColor(sf::Color::White);

    sf::Text registerButtonText("Register", font, 25);
    registerButtonText.setPosition(115, 205);
    registerButtonText.setFillColor(sf::Color::White);

    sf::Text errorText;
    errorText.setFont(font);
    errorText.setCharacterSize(30);
    errorText.setPosition(205, 205);
    errorText.setFillColor(sf::Color::Red);

    std::string username = "";
    std::string password = "";
    bool typingUsername = false, typingPassword = false;

    // 主循環
    bool firstConnection = true;
    int registerSuccess = true;
    bool loginSuccess = true;

    while (window.isOpen()) {
        std::cout << "firstConnection: " << firstConnection << std::endl;
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // 處理鼠標點擊
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    if (usernameBox.getGlobalBounds().contains(mousePos)) {
                        typingUsername = true;
                        typingPassword = false;
                    } else if (passwordBox.getGlobalBounds().contains(mousePos)) {
                        typingUsername = false;
                        typingPassword = true;
                    } else if (loginButton.getGlobalBounds().contains(mousePos)) {
                        std::cout << username << " " << password << std::endl;
                        loginSuccess = Login(client, username, password, firstConnection);
                        if(!loginSuccess) errorText.setString("Login failed");
                        typingUsername = true;
                        typingPassword = false;
                    } else if (registerButton.getGlobalBounds().contains(mousePos)) {
                        std::cout << "Register" << std::endl;
                        registerSuccess = Register(client, username, password, firstConnection);
                        if(registerSuccess == 1){
                            errorText.setColor(sf::Color::Green);
                            errorText.setString("Register success");
                            errorText.setColor(sf::Color::Red);
                        }
                        else if(registerSuccess == 0) errorText.setString("Register repeat");
                        else if(registerSuccess == -1) errorText.setString("Register failed");
                        typingUsername = true;
                        typingPassword = false;
                        continue;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    std::cout << username << " " << password << std::endl;
                    if(Login(client, username, password, firstConnection)){
                        errorText.setString("Login success");
                    };
                    typingUsername = true;
                    typingPassword = false;
                }
            }
            // 處理鍵盤輸入
            if (event.type == sf::Event::TextEntered) {
                if (typingUsername) {
                    if (event.text.unicode == '\b' && !username.empty()) { // Backspace
                        username.pop_back();
                    } else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                        username += static_cast<char>(event.text.unicode);
                    }
                } else if (typingPassword) {
                    if (event.text.unicode == '\b' && !password.empty()) { // Backspace
                        password.pop_back();
                    } else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                        password += static_cast<char>(event.text.unicode);
                    }
                }
            }
        }

        window.clear(sf::Color::Black);


        // 繪製介面元素
        window.draw(background);
        window.draw(usernameLabel);
        window.draw(passwordLabel);
        window.draw(usernameBox);
        window.draw(passwordBox);
        window.draw(loginButton);
        window.draw(registerButton);
        window.draw(loginButtonText);
        window.draw(registerButtonText);
        window.draw(errorText);
        // 在文本框中顯示已輸入的帳號和密碼（密碼以*顯示）
        sf::Text enteredUsername(username, font, 20);
        enteredUsername.setPosition(155, 55);
        enteredUsername.setFillColor(sf::Color::Black);
        window.draw(enteredUsername);

        std::string displayedPassword(password.length(), '*');
        sf::Text enteredPassword(displayedPassword, font, 20);
        enteredPassword.setPosition(155, 125);
        enteredPassword.setFillColor(sf::Color::Black);
        window.draw(enteredPassword);

        window.display();
    }

    return 0;
}
