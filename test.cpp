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
    Packet packet(MAPMODE, name, "", 0, 0, "");


    // 建立視窗
    sf::RenderWindow window(sf::VideoMode(500, 500), "聊Bar");

    // 加載地圖和角色的紋理
    sf::Texture mapTexture, characterTexture;
    if (!mapTexture.loadFromFile("Assets/Pictures/map.png") || !characterTexture.loadFromFile("Assets/Pictures/boy.png")) {
        perror("圖片加載失敗");
        return -1;
    }

    // // 接收伺服器發送的其他玩家的名字
    // std::unordered_map<char*, sf::Sprite> otherCharacters;
    // while(true) {
    //     Packet packet = TCPdata.receiveData();
    //     if (packet.mode_packet == MAPMODE) {
    //         sf::Sprite otherCharacter(characterTexture);
    //         otherCharacter.setScale(0.08f, 0.08f);
    //         otherCharacter.setPosition(packet.x_packet, packet.y_packet);
    //         otherCharacters[packet.sender_name] = otherCharacter;
    //     }
    // }

    // 給伺服器發送自己的名字
    TCPdata.sendData(packet);

    // 設置地圖
    sf::Sprite mapSprite(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    // 設置主角
    sf::Sprite character(characterTexture);
    character.setScale(0.08f, 0.08f);
    character.setPosition(packet.x_packet, packet.y_packet);


    sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    view.setCenter(character.getPosition());

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
        }

        // 更新主角的位置
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            character.move(0, -0.07f);
            Changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            character.move(-0.07f, 0);
            Changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            character.move(0, 0.07f);
            Changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            character.move(0.07f, 0);
            Changed = true;
        }

        // while(true){
        //     Packet packet = TCPdata.receiveData();
        //     if (packet.mode_packet == MAPMODE) {
        //         if (otherCharacters.find(packet.sender_name) == otherCharacters.end()) {
        //             sf::Sprite otherCharacter(characterTexture);
        //             otherCharacter.setScale(0.08f, 0.08f);
        //             otherCharacter.setPosition(packet.x_packet, packet.y_packet);
        //             otherCharacters[packet.sender_name] = otherCharacter;
        //         } else {
        //             otherCharacters[packet.sender_name].setPosition(packet.x_packet, packet.y_packet);
        //         }   
        //     }
        // }

        // // 渲染
        // for(auto& otherCharacter : otherCharacters) {
        //     window.draw(otherCharacter.second);
        // })
        if (Changed) {
            Packet packet(MAPMODE, name, "", character.getPosition().x, character.getPosition().y, "");
            TCPdata.sendData(packet);
        }

        view.setCenter(character.getPosition());
        window.setView(view);
        window.clear();
        window.draw(mapSprite);
        window.draw(character);

        // 繪製其他玩家

        // 顯示
        window.display();
    }

    return 0;
}
