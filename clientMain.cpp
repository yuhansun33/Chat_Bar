#include "header.h"
#include "clientTCP.h"

sf::View view;
sf::RenderWindow window;
sf::Texture mapTexture;
sf::Texture characterTexture;
sf::Font font;
sf::Font chatFont;
sf::Music bgm;
sf::Sprite mapSprite;
sf::Color lavenderBlush;
sf::Color brown;

class ChatEnvironment{
    private:
        int chatState;
        bool changeView;
        std::string requestSenderName;
        std::string requestReceiverName;
        sf::Texture requestTexture;
        sf::Texture chatroomTexture;
        sf::Sprite requestSprite;
        sf::Sprite chatroomSprite;
        sf::Clock chatClock;
        sf::Text requestText;
        sf::Text chatRecord;
        std::string chatHistory;
        std::string userInput;
        sf::RectangleShape chatInputBox;
    public:
        ChatEnvironment(){
            if (!requestTexture.loadFromFile("Assets/Pictures/request.png")) {
                perror("request圖片加載失敗");
                exit(-1);
            }
            if(!chatroomTexture.loadFromFile("Assets/Pictures/chatroom.png")){
                perror("chatroom圖片加載失敗");
                exit(-1);
            }
            chatInputBox.setSize(sf::Vector2f(300, 35));
            chatInputBox.setFillColor(sf::Color::White);
            requestSprite.setTexture(requestTexture);
            requestSprite.setScale(0.5f, 0.5f);
            chatroomSprite.setTexture(chatroomTexture);
            chatroomSprite.setScale(0.45f, 0.35f);
            requestText.setFont(font);
            requestText.setCharacterSize(30);
            requestText.setFillColor(brown);
            chatRecord.setFont(chatFont);
            chatRecord.setCharacterSize(30);
            chatRecord.setFillColor(sf::Color::White);
        }
        void startTimer(){chatClock.restart();}
        sf::Time getChatTime(){return chatClock.getElapsedTime();}
};

