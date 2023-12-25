#include "header.h"
#include "clientTCP.h"

class ChatEnvironment{
public:
    bool HasRequest = false;
    bool RequestSent = false;
    bool InChatMode = false;
    std::string ReqSender;
    std::string ReqReceiver;

    sf::Texture requestTexture;
    sf::Sprite requestSprite;
    sf::Texture chatroomTexture;
    sf::Sprite chatroomSprite;
    bool changeView = false;

    ChatEnvironment(){
        if (!requestTexture.loadFromFile("Assets/Pictures/request.png")) {
            perror("request圖片加載失敗");
            exit(-1);
        }
        if(!chatroomTexture.loadFromFile("Assets/Pictures/chatroom.png")){
            perror("chatroom圖片加載失敗");
            exit(-1);
        }
        requestSprite.setTexture(requestTexture);
        requestSprite.setScale(0.5f, 0.5f);
        chatroomSprite.setTexture(chatroomTexture);
        chatroomSprite.setScale(0.45f, 0.35f);

    }
};

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


int main(int argc, char** argv) {
    // 建立與伺服器的連接
    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);

    // 輸入名字
    std::string name;
    if (argc == 2) {
        name = argv[1];
    } else {
        return -1;
    }
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
    sf::Font chatfont;
    if (!chatfont.loadFromFile("Assets/Fonts/chat_font.ttf")) {
        perror("字體加載失敗");
        return -1;
    }
    // 定義顏色
    sf::Color brown(139, 69, 19);
    sf::Color LavenderBlush(255, 240, 245);

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

    // 設置聊天環境
    ChatEnvironment chatEnv;

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
    // 設置文字
    sf::Text messageBar;
    messageBar.setFont(font);  // 使用前面加载的字体
    messageBar.setCharacterSize(50);  // 字体大小
    messageBar.setFillColor(LavenderBlush);  // 字体颜色
    sf::Text requestText;
    requestText.setFont(font);
    requestText.setCharacterSize(30);
    requestText.setFillColor(brown);
    sf::Text chatRecord;
    chatRecord.setFont(chatfont);
    chatRecord.setCharacterSize(30);
    chatRecord.setFillColor(sf::Color::White);
    std::string chatHistory;
    std::string userInput;
    // 設置文字框
    sf::RectangleShape inputBox(sf::Vector2f(300, 35));
    inputBox.setFillColor(sf::Color::White);
    // 設置初始聊天視角
    

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
            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Enter){
                    if(userInput.empty()) continue;
                    std::cout << "userInput: " << userInput << std::endl;
                    Packet chatPacket(CHATMODE, name, chatEnv.ReqReceiver.c_str(), 0, 0, userInput.c_str());
                    TCPdata.sendData(chatPacket);
                    chatHistory += name + "  :  " + userInput + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    userInput.clear();
                    //移動視角
                    chatEnv.chatroomSprite.move(0, MOVEDISTANCE); // 向上移动背景图
                    view.move(0, MOVEDISTANCE); // 向上移动视图
                    window.setView(view);
                    inputBox.move(0, MOVEDISTANCE);
                    chatEnv.changeView = true;
                }
                if (chatEnv.InChatMode && event.key.code == sf::Keyboard::Escape) {
                    std::string message =  name + " left the chatroom.\nPress Esc back to map.";
                    Packet chatPacket(ESCMODE, name, chatEnv.ReqReceiver.c_str(), 0, 0, message.c_str());
                    TCPdata.sendData(chatPacket);
                    std::cout << "發送離開聊天室訊息" << std::endl;
                    chatHistory += message + "\n";
                    chatEnv.changeView = true;
                    // 回到地圖模式
                    chatEnv.InChatMode = false;
                }
            }
            if(event.type == sf::Event::TextEntered){
                if(event.text.unicode == '\b' && !userInput.empty()){
                    userInput.pop_back();
                }else if(event.text.unicode >= 32 && event.text.unicode <= 126){
                    userInput += static_cast<char>(event.text.unicode);
                }
            }
        }
        // 地圖模式
        if(chatEnv.InChatMode == false){
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
                        chatEnv.ReqReceiver = mainCharacterName;
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
                    } else if (updatePacket.x_packet == -500 && updatePacket.y_packet == -500) {
                        otherCharacters.erase(updatePacket.sender_name);
                    } else {
                        otherCharacters[updatePacket.sender_name].setPosition(updatePacket.x_packet, updatePacket.y_packet);
                        otherCharacters[updatePacket.sender_name].refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                    }
                    if(otherCharacters[updatePacket.sender_name].getDistance() < minDistance) {
                        minDistance = otherCharacters[updatePacket.sender_name].getDistance();
                        mainCharacterName = updatePacket.sender_name;
                        chatEnv.ReqReceiver = mainCharacterName;
                        if(minDistance < CHATDISTANCE) {
                            std::string message =  "Press X to ask " + mainCharacterName + " to ChatBar!";
                            messageBar.setString(message);
                        }else {
                            messageBar.setString("");
                        }
                    }
                }
                if (updatePacket.mode_packet == REQMODE) {
                    if(strcmp(updatePacket.message, "Connect?") == 0){
                        std::cout << "收到聊天請求" << std::endl;
                        chatEnv.HasRequest = true;
                        chatEnv.ReqSender = updatePacket.sender_name;
                        std::cout << "chatEnv.ReqFrom: " << chatEnv.ReqSender << std::endl;
                    }else if(strcmp(updatePacket.message, "Can chat") == 0){
                        std::cout << "收到聊天同意" << std::endl;
                        chatEnv.RequestSent = false;
                        chatEnv.InChatMode = true;
                        //重設聊天室
                        chatEnv.changeView = false;
                        chatHistory.clear();
                        userInput.clear();
                    }else if(strcmp(updatePacket.message, "Can not chat") == 0){
                        std::cout << "收到聊天拒絕" << std::endl;
                        chatEnv.RequestSent = false;
                    }
                }
            }
            //處裡按下要求聊天
            if (isWindowFocused && !chatEnv.RequestSent && sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
                // std::cout << "(2)minDistance: " << minDistance << std::endl;
                // if (minDistance < CHATDISTANCE) {
                    std::cout << "======================按下X, 送出first request" << std::endl;
                    std::cout << "name: " << name << std::endl;
                    std::cout << "recver: " << chatEnv.ReqReceiver << std::endl;
                    Packet chatRequestPacket(REQMODE, name, chatEnv.ReqReceiver.c_str(), 0, 0, "first request");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.RequestSent = true;
                // }
            }
            //處裡被要求聊天
            if(chatEnv.HasRequest) {
                std::string message =  chatEnv.ReqSender + ":\nrequests to chat!\n"
                                                        +   "        (y/n)";
                requestText.setString(message);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {
                    //接受請求
                    Packet chatRequestPacket(REQMODE, name, chatEnv.ReqSender.c_str(), 0, 0, "Yes");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.HasRequest = false;
                    chatEnv.InChatMode = true;
                    //重設聊天室
                    chatEnv.changeView = false;
                    chatHistory.clear();
                    userInput.clear();
                    std::cout << "按下Y" << std::endl;
                    // 進入聊天模式
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                    //拒絕請求
                    Packet chatRequestPacket(REQMODE, name, chatEnv.ReqSender.c_str(), 0, 0, "No");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.HasRequest = false;
                    std::cout << "按下N" << std::endl;                        
                }
            }   

            view.setCenter(mainCharacter.getPosition());
            sf::Vector2f viewCenter = view.getCenter();
            sf::Vector2f viewSize = view.getSize();
            window.setView(view);
            window.clear();
            window.draw(mapSprite);
            mainCharacter.Draw(window);

            // 繪製其他玩家
            // 渲染
            for(auto& otherCharacter : otherCharacters) otherCharacter.second.Draw(window);

            if(!chatEnv.HasRequest){
                float messageBarX = viewCenter.x - viewSize.x / 2 + 10;  // 视图左边缘 + 10
                float messageBarY = viewCenter.y + viewSize.y / 2 - messageBar.getCharacterSize();  // 视图底边缘 - 30
                messageBar.setPosition(messageBarX, messageBarY);
                window.draw(messageBar);
            }else if(chatEnv.HasRequest){
                chatEnv.requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(chatEnv.requestSprite);
                requestText.setPosition(chatEnv.requestSprite.getPosition().x + 65, chatEnv.requestSprite.getPosition().y + 100);
                window.draw(requestText);
            }
        //地圖模式結束
        }else if(chatEnv.InChatMode == true){
            // 聊天模式
            while(true){
                Packet packet = TCPdata.receiveDataNonBlock();
                if (packet.mode_packet == EMPTYMODE) {
                    std::cout << "no msg" << std::endl;
                    break;
                }
                if(packet.mode_packet == CHATMODE){
                    std::string senderName(packet.sender_name);
                    std::string messageText(packet.message);
                    chatHistory +=  senderName + "  :  " + messageText + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    //移動視角
                    chatEnv.chatroomSprite.move(0, MOVEDISTANCE); // 向上移动背景图
                    view.move(0, MOVEDISTANCE); // 向上移动视图
                    window.setView(view);
                    inputBox.move(0, MOVEDISTANCE);
                    chatEnv.changeView = true;
                }
                if(packet.mode_packet == ESCMODE){
                    std::cout << "收到離開聊天室訊息" << std::endl;
                    std::string messageText(packet.message);
                    chatHistory += messageText + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    //移動視角
                    chatEnv.chatroomSprite.move(0, 2 * MOVEDISTANCE); // 向上移动背景图
                    view.move(0, 2 * MOVEDISTANCE); // 向上移动视图
                    window.setView(view);
                    inputBox.move(0, 2 * MOVEDISTANCE);
                    chatEnv.changeView = true;
                }
            }




            //view, clear, draw
            sf::FloatRect spriteBounds = chatEnv.chatroomSprite.getGlobalBounds();
            sf::Vector2f spriteCenter(spriteBounds.left + spriteBounds.width / 2.0f,
                                    spriteBounds.top + spriteBounds.height / 2.0f);
            if(chatEnv.changeView == false){
                view.setCenter(spriteCenter);
                window.setView(view);
                inputBox.setPosition(chatEnv.chatroomSprite.getPosition().x + spriteBounds.width / 3.3f,
                            chatEnv.chatroomSprite.getPosition().y + spriteBounds.height / 1.115f);
                chatRecord.setPosition(chatEnv.chatroomSprite.getPosition().x + spriteBounds.width / 8.5f,
                            chatEnv.chatroomSprite.getPosition().y + spriteBounds.height / 1.14f);
            }
            
            chatRecord.setString(chatHistory);
            window.clear();
            window.draw(chatRecord);
            window.draw(chatEnv.chatroomSprite);
            window.draw(inputBox);

            sf::Text enterInput(userInput, chatfont, 30);
            enterInput.setPosition(inputBox.getPosition().x + 5, inputBox.getPosition().y);
            enterInput.setFillColor(sf::Color::Black);
            window.draw(enterInput);
        }
        // 顯示
        window.display();
    }

    return 0;
}
