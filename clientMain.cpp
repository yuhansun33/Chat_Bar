#include "header.h"
#include "clientTCP.h"



int main() {
    // 建立與伺服器的連接
    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);

    // 輸入名字
    std::string name;
    std::cout << "請輸入你的名字: ";
    std::getline(std::cin, name); 
    Packet packet(MAPMODE, name, "", 918, 847, "");


    // 建立視窗
    sf::RenderWindow window(sf::VideoMode(500, 500), "Chat Bar");
    window.setFramerateLimit(60);

    // 加載地圖和角色的紋理
    sf::Texture mapTexture, characterTexture;
    if (!mapTexture.loadFromFile("Assets/Pictures/map.png") || !characterTexture.loadFromFile("Assets/Pictures/boy.png")) {
        perror("圖片加載失敗");
        return -1;
    }
    sf::Font font;
    if (!font.loadFromFile("Assets/Fonts/login_font.ttf")) {
        perror("字體加載失敗");
        return -1;
    }

    Packet rp = TCPdata.receiveData();
    int otherCharactersSize = 0;
    std::cout << "test" << std::endl;
    if (rp.mode_packet == INITMODE) {
        otherCharactersSize = (int)rp.x_packet;
        std::cout << "otherCharactersSize: " << otherCharactersSize << std::endl;
    }


    std::unordered_map<std::string, sf::Sprite> otherCharacters;
    for(int i = 0; i < otherCharactersSize; i++) {
        Packet packet = TCPdata.receiveData();
        if (packet.mode_packet == MAPMODE) {
            sf::Sprite otherCharacter(characterTexture);
            otherCharacter.setScale(0.08f, 0.08f);
            otherCharacter.setPosition(packet.x_packet, packet.y_packet);
            std::string othername = packet.sender_name;
            otherCharacters[othername] = otherCharacter;
            std::cout << packet.sender_name << std::endl;
        }
    }

    // 給伺服器發送自己的名字
    TCPdata.sendData(packet);

    // 設置地圖
    sf::Sprite mapSprite(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    // 設置主角
    sf::Sprite character(characterTexture);
    character.setScale(0.08f, 0.08f);
    character.setPosition(packet.x_packet, packet.y_packet);
    sf::Text characterName(name, font, 20);
    characterName.setFillColor(sf::Color::White);
    characterName.setPosition(character.getPosition().x + 20, character.getPosition().y - 20);

    sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    view.setCenter(character.getPosition());
    bool isWindowFocused = true;
    TCPdata.turnOnNonBlock();

    // 遊戲主循環
    while (window.isOpen()) {
        bool Changed = false;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                Changed = true;
            }
            if (event.type == sf::Event::Resized) {
                float newWidth = static_cast<float>(event.size.width);
                float newHeight = static_cast<float>(event.size.height);
                float aspectRatio = newWidth / newHeight;

                sf::FloatRect visibleArea(0, 0, 800.f * aspectRatio, 600.f); // Adjust 800.f and 600.f based on your desired aspect ratio
                view = sf::View(visibleArea);
            }
            if (event.type == sf::Event::GainedFocus) { isWindowFocused = true; }
            if (event.type == sf::Event::LostFocus) { isWindowFocused = false; }
        }

        if(isWindowFocused){
            // 更新主角的位置
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                character.move(0, -2.7f);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                character.move(-2.7f, 0);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                character.move(0, 2.7f);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                character.move(2.7f, 0);
                Changed = true;
            }
        }
        characterName.setPosition(character.getPosition().x + 20, character.getPosition().y - 20);

        // 接收其他玩家的位置
        while(true){
            Packet packet = TCPdata.receiveDataNonBlock();
            if (packet.mode_packet == EMPTYMODE) {
                std::cout << "break" << std::endl;
                break;
            }
            if (packet.mode_packet == MAPMODE) {
                std::cout << "packet.sender_name: " << packet.sender_name << std::endl;
                std::cout << "packet.x_packet: " << packet.x_packet << std::endl;
                std::cout << "packet.y_packet: " << packet.y_packet << std::endl;

                if (otherCharacters.find(packet.sender_name) == otherCharacters.end()) {
                    sf::Sprite otherCharacter(characterTexture);
                    otherCharacter.setScale(0.08f, 0.08f);
                    otherCharacter.setPosition(packet.x_packet, packet.y_packet);
                    otherCharacters[packet.sender_name] = otherCharacter;
                } else {
                    otherCharacters[packet.sender_name].setPosition(packet.x_packet, packet.y_packet);
                }   
            }
        }


        if (Changed) {
            Packet packet(MAPMODE, name, "", character.getPosition().x, character.getPosition().y, "");
            TCPdata.sendData(packet);
        }

        view.setCenter(character.getPosition());
        window.setView(view);
        window.clear();
        window.draw(mapSprite);
        window.draw(character);
        window.draw(characterName);

        // 繪製其他玩家
        // 渲染
        for(auto& otherCharacter : otherCharacters) {
            window.draw(otherCharacter.second);
        }

        // 顯示
        window.display();
    }

    return 0;
}