class Character {
    private:
        sf::Sprite characterSprite;
        sf::Text characterName;
        sf::Vector2f characterPosition;
    public:
        Character(){};
        Character(std::string name, float x, float y){
            characterPosition.x = x;
            characterPosition.y = y;
            characterSprite.setTexture(characterTexture);
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
        void draw() {
            window.draw(characterSprite);
            window.draw(characterName);
        }
        bool mainCharacterMove() {
            bool Changed = false; 
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
    friend class OtherCharacters;
    private:
        float distance;
    public:
        OtherCharacter(){};
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
};

class Rank {
    private:
        sf::Color frameBackgroundColor, titleTextColor, bestRankTextColor, yourRankTextColor;
        sf::RectangleShape bestRankFrame, yourRankFrame;
        sf::Text bestRankText, yourRankText, bestRankName, bestRankScore, yourRankName, yourRankScore;
    public:
        Rank(sf::Font& font, std::string name){
            frameBackgroundColor = sf::Color(238, 221, 130, 230);
            titleTextColor = sf::Color(218, 165, 32);
            bestRankTextColor = sf::Color(205, 92, 92);
            yourRankTextColor = sf::Color(128, 128, 128);

            bestRankFrame = sf::RectangleShape(sf::Vector2f(180, 60));
            bestRankFrame.setFillColor(frameBackgroundColor);

            yourRankFrame = sf::RectangleShape(sf::Vector2f(180, 60));
            yourRankFrame.setFillColor(frameBackgroundColor);

            bestRankText.setFont(font);
            bestRankText.setCharacterSize(20);
            bestRankText.setFillColor(titleTextColor);
            bestRankText.setString("BEST");

            bestRankName.setFont(font);
            bestRankName.setCharacterSize(25);
            bestRankName.setFillColor(bestRankTextColor);
            bestRankName.setString("-"); 

            bestRankScore.setFont(font);
            bestRankScore.setCharacterSize(20);
            bestRankScore.setFillColor(bestRankTextColor);
            bestRankScore.setString("0.0");

            yourRankText.setFont(font);
            yourRankText.setCharacterSize(20);
            yourRankText.setFillColor(titleTextColor);
            yourRankText.setString("YOUR'S");

            yourRankName.setFont(font);
            yourRankName.setCharacterSize(25);
            yourRankName.setFillColor(yourRankTextColor);
            yourRankName.setString(name);

            yourRankScore.setFont(font);
            yourRankScore.setCharacterSize(20);
            yourRankScore.setFillColor(yourRankTextColor);
            yourRankScore.setString("0.0"); 
        };
        void setPosition(){
            sf::Vector2f viewSize = view.getSize();
            sf::Vector2f viewCenter = view.getCenter();
            bestRankFrame.setPosition(viewCenter.x + viewSize.x / 2 - 190, viewCenter.y - viewSize.y / 2 + 5);
            bestRankName.setPosition(viewCenter.x + viewSize.x / 2 - 170, viewCenter.y - viewSize.y / 2 + 30);
            bestRankScore.setPosition(viewCenter.x + viewSize.x / 2 - 60, viewCenter.y - viewSize.y / 2 + 35);
            yourRankFrame.setPosition(viewCenter.x + viewSize.x / 2 - 190, viewCenter.y - viewSize.y / 2 + 70);
            yourRankName.setPosition(viewCenter.x + viewSize.x / 2 - 170, viewCenter.y - viewSize.y / 2 + 95);
            yourRankScore.setPosition(viewCenter.x + viewSize.x / 2 - 60, viewCenter.y - viewSize.y / 2 + 100);
        }
        void draw(){
            window.draw(bestRankFrame);
            window.draw(yourRankFrame);
            window.draw(bestRankText);
            window.draw(yourRankText);
            window.draw(bestRankName);
            window.draw(bestRankScore);
            window.draw(yourRankName);
            window.draw(yourRankScore);
        };
        void updateTime(char* time){
            std::string timeStr(time);
            yourRankScore.setString(time);
        };
        void updateRank(char* name, char* time){
            std::string nameStr(name);
            std::string timeStr(time);
            bestRankName.setString(name);
            bestRankScore.setString(time);
        };
};

class OtherCharacters{
    private:
        float minDistance;
        std::string minDistanceCharacterName;
        std::unordered_map<std::string, OtherCharacter> otherCharactersMap;
        sf::Text informOtherCharacterNearBy;
    public:
        OtherCharacters(ClientConnectToServer &TCPdata){
            informOtherCharacterNearBy.setFont(font);
            informOtherCharacterNearBy.setCharacterSize(50);
            informOtherCharacterNearBy.setFillColor(lavenderBlush);
            minDistance = 10000000;
            Packet initOtherCharacterPacket = TCPdata.receiveData();
            if(initOtherCharacterPacket.mode_packet != INITMODE) return;
            int otherCharacterNumber = (int)initOtherCharacterPacket.x_packet;
            for(int i = 0; i < otherCharacterNumber ; i++) {
                Packet packet = TCPdata.receiveData();
                if (packet.mode_packet == MAPMODE) {
                    OtherCharacter otherCharacter(characterTexture, font, packet.sender_name, packet.x_packet, packet.y_packet, packet.x_packet, packet.y_packet);
                    otherCharactersMap[packet.sender_name] = otherCharacter;
                }
            }
        };
        void drawAllOtherCharacters(){
            for(auto& otherCharacter : otherCharactersMap) otherCharacter.second.draw();
        }
        void updateOtherCharactersByPacket(Character& mainCharacter, Packet& otherCharacterUpdatePacket){
            if(otherCharactersMap.find(otherCharacterUpdatePacket.sender_name) == otherCharactersMap.end()){
                OtherCharacter otherCharacter(characterTexture, font, otherCharacterUpdatePacket.sender_name, otherCharacterUpdatePacket.x_packet, otherCharacterUpdatePacket.y_packet, mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                otherCharactersMap[otherCharacterUpdatePacket.sender_name] = otherCharacter;
            } else if (otherCharacterUpdatePacket.x_packet == NOWHERE && otherCharacterUpdatePacket.y_packet == NOWHERE) {
                otherCharactersMap.erase(otherCharacterUpdatePacket.sender_name);
            } else {
                otherCharactersMap[otherCharacterUpdatePacket.sender_name].setPosition(otherCharacterUpdatePacket.x_packet, otherCharacterUpdatePacket.y_packet);
                otherCharactersMap[otherCharacterUpdatePacket.sender_name].refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
            }
            if(otherCharactersMap[otherCharacterUpdatePacket.sender_name].getDistance() < minDistance) {
                minDistance = otherCharactersMap[otherCharacterUpdatePacket.sender_name].getDistance();
                minDistanceCharacterName = otherCharacterUpdatePacket.sender_name;
                if(minDistance < CHATDISTANCE) {
                    std::string message =  "Press X to ask " + minDistanceCharacterName + " to ChatBar!";
                    informOtherCharacterNearBy.setString(message);
                }else {
                    informOtherCharacterNearBy.setString("");
                }
            }
        }
        void updateOtherCharactersByMainCharacter(Character& mainCharacter){
            for(auto& otherCharacter : otherCharactersMap){
                otherCharacter.second.refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                if(otherCharacter.second.getDistance() < minDistance) {
                    minDistance = otherCharacter.second.getDistance();
                    minDistanceCharacterName = otherCharacter.first;
                    if(minDistance < CHATDISTANCE) {
                        std::string message =  "Press X to ask " + minDistanceCharacterName + " to ChatBar!";
                        informOtherCharacterNearBy.setString(message);
                    }else {
                        informOtherCharacterNearBy.setString("");
                    }
                }
            }
        }
};

void clientPacketHandler(Packet& clientReceivedPacket, ClientConnectToServer& TCPdata, OtherCharacters& OCs, Character& mainCharacter, Rank& rank){
    switch (clientReceivedPacket.mode_packet) {
        case EMPTYMODE:
            break;
        case MAPMODE:
            OCs.updateOtherCharactersByPacket(mainCharacter, clientReceivedPacket);
            break;
        // case CHATMODE:    
            // break;
        case REQMODE:

            break;
        case TIMEMODE:
            rank.updateTime(clientReceivedPacket.message);
            break;
        case RANKMODE:
            rank.updateRank(clientReceivedPacket.sender_name, clientReceivedPacket.message);
            break;
        default:
            break;
    } 
}

int main(int argc, char** argv) {

    brown           = sf::Color(139, 69, 19);
    lavenderBlush   = sf::Color(255, 240, 245);

    if (!bgm.openFromFile("Assets/Musics/bgm.ogg")) {
        perror("音樂加載失敗");
        return -1;
    }
    if (!mapTexture.loadFromFile("Assets/Pictures/map.png")) {
        perror("地圖加載失敗");
        return -1;
    }
    if(!characterTexture.loadFromFile("Assets/Pictures/boy.png")){
        perror("主角圖片加載失敗");
        return -1;
    }
    if (!font.loadFromFile("Assets/Fonts/login_font.ttf")) {
        perror("字體加載失敗");
        return -1;
    }
    if (!chatFont.loadFromFile("Assets/Fonts/chat_font.ttf")) {
        perror("字體加載失敗");
        return -1;
    }
    mapSprite.setTexture(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);
    std::string name(argv[1]);
    sf::Vector2f initPosition(918, 847);
    Packet mainCharacterPacket(MAPMODE, name, "", initPosition.x, initPosition.y, "");
    window.create(sf::VideoMode(500, 500), "ChatBar");
    window.setFramerateLimit(60);
    bgm.setLoop(true);
    bgm.play();

    // 接受其他玩家的資訊
    // std::unordered_map<std::string, OtherCharacter> otherCharacters;
    // Packet otherCharacterPacketSize = TCPdata.receiveData();
    // if (otherCharacterPacketSize.mode_packet == INITMODE) {
    //     int otherCharacterNumber = (int)otherCharacterPacketSize.x_packet;
    //     for(int i = 0; i < otherCharacterNumber ; i++) {
    //         Packet packet = TCPdata.receiveData();
    //         if (packet.mode_packet == MAPMODE) {
    //             OtherCharacter otherCharacter(characterTexture, font, packet.sender_name, packet.x_packet, packet.y_packet, packet.x_packet, packet.y_packet);
    //             otherCharacters[packet.sender_name] = otherCharacter;
    //         }
    //     }
    // }

    // 給伺服器發送自己的資訊
    TCPdata.sendData(mainCharacterPacket);

    // 設置聊天環境
    ChatEnvironment chatEnv;

    // 設置主角
    Character mainCharacter(name, mainCharacterPacket.x_packet, mainCharacterPacket.y_packet);

    view.reset(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    view.setCenter(mainCharacter.getPosition());
    
    // Nonblock模式
    TCPdata.turnOnNonBlock();

    Rank rank(font, name);

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
                    Packet chatPacket(CHATMODE, name, chatEnv.requestReceiver.c_str(), 0, 0, userInput.c_str());
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
                if (chatEnv.isChatting && event.key.code == sf::Keyboard::Escape) {
                    std::string message =  name + " left the chatroom.\nPress Esc back to map.";
                    Packet chatPacket(ESCMODE, name, chatEnv.requestReceiver.c_str(), 0, 0, message.c_str());
                    TCPdata.sendData(chatPacket);
                    std::cout << "發送離開聊天室訊息" << std::endl;
                    chatHistory += message + "\n";  
                    chatEnv.changeView = true;
                    // 回到地圖模式
                    chatEnv.isChatting = false;
                    //結束計時
                    sf::Time duration = chatEnv.getChatTime();
                    float minutes = duration.asSeconds() / 60.0f;
                    char minutesStr[20];
                    sprintf(minutesStr, "%.1f", minutes);
                    std::cout << "minutes: " << minutesStr << " min" << std::endl;
                    Packet timePacket(TIMEMODE, name, "", 0, 0, minutesStr);
                    TCPdata.sendData(timePacket);
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
        if(chatEnv.isChatting == false){
            // std::string mainCharacterName;
            // float minDistance = 100000000;
            if (mainCharacter.mainCharacterMove()) {
                Packet packet(MAPMODE, name, "", mainCharacter.getPosition().x, mainCharacter.getPosition().y, "");
                TCPdata.sendData(packet);
                // for(auto& otherCharacter : otherCharacters){
                //     otherCharacter.second.refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                //     if(minDistance > otherCharacter.second.getDistance()) {
                //         minDistance = otherCharacter.second.getDistance();
                //         mainCharacterName = otherCharacter.first;
                //         chatEnv.requestReceiver = mainCharacterName;
                //         if(minDistance < CHATDISTANCE) {
                //             std::string message =  "Press X to ask " + mainCharacterName + " to ChatBar!";
                //             systemMessage.setString(message);
                //         }else {
                //             systemMessage.setString("");
                //         }
                //     }
                // }    
            }
            // 接收其他玩家的位置
            while(true){
                // Packet updatePacket = TCPdata.receiveDataNonBlock();
                // if (updatePacket.mode_packet == EMPTYMODE) {
                //     std::cout << "break" << std::endl;
                //     break;
                // }
                // if (updatePacket.mode_packet == MAPMODE) {
                //     std::cout << "packet.sender_name: " << updatePacket.sender_name << std::endl;
                //     std::cout << "packet.x_packet: " << updatePacket.x_packet << std::endl;
                //     std::cout << "packet.y_packet: " << updatePacket.y_packet << std::endl;
                //     if (otherCharacters.find(updatePacket.sender_name) == otherCharacters.end()) {
                //         OtherCharacter otherCharacter(characterTexture, font, updatePacket.sender_name, updatePacket.x_packet, updatePacket.y_packet, mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                //         otherCharacters[updatePacket.sender_name] = otherCharacter;
                //     } else if (updatePacket.x_packet == -500 && updatePacket.y_packet == -500) {
                //         otherCharacters.erase(updatePacket.sender_name);
                //     } else {
                //         otherCharacters[updatePacket.sender_name].setPosition(updatePacket.x_packet, updatePacket.y_packet);
                //         otherCharacters[updatePacket.sender_name].refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                //     }
                //     if(otherCharacters[updatePacket.sender_name].getDistance() < minDistance) {
                //         minDistance = otherCharacters[updatePacket.sender_name].getDistance();
                //         mainCharacterName = updatePacket.sender_name;
                //         chatEnv.requestReceiver = mainCharacterName;
                //         if(minDistance < CHATDISTANCE) {
                //             std::string message =  "Press X to ask " + mainCharacterName + " to ChatBar!";
                //             systemMessage.setString(message);
                //         }else {
                //             systemMessage.setString("");
                //         }
                //     }
                // }
                if (updatePacket.mode_packet == REQMODE) {
                    if(strcmp(updatePacket.message, "Connect?") == 0){
                        std::cout << "收到聊天請求" << std::endl;
                        chatEnv.isRequestRecv = true;
                        chatEnv.requestSender = updatePacket.sender_name;
                        std::cout << "chatEnv.ReqFrom: " << chatEnv.requestSender << std::endl;
                    }else if(strcmp(updatePacket.message, "Can chat") == 0){
                        std::cout << "收到聊天同意" << std::endl;
                        chatEnv.isRequestSend = false;
                        chatEnv.isChatting = true;
                        //開始計時
                        chatEnv.startTimer();
                        //重設聊天室
                        chatEnv.changeView = false;
                        chatHistory.clear();
                        userInput.clear();
                    }else if(strcmp(updatePacket.message, "Can not chat") == 0){
                        std::cout << "收到聊天拒絕" << std::endl;
                        chatEnv.isRequestSend = false;
                    }
                }
                // if(updatePacket.mode_packet == TIMEMODE){
                //     std::cout << "收到時間訊息" << std::endl;
                //     std::string messageText(updatePacket.message);
                //     yourRankScore.setString(messageText);
                // }
                // if(updatePacket.mode_packet == RANKMODE){
                //     std::cout << "收到最高時間訊息" << std::endl;
                //     std::string user(updatePacket.sender_name);
                //     std::string messageText(updatePacket.message);
                //     bestRankName.setString(user);
                //     bestRankScore.setString(messageText);
                // }
            }
            //處裡按下要求聊天
            if (isWindowFocused && !chatEnv.isRequestSend && sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
                // std::cout << "(2)minDistance: " << minDistance << std::endl;
                // if (minDistance < CHATDISTANCE) {
                    std::cout << "======================按下X, 送出first request" << std::endl;
                    std::cout << "name: " << name << std::endl;
                    std::cout << "recver: " << chatEnv.requestReceiver << std::endl;
                    Packet chatRequestPacket(REQMODE, name, chatEnv.requestReceiver.c_str(), 0, 0, "first request");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.isRequestSend = true;
                // }
            }
            //處裡被要求聊天
            if(chatEnv.isRequestRecv) {
                std::string message =  chatEnv.requestSender + ":\nrequests to chat!\n"
                                                        +   "        (y/n)";
                requestText.setString(message);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) {
                    //接受請求
                    Packet chatRequestPacket(REQMODE, name, chatEnv.requestSender.c_str(), 0, 0, "Yes");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.isRequestRecv = false;
                    chatEnv.isChatting = true;
                    //開始計時
                    chatEnv.startTimer();
                    //重設聊天室
                    chatEnv.changeView = false;
                    chatHistory.clear();
                    userInput.clear();
                    std::cout << "按下Y" << std::endl;
                    // 進入聊天模式
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                    //拒絕請求
                    Packet chatRequestPacket(REQMODE, name, chatEnv.requestSender.c_str(), 0, 0, "No");
                    TCPdata.sendData(chatRequestPacket);
                    chatEnv.isRequestRecv = false;
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
            // 計分框
            // 繪製其他玩家
            // 渲染
            for(auto& otherCharacter : otherCharacters) otherCharacter.second.Draw(window);
            rank.setPosition(view);
            rank.Draw();

            if(!chatEnv.isRequestRecv){
                float systemMessageX = viewCenter.x - viewSize.x / 2 + 10;  
                float systemMessageY = viewCenter.y + viewSize.y / 2 - systemMessage.getCharacterSize();  
                systemMessage.setPosition(systemMessageX, systemMessageY);
                window.draw(systemMessage);
            }else if(chatEnv.isRequestRecv){
                chatEnv.requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(chatEnv.requestSprite);
                requestText.setPosition(chatEnv.requestSprite.getPosition().x + 65, chatEnv.requestSprite.getPosition().y + 100);
                window.draw(requestText);
            }
        //地圖模式結束
        }else if(chatEnv.isChatting == true){
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

            sf::Text enterInput(userInput, chatFont, 30);
            enterInput.setPosition(inputBox.getPosition().x + 5, inputBox.getPosition().y);
            enterInput.setFillColor(sf::Color::Black);
            window.draw(enterInput);
        }
        window.display();
    }
    return 0;
}
