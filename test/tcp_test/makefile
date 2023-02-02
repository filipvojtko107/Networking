CC = g++
CC_FLAGS = -std=c++17 -Wall -pthread
DEFAULT_PATH = ../../
TCP_PATH = ../../tcp/
CPP_FILES = parametres.cpp $(TCP_PATH)thread_pool.cpp $(DEFAULT_PATH)socket.cpp $(DEFAULT_PATH)networking_error.cpp $(DEFAULT_PATH)netbase.cpp $(TCP_PATH)tcp*.cpp
SERVER_CPP_FILE = server.cpp
CLIENT_CPP_FILE = client.cpp
SERVER_TARGET = server
CLIENT_TARGET = client
ALL_TARGETS = $(SERVER_TARGET) $(CLIENT_TARGET)


all: $(ALL_TARGETS)


$(SERVER_TARGET): $(CPP_FILES) $(SERVER_CPP_FILE)
	$(CC) -I$(DEFAULT_PATH) -I$(TCP_PATH) $(CC_FLAGS) $(CPP_FILES) $(SERVER_CPP_FILE) -o $(SERVER_TARGET)


$(CLIENT_TARGET): $(CPP_FILES) $(CLIENT_CPP_FILE)
	$(CC) -I$(DEFAULT_PATH) -I$(TCP_PATH) $(CC_FLAGS) $(CPP_FILES) $(CLIENT_CPP_FILE) -o $(CLIENT_TARGET)


clean:
	rm -f $(ALL_TARGETS)
