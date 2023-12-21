#include "header.h"
#include "clientTCP.h"

class Character {
    private:
        sf::Sprite characterSprite;
        sf::Text characterName;
        sf::Vector2f characterPosition;
    public:
        Character(){};
        Character(sf::Texture& texture, sf::Font& font, std::string name, float x, float y){
            characterPosition.x = x;
            characterPosition.y = y;
            characterSprite.setTexture(texture);
            characterSprite.setScale(0.08f, 0.08f);
            characterSprite.setPosition(characterPosition);
            characterName.setString(name);
            characterName.setFont(font);
            characterName.setCharacterSize(20);
            characterName.setFillColor(sf::Color::White);
            characterName.setPosition(characterPosition.x + 20, characterPosition.y - 20);
        }
        void move(float x, float y) {
            characterPosition.x += x;
            characterPosition.y += y;
            characterSprite.setPosition(characterPosition);
            characterName.setPosition(characterPosition.x + 20, characterPosition.y - 20);
        }
        sf::Vector2f getPosition() {
            return characterPosition;
        }
        void setPosition(float x, float y) {
            characterPosition.x = x;
            characterPosition.y = y;
            characterSprite.setPosition(characterPosition);
            characterName.setPosition(characterPosition.x + 20, characterPosition.y - 20);
        }
        void Draw(sf::RenderWindow& window) {
            window.draw(characterSprite);
            window.draw(characterName);
        }
        bool mainCharacterMove(bool isWindowFocused) {
            bool Changed = false; 
            if (!isWindowFocused) return Changed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                move(0, -2.7f);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                move(-2.7f, 0);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                move(0, 2.7f);
                Changed = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                move(2.7f, 0);
                Changed = true;
            }
            return Changed;
        }
};

class OtherCharacter : public Character {
public:
    OtherCharacter(){};
    // 使用 Character 的构造函数，并添加 distance 的初始化
    OtherCharacter(sf::Texture& texture, sf::Font& font, std::string name, float x, float y, float mx, float my) : Character(texture, font, name, x, y){
        refreshDistance(mx, my);
    }
    void refreshDistance(float mx, float my) {
        distance = sqrt(pow(mx - getPosition().x, 2) + pow(my - getPosition().y, 2));
        std::cout << "distance: " << distance << std::endl;
    }
    float getDistance() {
        return distance;
    }
private:
    float distance;
};


