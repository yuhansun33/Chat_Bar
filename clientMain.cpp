#include "header.h"
#include "clientTCP.h"

std::string             playerID;
sf::View                view;
sf::Color               lavenderBlush;
sf::Color               brown;
sf::Texture             mapTexture;
sf::Texture             characterTexture;
sf::Font                font;
sf::Font                chatFont;
sf::Music               bgm;
sf::Sprite              mapSprite;
sf::Text                systemMessage;
sf::RenderWindow        window;

/*documentation:

classes:
    ClientConnectToServer:
        處理有關TCP連線的class，包含連線、傳送封包、接收封包、設定非堵塞等功能
    ChatEnvironment:
        處理有關聊天室的class，包含聊天室的狀態、聊天室的背景圖、聊天室的文字、聊天室的輸入框、聊天室的計時器等功能

    Character: 主角的class
    OtherCharacter: 其他玩家的class
    OtherCharacters: 其他玩家的集合
    Rank: 計分框的class
*/

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
        OtherCharacter(sf::Texture& texture, sf::Font& font, std::string name, float x, float y, float mx, float my) : Character(name, x, y){
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
        Rank(sf::Font& font, std::string name = playerID){
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
            bestRankText.setPosition(viewCenter.x + viewSize.x / 2 - 120, viewCenter.y - viewSize.y / 2 + 5);
            bestRankName.setPosition(viewCenter.x + viewSize.x / 2 - 170, viewCenter.y - viewSize.y / 2 + 30);
            bestRankScore.setPosition(viewCenter.x + viewSize.x / 2 - 60, viewCenter.y - viewSize.y / 2 + 35);
            yourRankFrame.setPosition(viewCenter.x + viewSize.x / 2 - 190, viewCenter.y - viewSize.y / 2 + 70);
            yourRankText.setPosition(viewCenter.x + viewSize.x / 2 - 130, viewCenter.y - viewSize.y / 2 + 70);
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
        void updateRank(char* bestName, char* time){
            std::string bestNameStr(bestName);
            std::string timeStr(time);
            bestRankName.setString(bestNameStr);
            bestRankScore.setString(timeStr);
        };
};

class OtherCharacters{
    private:
        float minDistance;
        std::string minDistanceCharacterName;
        std::unordered_map<std::string, OtherCharacter> otherCharactersMap;
    public:
        OtherCharacters(ClientConnectToServer &TCPdata){
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
                    systemMessage.setString(message);
                }else {
                    systemMessage.setString("");
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
                        systemMessage.setString(message);
                    }else {
                        systemMessage.setString("");
                    }
                }
            }
        }
};

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
            chatHistory.clear();
            userInput.clear();
            chatState = CHATSTATENONE;
            changeView = false;
        }

        void chatReplyReceivedHandler(Packet& packet){
            if(strcmp(packet.message, "Connect?") == 0){
                chatState = CHATSTATERECV;
                requestSenderName = packet.sender_name;
            }
            if(strcmp(packet.message, "Can not chat") == 0){
                chatState = CHATSTATENONE;
            }
            if(strcmp(packet.message, "Can chat") == 0){
                chatState = CHATSTATECHAT;
                chatStart();
            }
        }

        void chatRequestSendHandler(ClientConnectToServer& TCPdata){
            if(chatState != CHATSTATENONE) return;
            Packet chatRequestPacket(REQMODE, "", requestReceiverName.c_str(), 0, 0, "Connect?");
            TCPdata.sendData(chatRequestPacket);
            chatState = CHATSTATESEND;
        }

        void chatStart(){
            changeView = false;
            chatClock.restart();
            chatHistory.clear();
            userInput.clear();
        }

        void chatKeyHandler(sf::Event& event,  ClientConnectToServer& TCPdata, bool isWindowFocused) {
            if(!isWindowFocused) return;
            if(chatState == CHATSTATENONE){
                Packet chatRequestPacket(REQMODE, playerID, requestReceiverName.c_str(), 0, 0, "first request");
                TCPdata.sendData(chatRequestPacket);
                chatState = CHATSTATESEND;
            }
            else if(chatState == CHATSTATECHAT){
                if(event.type == sf::Event::TextEntered){
                    if(event.text.unicode == '\b' && !userInput.empty()){
                        userInput.pop_back();
                    }else if(event.text.unicode >= 32 && event.text.unicode <= 126){
                        userInput += static_cast<char>(event.text.unicode);
                    }
                }
                if(event.key.code == sf::Keyboard::Enter){
                    if(userInput.empty()) return;
                    Packet chatPacket(CHATMODE, "", requestReceiverName.c_str(), 0, 0, userInput.c_str());
                    TCPdata.sendData(chatPacket);
                    chatHistory += playerID + "  :  " + userInput + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    userInput.clear();
                    chatroomSprite.move(0, MOVEDISTANCE); 
                    view.move(0, MOVEDISTANCE); 
                    window.setView(view);
                    chatInputBox.move(0, MOVEDISTANCE);
                    changeView = true;
                }
                if(event.key.code == sf::Keyboard::Escape){
                    std::string message =  playerID + " left the chatroom.\nPress Esc back to map.";
                    Packet chatPacket(ESCMODE, playerID, requestReceiverName.c_str(), 0, 0, message.c_str());
                    TCPdata.sendData(chatPacket);
                    std::cout << "發送離開聊天室訊息" << std::endl;
                    chatHistory += message + "\n";  
                    changeView = true;
                    chatState = CHATSTATENONE;
                    sf::Time duration = chatClock.getElapsedTime();
                    float minutes = duration.asSeconds() / 60.0f;
                    char minutesStr[20];
                    sprintf(minutesStr, "%.1f", minutes);
                    std::cout << "minutes: " << minutesStr << " min" << std::endl;
                    Packet timePacket(TIMEMODE, playerID, "", 0, 0, minutesStr);
                    TCPdata.sendData(timePacket);
                }
            }
        }

        void chatRequestReceiveHandler(ClientConnectToServer &TCPdata, Character& mainCharacter){
            std::string message =  
            requestSenderName + ":\nrequests to chat!\n"
                              +   "        (y/n)       ";
            requestText.setString(message);
            if(chatState != CHATSTATERECV) {
                requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(requestSprite);
                requestText.setPosition(requestSprite.getPosition().x + 65, requestSprite.getPosition().y + 100);
                window.draw(requestText);
                return;
            };
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Y)){
                Packet chatRequestPacket(REQMODE, "", requestSenderName.c_str(), 0, 0, "Yes");
                TCPdata.sendData(chatRequestPacket);
                chatState = CHATSTATECHAT;
                chatStart();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                Packet chatRequestPacket(REQMODE, "", requestSenderName.c_str(), 0, 0, "No");
                TCPdata.sendData(chatRequestPacket);
                chatState = CHATSTATENONE;
            }
            sf::Vector2f viewSize = view.getSize();
            sf::Vector2f viewCenter = view.getCenter();
            float systemMessageX = viewCenter.x - viewSize.x / 2 + 10;
            float systemMessageY = viewCenter.y + viewSize.y / 2 - systemMessage.getCharacterSize();
            systemMessage.setPosition(systemMessageX, systemMessageY);
            window.draw(systemMessage);
        }

        void chatHandler(ClientConnectToServer &TCPdata){
            if(chatState != CHATSTATECHAT) return;
            while(1){
                Packet chatPacket = TCPdata.receiveDataNonBlock();
                if(chatPacket.mode_packet == EMPTYMODE) break;
                if(chatPacket.mode_packet == CHATMODE){
                    std::string senderName(chatPacket.sender_name);
                    std::string messageText(chatPacket.message);
                    chatHistory +=  senderName + "  :  " + messageText + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    //移動視角
                    chatroomSprite.move(0, MOVEDISTANCE); // 向上移动背景图
                    view.move(0, MOVEDISTANCE); // 向上移动视图
                    window.setView(view);
                    chatInputBox.move(0, MOVEDISTANCE);
                    changeView = true;
                }
                if(chatPacket.mode_packet == ESCMODE){
                    std::cout << "收到離開聊天室訊息" << std::endl;
                    std::string messageText(chatPacket.message);
                    chatHistory += messageText + "\n";
                    std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    //移動視角
                    chatroomSprite.move(0, 2 * MOVEDISTANCE); // 向上移动背景图
                    view.move(0, 2 * MOVEDISTANCE); // 向上移动视图
                    window.setView(view);
                    chatInputBox.move(0, 2 * MOVEDISTANCE);
                    changeView = true;
                }
            }
        }

        void chatDraw(Character& mainCharacter) {
            sf::Vector2f viewCenter = view.getCenter();
            sf::Vector2f viewSize = view.getSize();
            if(chatState == CHATSTATENONE or chatState == CHATSTATESEND){
                float systemMessageX = viewCenter.x - viewSize.x / 2 + 10;
                float systemMessageY = viewCenter.y + viewSize.y / 2 - systemMessage.getCharacterSize();
                systemMessage.setPosition(systemMessageX, systemMessageY);
                window.draw(systemMessage);
            }
            if(chatState == CHATSTATERECV){
                requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(requestSprite);
                requestText.setPosition(requestSprite.getPosition().x + 65, requestSprite.getPosition().y + 100);
                window.draw(requestText);
            }
            if(chatState == CHATSTATECHAT){
                sf::FloatRect spriteBounds = chatroomSprite.getGlobalBounds();
                sf::Vector2f spriteCenter(spriteBounds.left + spriteBounds.width / 2.0f,
                                        spriteBounds.top + spriteBounds.height / 2.0f);
                if(changeView == false){
                    view.setCenter(spriteCenter);
                    window.setView(view);
                    chatInputBox.setPosition(chatroomSprite.getPosition().x + spriteBounds.width / 3.3f,
                                chatroomSprite.getPosition().y + spriteBounds.height / 1.115f);
                    chatRecord.setPosition(chatroomSprite.getPosition().x + spriteBounds.width / 8.5f,
                                chatroomSprite.getPosition().y + spriteBounds.height / 1.14f);
                }
                
                chatRecord.setString(chatHistory);
                window.draw(chatRecord);
                window.draw(chatroomSprite);
                window.draw(chatInputBox);

                sf::Text enterInput(userInput, chatFont, 30);
                enterInput.setPosition(chatInputBox.getPosition().x + 5, chatInputBox.getPosition().y);
                enterInput.setFillColor(sf::Color::Black);
                window.draw(enterInput);
            }
        }

        void startTimer(){chatClock.restart();}
        sf::Time getChatTime(){return chatClock.getElapsedTime();}
};


