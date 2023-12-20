#include "header.h"
#include "clientTCP.h"



int main() {
    // 建立與伺服器的連接
    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);

    // 給伺服器發送自己的名字
    std::string name;
    std::cout << "請輸入你的名字: ";
    std::getline(std::cin, name); // 使用getline来读取整行
    // 使用name.c_str()来获取C风格字符串

    Packet packet;
    packet.mode_packet = MAPMODE;
    memset(packet.sender_name, 0, NAMELINE);
    strncpy(packet.sender_name, name.c_str(), NAMELINE-1);
    packet.sender_name[NAMELINE] = '\0';
    packet.x_packet = 0;
    packet.y_packet = 0;
    TCPdata.sendData(packet);
    // 接收伺服器發送的其他玩家的名字


    // 建立視窗
    sf::RenderWindow window(sf::VideoMode(500, 500), "聊Bar");

    // 加載地圖和角色的紋理
    sf::Texture mapTexture, characterTexture;
    if (!mapTexture.loadFromFile("Assets/Pictures/map.png") || !characterTexture.loadFromFile("Assets/Pictures/boy.png")) {
        perror("圖片加載失敗");
        return -1;
    }

    // 設置地圖
    sf::Sprite mapSprite(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    // 設置主角
    sf::Sprite character(characterTexture);
    character.setScale(0.08f, 0.08f);
    character.setPosition(packet.x_packet, packet.y_packet);

    // 假設從伺服器接收到的其他角色位置數據
    // std::vector<Player> otherPlayers = {
        
    // };

    sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    view.setCenter(character.getPosition());

    // 遊戲主循環
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 更新主角的位置
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            character.move(0, -0.07f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            character.move(-0.07f, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            character.move(0, 0.07f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            character.move(0.07f, 0);
        }

        // 渲染
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
