# Compiler settings
CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g
LDFLAGS = -pthread -lmysqlclient

# Target executable
TARGET = slim-web-server  # Changed from bin/slim-web-server to current directory

# Source directories
BLOCK_DEQUE_DIR = src/block_deque
BUFFER_DIR = src/buffer
HTTP_DIR = src/http
LOG_DIR = src/log
SERVER_DIR = src/server
SQL_DIR = src/sql_connect
THREAD_POOL_DIR = src/thread_pool
TIMER_DIR = src/timer

# Object files directory
OBJ_DIR = obj

# Source and object files
SOURCES = $(wildcard $(LOG_DIR)/*.cpp $(THREAD_POOL_DIR)/*.cpp $(TIMER_DIR)/*.cpp \
          $(HTTP_DIR)/*.cpp $(SERVER_DIR)/*.cpp $(BUFFER_DIR)/*.cpp \
          $(BLOCK_DEQUE_DIR)/*.cpp $(SQL_DIR)/*.cpp src/main.cpp)
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# Build all components
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET)
	find $(OBJ_DIR) -name "*.o" -type f -delete
	rm -rf $(OBJ_DIR)