void initClientBasicElements(){
    brown           = sf::Color(139, 69, 19);
    lavenderBlush   = sf::Color(255, 240, 245);
    if (!bgm.openFromFile("Assets/Musics/bgm.ogg")) {
        perror("音樂加載失敗");
    }
    if (!mapTexture.loadFromFile("Assets/Pictures/map.png")) {
        perror("地圖加載失敗");
        exit(-1);
    }
    if (!characterTexture.loadFromFile("Assets/Pictures/boy.png")){
        perror("主角圖片加載失敗");
        exit(-1);
    }
    if (!font.loadFromFile("Assets/Fonts/login_font.ttf")) {
        perror("字體加載失敗");
        exit(-1);
    }
    if (!chatFont.loadFromFile("Assets/Fonts/chat_font.ttf")) {
        perror("字體加載失敗");
        exit(-1);
    }
    mapSprite.setTexture(mapTexture);
    mapSprite.setScale(2.5f, 2.5f);

    systemMessage.setFont(font);
    systemMessage.setCharacterSize(50);
    systemMessage.setFillColor(lavenderBlush);

    window.create(sf::VideoMode(500, 500), "ChatBar");
    window.setFramerateLimit(60);
    
    bgm.setLoop(true);
    bgm.play();

    view.reset(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
}

float viewResize(sf::Event &event){
    float newWidth = static_cast<float>(event.size.width);
    float newHeight = static_cast<float>(event.size.height);
    float aspectRatio = newWidth / newHeight;
    sf::FloatRect visibleArea(0, 0, 800.f * aspectRatio, 600.f);
    view = sf::View(visibleArea);
    return aspectRatio;
}

int main(int argc, char** argv) {
    initClientBasicElements();
    playerID = argv[1];
    ClientConnectToServer TCPdata;
    TCPdata.serverIPPort(SERVERIP, GAMEPORT);
    sf::Vector2f initPosition(918, 847);
    Character mainCharacter(playerID, initPosition.x, initPosition.y);
    Packet mainCharacterPacket(MAPMODE, playerID, "", initPosition.x, initPosition.y, "");
    TCPdata.sendData(mainCharacterPacket);
    view.setCenter(mainCharacter.getPosition());
    OtherCharacters otherCharacters(TCPdata);
    ChatEnvironment chatEnvironment;
    TCPdata.turnOnNonBlock();
    Rank rank(font, playerID);

    float aspectRatio       = 0;
    bool isWindowFocused    = true;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)        window.close();
            if (event.type == sf::Event::GainedFocus)   isWindowFocused = true;
            if (event.type == sf::Event::LostFocus)     isWindowFocused = false; 
            if (event.type == sf::Event::Resized)       aspectRatio = viewResize(event);
            if (event.type == sf::Event::KeyPressed)    chatEnvironment.chatKeyHandler(event, TCPdata, isWindowFocused);
        }
        if (mainCharacter.mainCharacterMove()) {
            Packet packet(MAPMODE, playerID, "", mainCharacter.getPosition().x, mainCharacter.getPosition().y, "");
            TCPdata.sendData(packet);
            otherCharacters.updateOtherCharactersByMainCharacter(mainCharacter);
        }
        while(true){
            Packet clientReceivedPacket = TCPdata.receiveDataNonBlock();
            switch (clientReceivedPacket.mode_packet) {
                case EMPTYMODE:
                    break;
                case MAPMODE:
                    otherCharacters.updateOtherCharactersByPacket(mainCharacter, clientReceivedPacket);
                    break;
                case REQMODE:
                    chatEnvironment.chatReplyReceivedHandler(clientReceivedPacket);
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
        view.setCenter(mainCharacter.getPosition());
        window.setView(view);
        window.clear();
        window.draw(mapSprite);
        mainCharacter.draw();
        otherCharacters.drawAllOtherCharacters();
        rank.setPosition();
        rank.draw();
        chatEnvironment.chatDraw(mainCharacter);
        window.display();
    }
    return 0;
}
