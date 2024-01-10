#include "header.h"
#include "clientTCP.h"
std::string             playerID;
sf::View                view;
sf::Color               lavenderBlush;
sf::Color               brown;
sf::Texture             chatRoomIconTexture;
sf::Texture             mapTexture;
sf::Texture             characterTexture;
sf::Font                font;
sf::Font                chatFont;
sf::Music               bgm;
sf::Sprite              chatRoomIconSprite;
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
        bool mainCharacterMove(bool isWindowFocused) {
            if(!isWindowFocused) return false;
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
class ChatRoomIcon{
    private:
        sf::Vector2f chatRoomIconPostion; 
        std::string chatRoomIconID;
        float distance;
    public:
        ChatRoomIcon(){
            chatRoomIconPostion.x = NOWHERE;
            chatRoomIconPostion.y = NOWHERE;
            distance = INFINDISTANCE;

        }
        ChatRoomIcon(Packet& chatRoomIconPacket, Character& mainCharacter){
            if(!chatRoomIconTexture.loadFromFile("Assets/Pictures/chatroomicon.png")){
                perror("chatroom圖片加載失敗");
                exit(-1);
            }
            chatRoomIconID = chatRoomIconPacket.sender_name;
            chatRoomIconPostion = sf::Vector2f(chatRoomIconPacket.x_packet, chatRoomIconPacket.y_packet);
            chatRoomIconSprite.setPosition(chatRoomIconPostion);
            // std::cout << "CHATROOMICON: " << chatRoomIconPostion.x << " " << chatRoomIconPostion.y << std::endl;
            refreshDistance(mainCharacter);
        }
        ~ChatRoomIcon(){};
        void draw(){
            sf::Vector2f nowhere = sf::Vector2f(NOWHERE, NOWHERE);
            if(chatRoomIconPostion != nowhere) window.draw(chatRoomIconSprite);
        }
        float getDistance(){
            return distance;
        }
        float refreshDistance(Character& character){
            return distance = sqrt(pow(character.getPosition().x - chatRoomIconPostion.x, 2) + pow(character.getPosition().y - chatRoomIconPostion.y, 2));
        }
        void sendChatRoomRequest(sf::Event& event, ClientConnectToServer& TCPdata){
            if(event.key.code == sf::Keyboard::Z){
                Packet chatRequestPacket(JOINMODE, playerID, chatRoomIconID, 0, 0, "");
                TCPdata.sendData(chatRequestPacket);
            }
        }
        std::string getChatRoomIconIDStr(){
            return chatRoomIconID;
        }
        int getChatRoomIconIDInt(){
            return stoi(chatRoomIconID);
        }
};

class ChatRoomIcons{
    private:
        float minDistance;
        ChatRoomIcon minDistanceChatRoomIcon;
        std::unordered_map<std::string, ChatRoomIcon> chatRoomIcons;
    public:
        ChatRoomIcons(Packet& initChatRoomPacket, ClientConnectToServer& TCPdata, Character& mainCharacter){
            minDistance = INFINDISTANCE;
            if(initChatRoomPacket.mode_packet != INITMODE) return;
            int otherCharacterNumber = (int)initChatRoomPacket.y_packet;
            for(int i = 0 ; i < otherCharacterNumber ; i++) {
                Packet packet = TCPdata.receiveData();
                if (packet.mode_packet == ROOMMODE) {
                    ChatRoomIcon chatRoomIcon(packet, mainCharacter);
                    chatRoomIcons[chatRoomIcon.getChatRoomIconIDStr()] = chatRoomIcon;
                }
            }
        }
        ~ChatRoomIcons(){};
        void draw(){
            for(auto& chatRoomIcon : chatRoomIcons) chatRoomIcon.second.draw();
        }
        void updateChatRoomIconByPacket(Packet& chatRoomIconPacket, Character& mainCharacter){
            std::string chatRoomIconID(chatRoomIconPacket.sender_name);
            if(chatRoomIconPacket.x_packet == NOWHERE && chatRoomIconPacket.y_packet == NOWHERE){
                if(chatRoomIconID == minDistanceChatRoomIcon.getChatRoomIconIDStr()){
                    chatRoomIcons.erase(chatRoomIconID);
                    refreshAllChatRoomDistance(mainCharacter);
                }else{
                    chatRoomIcons.erase(chatRoomIconID);
                }
                return;
            }
            minDistance = INFINDISTANCE;
            ChatRoomIcon chatRoomIcon(chatRoomIconPacket, mainCharacter);
            chatRoomIcons[chatRoomIconID] = chatRoomIcon;

            if(chatRoomIcon.getDistance() < minDistance){
                minDistance = chatRoomIcon.getDistance();
                minDistanceChatRoomIcon = chatRoomIcon;
            }
        }
        void refreshAllChatRoomDistance(Character& mainCharacter){
            minDistance = INFINDISTANCE;
            for(auto& chatRoomIcon : chatRoomIcons){
                chatRoomIcon.second.refreshDistance(mainCharacter);
                if(chatRoomIcon.second.getDistance() < minDistance){
                    minDistance = chatRoomIcon.second.getDistance();
                    minDistanceChatRoomIcon = chatRoomIcon.second;
                }
            }
        }
        float getMinDistance(){
            return minDistance;
        }
        std::string getMinDistanceChatRoomIcon(){
            return minDistanceChatRoomIcon.getChatRoomIconIDStr();
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
        }
        float getDistance() {
            return distance;
        }
};


class OtherCharacters{
    private:
        float minDistance;
        std::string minDistanceCharacterName;
        std::unordered_map<std::string, OtherCharacter> otherCharactersMap;
    public:
        OtherCharacters(Packet& initOtherCharacterPacket, ClientConnectToServer &TCPdata){
            minDistance = INFINDISTANCE;
            if(initOtherCharacterPacket.mode_packet != INITMODE) return;
            int otherCharacterNumber = (int)initOtherCharacterPacket.x_packet;
            for(int i = 0 ; i < otherCharacterNumber ; i++) {
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
            if(otherCharacterUpdatePacket.sender_name == playerID){
                return;
            }
            if(otherCharactersMap.find(otherCharacterUpdatePacket.sender_name) == otherCharactersMap.end()){
                OtherCharacter otherCharacter(characterTexture, font, otherCharacterUpdatePacket.sender_name, otherCharacterUpdatePacket.x_packet, otherCharacterUpdatePacket.y_packet, mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                otherCharactersMap[otherCharacterUpdatePacket.sender_name] = otherCharacter;
            } else {
                otherCharactersMap[otherCharacterUpdatePacket.sender_name].setPosition(otherCharacterUpdatePacket.x_packet, otherCharacterUpdatePacket.y_packet);
                otherCharactersMap[otherCharacterUpdatePacket.sender_name].refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
            }
            if(otherCharactersMap[otherCharacterUpdatePacket.sender_name].getDistance() < minDistance) {
                minDistance = otherCharactersMap[otherCharacterUpdatePacket.sender_name].getDistance();
                minDistanceCharacterName = otherCharacterUpdatePacket.sender_name;
            }
            if(otherCharactersMap[minDistanceCharacterName].getDistance() != minDistance) updateOtherCharactersByMainCharacter(mainCharacter);
        }
        void updateOtherCharactersByMainCharacter(Character& mainCharacter){
            minDistance = INFINDISTANCE;
            for(auto& otherCharacter : otherCharactersMap){
                otherCharacter.second.refreshDistance(mainCharacter.getPosition().x, mainCharacter.getPosition().y);
                if(otherCharacter.second.getDistance() < minDistance) {
                    minDistance = otherCharacter.second.getDistance();
                    minDistanceCharacterName = otherCharacter.first;
                }
            }
        }
        float getMinDistance(){
            return minDistance;
        }
        std::string getMinDistanceCharacterName(){
            return minDistanceCharacterName;
        }
};

class ChatEnvironment{
    private:
        int clockSeconds;
        int chatState;
        float minDistance;
        bool isFriendRequstCancealed;
        bool changeView;
        std::string chatTimeStr;
        std::string requestSenderName;
        std::string requestReceiverName;
        sf::Texture requestTexture;
        sf::Texture chatroomTexture;
        sf::Texture chatClockTexture;

        sf::Sprite requestSprite;
        sf::Sprite chatroomSprite;
        sf::Sprite chatClockSprite;
        sf::Clock chatClock;
        sf::Text requestText;
        sf::Text chatRecord;
        sf::Text chatTimeText;
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
            if(!chatClockTexture.loadFromFile("Assets/Pictures/chatclock.png")){
                perror("chatclock圖片加載失敗");
                exit(-1);
            }
            if(!chatRoomIconTexture.loadFromFile("Assets/Pictures/chatroomicon.png")){
                perror("chatroom圖片加載失敗");
                exit(-1);
            }
            chatRoomIconSprite.setTexture(chatRoomIconTexture);
            chatRoomIconSprite.setScale(0.15f, 0.15f);
            chatInputBox.setSize(sf::Vector2f(300, 35));
            chatInputBox.setFillColor(sf::Color::White);
            requestSprite.setTexture(requestTexture);
            requestSprite.setScale(0.5f, 0.5f);
            chatroomSprite.setTexture(chatroomTexture);
            chatroomSprite.setScale(0.45f, 0.35f);
            chatTimeText.setFont(chatFont);
            chatTimeText.setCharacterSize(50);
            chatTimeText.setFillColor(sf::Color::White);
            chatTimeText.setPosition(650, 80);
            chatClockSprite.setTexture(chatClockTexture);
            chatClockSprite.setScale(0.2f, 0.15f);
            chatClockSprite.setPosition(550, 73);
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
            minDistance = INFINDISTANCE;
        }

        int getChatState(){ return chatState; }

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

        void setRequestReceiverName(std::string name){
            requestReceiverName = name;
        }

        void chatStart(){
            changeView = false;
            chatClock.restart();
            chatHistory.clear();
            userInput.clear();
        }

        void chatTextHandler(sf::Event& event, bool isWindowFocused){
            if(!isWindowFocused) return;
            if(chatState != CHATSTATECHAT) return;
            if(event.text.unicode == '\b' && !userInput.empty()){
                userInput.pop_back();
            }else if(event.text.unicode >= 32 && event.text.unicode <= 126){
                userInput += static_cast<char>(event.text.unicode);
            }            
        }

        void chatKeyHandler(sf::Event& event,  ClientConnectToServer& TCPdata, bool isWindowFocused) {
            if(!isWindowFocused) return;
            if(chatState == CHATSTATENONE){
                if(event.key.code == sf::Keyboard::X){
                    Packet chatRequestPacket(REQMODE, playerID, requestReceiverName.c_str(), 0, 0, "first request");
                    TCPdata.sendData(chatRequestPacket);
                    chatState = CHATSTATESEND;
                }
                if(event.key.code == sf::Keyboard::Z){
                    Packet chatRequestPacket(JOINMODE, playerID, requestReceiverName.c_str(), 0, 0, "second request");
                    TCPdata.sendData(chatRequestPacket);
                    chatState = CHATSTATESEND;
                }
            }
            else if(chatState == CHATSTATECHAT){
                if(event.key.code == sf::Keyboard::Enter){
                    if(userInput.empty()) return;
                    Packet chatPacket(CHATMODE, playerID, requestReceiverName.c_str(), 0, 0, userInput.c_str());
                    TCPdata.sendData(chatPacket);
                    chatHistory += playerID + "  :  " + userInput + "\n";
                    // std::cout << "chatHistory: \n" << chatHistory << std::endl;
                    userInput.clear();
                    chatMoveView();
                }
                if(event.key.code == sf::Keyboard::Escape){
                    std::string message =  playerID + " left the chatroom.\nPress Esc back to map.";
                    Packet chatPacket(ESCMODE, playerID, requestReceiverName.c_str(), 0, 0, message.c_str());
                    TCPdata.sendData(chatPacket);
                    // std::cout << "發送離開聊天室訊息" << std::endl;
                    chatHistory += message + "\n";  
                    changeView = true;
                    chatState = CHATSTATENONE;
                    sf::Time duration = chatClock.getElapsedTime();
                    float minutes = duration.asSeconds() / 60.0f;
                    char minutesStr[20];
                    sprintf(minutesStr, "%.1f", minutes);
                    // std::cout << "minutes: " << minutesStr << " min" << std::endl;
                    Packet timePacket(TIMEMODE, playerID, "", 0, 0, minutesStr);
                    TCPdata.sendData(timePacket);
                }
                if(event.key.code == sf::Keyboard::BackSpace){
                    if(userInput.empty()) return;
                    userInput.pop_back();
                }
            }
        }

        void chatRequestReceiveHandler(ClientConnectToServer &TCPdata, Character& mainCharacter){
            std::string message =  
            requestSenderName + ":\nRequests to Chat!\n"
                              +   "        (Y/N)       ";
            requestText.setString(message);
            if(chatState != CHATSTATERECV) {
                requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(requestSprite);
                requestText.setPosition(requestSprite.getPosition().x + 65, requestSprite.getPosition().y + 100);
                window.draw(requestText);
                return;
            };
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Y)){
                Packet chatRequestPacket(REQMODE, playerID, requestSenderName.c_str(), 0, 0, "Yes");
                TCPdata.sendData(chatRequestPacket);
                chatState = CHATSTATECHAT;
                chatStart();
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
                Packet chatRequestPacket(REQMODE, playerID, requestSenderName.c_str(), 0, 0, "No");
                TCPdata.sendData(chatRequestPacket);
                chatState = CHATSTATENONE;
            }
        }

        void chatHandlerEsc(Packet& chatPacket, ClientConnectToServer &TCPdata){
            if(chatState != CHATSTATECHAT) return;
            // std::cout << "收到離開聊天室訊息" << std::endl;
            std::string messageText(chatPacket.message);
            chatHistory += messageText + "\n";
            // std::cout << "chatHistory: \n" << chatHistory << std::endl;
            chatMoveView(2*MOVEDISTANCE);
        }

        void chatHandlerChat(Packet& chatPacket, ClientConnectToServer &TCPdata){
            if(chatState != CHATSTATECHAT) return;
            std::string senderName(chatPacket.sender_name);
            std::string messageText(chatPacket.message);
            chatHistory +=  senderName + "  :  " + messageText + "\n";
            chatMoveView();
        }

        void chatMoveView(float distance = MOVEDISTANCE){
            chatroomSprite.move(0, distance);
            chatTimeText.move(0, distance);
            chatClockSprite.move(0, distance);
            chatInputBox.move(0, distance);
            view.move(0, distance); 
            window.setView(view);
            changeView = true;
        }

        void chatDraw(Character& mainCharacter) {
            sf::Vector2f viewCenter = view.getCenter();
            sf::Vector2f viewSize = view.getSize();
            if(chatState == CHATSTATERECV){
                requestSprite.setPosition(mainCharacter.getPosition().x - 150, mainCharacter.getPosition().y - 150);
                window.draw(requestSprite);
                requestText.setPosition(requestSprite.getPosition().x + 65, requestSprite.getPosition().y + 100);
                window.draw(requestText);
            }
            if(chatState == CHATSTATECHAT){
                window.clear();
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
                getChatTime();
                chatRecord.setString(chatHistory);
                window.draw(chatRecord);
                window.draw(chatroomSprite);
                window.draw(chatInputBox);
                sf::Text enterInput(userInput, chatFont, 30);
                enterInput.setPosition(chatInputBox.getPosition().x + 5, chatInputBox.getPosition().y);
                enterInput.setFillColor(sf::Color::Black);
                window.draw(enterInput);
                window.draw(chatTimeText);
                window.draw(chatClockSprite);
                // if(clockSeconds > 60 and !isFriendRequstCancealed) window.draw(friendSendText);
            }
        }

        void startTimer(){chatClock.restart();}
        sf::Time getChatTime(){
            sf::Time timeCount = chatClock.getElapsedTime();
            clockSeconds = static_cast<int>(timeCount.asSeconds());
            chatTimeStr = std::to_string(clockSeconds) + "sec";
            chatTimeText.setString(chatTimeStr);
            return timeCount;
        }
        
        void chatMinDistanceReciever(float chatRoomMinDistance, float characterMinDistance, std::string chatRoomMinName, std::string characterMinName){
            if(chatRoomMinDistance <= characterMinDistance){
                requestReceiverName = chatRoomMinName;
                minDistance = chatRoomMinDistance;
            }
            else{
                requestReceiverName = characterMinName;
                minDistance = characterMinDistance;
            }
            if(systemMessageUpdate()) return;
            if(minDistance == chatRoomMinDistance){
                std::string message =  "Press Z to join the ChatRoom!";
                systemMessage.setString(message);
                return;
            }
            if(minDistance == characterMinDistance){
                if(characterMinName == "\n"){
                    systemMessage.setString("");
                    return;
                }
                std::string message =  "Press X to ask " + characterMinName + " to ChatBar!";
                systemMessage.setString(message);
            }
        }

        bool systemMessageUpdate(){
           if(minDistance > CHATDISTANCE) {
                systemMessage.setString("");
                return 1;
            }
            else return 0;
        }

        void systemMessageDraw(){
            systemMessageUpdate();
            sf::Vector2f viewSize = view.getSize();
            sf::Vector2f viewCenter = view.getCenter();
            float systemMessageX = viewCenter.x - viewSize.x / 2 + 10;
            float systemMessageY = viewCenter.y + viewSize.y / 2 - systemMessage.getCharacterSize();
            systemMessage.setPosition(systemMessageX, systemMessageY);
            window.draw(systemMessage);
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

class Friend{
    private:
        sf::Texture friendReceiveTexture;
        sf::Texture friendChecklistTexture;
        sf::Sprite friendReceiveSprite;
        sf::Sprite friendChecklistSprite;
        sf::Text friendSendText;
        int friendRequestState;
    public:
        Friend(){
            if(!friendReceiveTexture.loadFromFile("Assets/Pictures/friendrequest.png")){
                perror("friend圖片加載失敗");
                exit(-1);
            }
            if(!friendChecklistTexture.loadFromFile("Assets/Pictures/request.png")){
                perror("friend圖片加載失敗");
                exit(-1);
            }
            friendChecklistSprite.setTexture(friendChecklistTexture);
            friendChecklistSprite.setScale(0.5f, 0.5f);
            friendChecklistSprite.setPosition(100, 100);
            friendReceiveSprite.setTexture(friendReceiveTexture);
            friendReceiveSprite.setScale(0.4f, 0.3f);
            friendReceiveSprite.setPosition(100, 100);
            friendSendText.setFont(chatFont);
            friendSendText.setCharacterSize(30);
            friendSendText.setFillColor(sf::Color::White);
            friendSendText.setPosition(150, 150);
            friendSendText.setString("Press Ctrl+F to open roomlist or Press Ctrl+I to ignore it!");
            friendRequestState = FRI_REQ_UNQULIF;
        }

        void friendRequestTimeCheck(int clockSeconds){
            if(clockSeconds > 60) friendRequestState = FRI_REQ_PENDING;
        }

        void friendRequestKeyHandler(sf::Event& event, ClientConnectToServer& TCPdata){
            if(friendRequestState != FRI_REQ_PENDING) return;
            if(event.key.code == sf::Keyboard::F && event.key.control){
                friendRequestState = FRI_REQ_KEYSENT;
                Packet friendRequestPacket(FRIENDMODE, playerID, "", 0, 0, "friend request");
                TCPdata.sendData(friendRequestPacket);
            }
            if(event.key.code == sf::Keyboard::I && event.key.control){
                friendRequestState = FRI_REQ_KEYDENY;
            }
        }

        void friendReplyHandler(sf::Event& event, ClientConnectToServer& TCPdata, Packet& friendReplyPacket){
            if(friendRequestState == FRI_REQ_UNQULIF) return;
            if(friendRequestState == FRI_REQ_KEYSENT){
                if(event.key.code == sf::Keyboard::Y){
                    friendRequestState = FRI_REQ_PENDING;
                    Packet friendRequestPacket(FRIENDMODE, playerID, friendReplyPacket.sender_name, 0, 0, "friend request accept");
                    TCPdata.sendData(friendRequestPacket);
                }
                if(event.key.code == sf::Keyboard::N){
                    friendRequestState = FRI_REQ_PENDING;
                    Packet friendRequestPacket(FRIENDMODE, playerID, "", 0, 0, "friend request deny");
                    TCPdata.sendData(friendRequestPacket);
                }

            }
        }

};
void initClientBasicElements(){
    brown           = sf::Color(139, 69, 19);
    lavenderBlush   = sf::Color(255, 240, 245);
    if (!bgm.openFromFile("Assets/Musics/bgm.ogg")) {
        perror("音樂加載失敗");
        exit(-1);
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
    //bgm.play();

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
    Packet initPacket = TCPdata.receiveData();
    OtherCharacters otherCharacters(initPacket,TCPdata);
    ChatRoomIcons chatRoomIcons(initPacket, TCPdata, mainCharacter);
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
            if (event.type == sf::Event::TextEntered)   chatEnvironment.chatTextHandler(event, isWindowFocused);
        }
        while(true){
            Packet clientReceivedPacket = TCPdata.receiveDataNonBlock();
            std::string minDistanceCharacterName;
            if(clientReceivedPacket.mode_packet == EMPTYMODE){
                break;
            }
            if(clientReceivedPacket.mode_packet == MAPMODE){
                otherCharacters.updateOtherCharactersByPacket(mainCharacter, clientReceivedPacket);
                chatEnvironment.chatMinDistanceReciever(chatRoomIcons.getMinDistance(), otherCharacters.getMinDistance(), chatRoomIcons.getMinDistanceChatRoomIcon(), otherCharacters.getMinDistanceCharacterName());
            }
            if(clientReceivedPacket.mode_packet == ROOMMODE){
                chatRoomIcons.updateChatRoomIconByPacket(clientReceivedPacket, mainCharacter);
                chatEnvironment.chatMinDistanceReciever(chatRoomIcons.getMinDistance(), otherCharacters.getMinDistance(), chatRoomIcons.getMinDistanceChatRoomIcon(), otherCharacters.getMinDistanceCharacterName());
            }
            if(clientReceivedPacket.mode_packet == REQMODE){
                chatEnvironment.chatReplyReceivedHandler(clientReceivedPacket);
            }
            if(clientReceivedPacket.mode_packet == TIMEMODE){
                rank.updateTime(clientReceivedPacket.message);
            }
            if(clientReceivedPacket.mode_packet == RANKMODE){
                rank.updateRank(clientReceivedPacket.sender_name, clientReceivedPacket.message);
            }
            if(clientReceivedPacket.mode_packet == CHATMODE){
                chatEnvironment.chatHandlerChat(clientReceivedPacket, TCPdata);
            }
            if(clientReceivedPacket.mode_packet == ESCMODE){
                chatEnvironment.chatHandlerEsc(clientReceivedPacket, TCPdata);
            }
        }
        if(chatEnvironment.getChatState() == CHATSTATECHAT) goto chat;
        if (mainCharacter.mainCharacterMove(isWindowFocused)) {
            Packet packet(MAPMODE, playerID, "", mainCharacter.getPosition().x, mainCharacter.getPosition().y, "");
            TCPdata.sendData(packet);
            otherCharacters.updateOtherCharactersByMainCharacter(mainCharacter);
            chatRoomIcons.refreshAllChatRoomDistance(mainCharacter);
            chatEnvironment.chatMinDistanceReciever(chatRoomIcons.getMinDistance(), otherCharacters.getMinDistance(), chatRoomIcons.getMinDistanceChatRoomIcon(), otherCharacters.getMinDistanceCharacterName());
        }
        chatEnvironment.chatRequestReceiveHandler(TCPdata, mainCharacter);
        view.setCenter(mainCharacter.getPosition());
        window.setView(view);
        window.clear();
        window.draw(mapSprite);
        mainCharacter.draw();
        otherCharacters.drawAllOtherCharacters();
        rank.setPosition();
        rank.draw();
        chatRoomIcons.draw();
        chatEnvironment.systemMessageDraw();
chat:
        chatEnvironment.chatDraw(mainCharacter);
        window.display();
    }
    return 0;
}
