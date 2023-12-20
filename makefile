CXX=g++
CXXFLAGS=-Wall -std=c++11
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system
# 如果json库不是头文件只需要包含，而需要链接二进制库，也应该在这里添加 -ljson

# 添加SFML和json库头文件的路径，如果它们不在标准路径中
# 如果在标准路径中可以省略-I选项
# CXXFLAGS+=-I/path/to/SFML/include -I/path/to/json/include

# 客户端对象文件
CLIENT_OBJS=clientTCP.o elementTCP.o test.o

# 服务器端对象文件
SERVER_OBJS=elementTCP.o server.o

# 默认目标
all: main server

# 客户端可执行文件
main: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o main $(CLIENT_OBJS) $(LDFLAGS)

<<<<<<< HEAD
# 服务器端可执行文件
server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o server $(SERVER_OBJS) $(LDFLAGS)
=======
server: server.cpp
	$(CC) $(CFLAGS) $< -o $@ $(SFML_LIBS)
>>>>>>> b5ad52e8e52ca03def7bec5cefbcef0ddd861fca

# 模式规则来构建任何需要的对象文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理构建文件
clean:
	rm -f *.o main server

# 伪目标，确保 make clean 总是执行
.PHONY: clean
