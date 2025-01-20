# Makefile 示例

# 编译器和编译选项
CXX = /usr/bin/g++
CXXFLAGS = -Wall -std=c++17 -g

INCLUDE_DIRS = -I/usr/include -I/home/ciao/protobuf/include


# 链接器选项
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lboost_system -L/home/ciao/protobuf/lib -lprotobuf -lmysqlclient

# 源文件和目标文件
SRC = main.cpp player.pb.cpp cl.pb.cpp client.cpp
OBJ = $(SRC:.cpp=.o)

# 输出的可执行文件
TARGET = asio_test

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# 生成 .o 文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# 清理目标
clean:
	rm -f $(OBJ) $(TARGET)

# 伪目标
.PHONY: all clean install
