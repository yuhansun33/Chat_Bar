CXX=g++
CXXFLAGS=-Wall -std=c++11 -g
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lmysqlcppconn -lcrypto -lsfml-audio 

LOGIN_OBJS=login.o elementTCP.o readline.o clientTCP.o

CLIENT_OBJS=readline.o clientTCP.o elementTCP.o clientMain.o

SERVERLOGIN_OBJS=readline.o elementTCP.o serverTCP.o serverLOGIN.o 

SERVERGAME_OBJS=readline.o elementTCP.o serverTCP.o serverGame.o 

all: main login serverLOGIN serverGame

login: login.o
	$(CXX) $(CXXFLAGS) -o login $(LOGIN_OBJS) $(LDFLAGS)

main: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o main $(CLIENT_OBJS) $(LDFLAGS)

serverLOGIN: $(SERVERLOGIN_OBJS)
	$(CXX) $(CXXFLAGS) -o serverLOGIN $(SERVERLOGIN_OBJS) $(LDFLAGS)

serverGame: $(SERVERGAME_OBJS)
	$(CXX) $(CXXFLAGS) -o serverGame $(SERVERGAME_OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o main server serverLOGIN serverGame login 

.PHONY: clean