int main() {
    // 建立與伺服器的連接
    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);

    // 輸入名字
    std::string name;
    std::cout << "請輸入你的名字: ";
    std::getline(std::cin, name); 
    Packet mainCharacterPacket(MAPMODE, name, "", 918, 847, "");

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

    // 接受其他玩家的資訊
    std::unordered_map<std::string, OtherCharacter> otherCharacters;
    Packet otherCharacterPacketSize = TCPdata.receiveData();
    if (otherCharacterPacketSize.mode_packet == INITMODE) {
        for(int i = 0; i < (int)otherCharacterPacketSize.x_packet ; i++) {
            Packet packet = TCPdata.receiveData();
            if (packet.mode_packet == MAPMODE) {
                OtherCharacter otherCharacter(characterTexture, font, packet.sender_name, packet.x_packet, packet.y_packet, packet.x_packet, packet.y_packet);
                otherCharacters[packet.sender_name] = otherCharacter;
            }
        }
    }

    // 給伺服器發送自己的資訊
    TCPdata.sendData(mainCharacterPacket);

    // 設置地圖
    sf::Sprite mapSprite(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    // 設置主角
    Character mainCharacter(characterTexture, font, name, mainCharacterPacket.x_packet, mainCharacterPacket.y_packet);
    
    // 設置視角
    sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    view.setCenter(mainCharacter.getPosition());
    
    // Nonblock模式
    TCPdata.turnOnNonBlock();
    sf::Text messageBar;
    messageBar.setFont(font);  // 使用前面加载的字体
    messageBar.setCharacterSize(50);  // 字体大小
    messageBar.setFillColor(sf::Color::Cyan);  // 字体颜色
    // 遊戲主循環
    bool isWindowFocused = true;
    float aspectRatio = 0;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::GainedFocus) isWindowFocused = true;
            if (event.type == sf::Event::LostFocus) isWindowFocused = false; 
            if (event.type == sf::Event::Resized) {
                float newWidth = static_cast<float>(event.size.width);
                float newHeight = static_cast<float>(event.size.height);
                aspectRatio = newWidth / newHeight;
                sf::FloatRect visibleArea(0, 0, 800.f * aspectRatio, 600.f);
                view = sf::View(visibleArea);
            }
        }
        std::string mainCharacterName;
        float minDistance = 100000000;
        if (mainCharacter.mainCharacterMove(isWindowFocused)) {
            Packet packet(MAPMODE, name, "", mainCharacter.getPosition().x, mainCharacter.getPosition().y, "");
            TCPdata.sendData(packet);
            for(auto& otherCharacter : otherCharacters){
                otherCharacter.second.refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                if(minDistance > otherCharacter.second.getDistance()) {
                    minDistance = otherCharacter.second.getDistance();
                    mainCharacterName = otherCharacter.first;
                    if(minDistance < CHATDISTANCE) {
                        std::string message =  "Press X to ask " + mainCharacterName + " to ChatBar!";
                        messageBar.setString(message);
                    }else {
                        messageBar.setString("");
                    }
                }
            }    
        }
        // 接收其他玩家的位置
        while(true){
            Packet updatePacket = TCPdata.receiveDataNonBlock();
            if (updatePacket.mode_packet == EMPTYMODE) {
                std::cout << "break" << std::endl;
                break;
            }
            if (updatePacket.mode_packet == MAPMODE) {
                std::cout << "packet.sender_name: " << updatePacket.sender_name << std::endl;
                std::cout << "packet.x_packet: " << updatePacket.x_packet << std::endl;
                std::cout << "packet.y_packet: " << updatePacket.y_packet << std::endl;
                if (otherCharacters.find(updatePacket.sender_name) == otherCharacters.end()) {
                    OtherCharacter otherCharacter(characterTexture, font, updatePacket.sender_name, updatePacket.x_packet, updatePacket.y_packet, mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                    otherCharacters[updatePacket.sender_name] = otherCharacter;
                } else {
                    otherCharacters[updatePacket.sender_name].setPosition(updatePacket.x_packet, updatePacket.y_packet);
                    otherCharacters[updatePacket.sender_name].refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                }
                if(otherCharacters[updatePacket.sender_name].getDistance() < minDistance) {
                    minDistance = otherCharacters[updatePacket.sender_name].getDistance();
                    mainCharacterName = updatePacket.sender_name;
                    if(minDistance < CHATDISTANCE) {
                        std::string message =  "Press X to ask " + mainCharacterName + " to ChatBar!";
                        messageBar.setString(message);
                    }else {
                        messageBar.setString("");
                    }
                }
            }
        }

        view.setCenter(mainCharacter.getPosition());
        sf::Vector2f viewCenter = view.getCenter();
        sf::Vector2f viewSize = view.getSize();
        float messageBarX = viewCenter.x - viewSize.x / 2 + 10;  // 视图左边缘 + 10
        float messageBarY = viewCenter.y + viewSize.y / 2 - messageBar.getCharacterSize();  // 视图底边缘 - 30
        messageBar.setPosition(messageBarX, messageBarY);
        window.setView(view);
        window.clear();
        window.draw(mapSprite);
        window.draw(messageBar);
        mainCharacter.Draw(window);

        // 繪製其他玩家
        // 渲染
        for(auto& otherCharacter : otherCharacters) otherCharacter.second.Draw(window);

        // 顯示
        window.display();
    }

    return 0;
}
