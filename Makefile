CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

SRC = src
BIN = chat_server
CLIENT = chat_client

all: $(BIN) $(CLIENT)

$(BIN): $(SRC)/main.cpp $(SRC)/server.cpp
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)/main.cpp $(SRC)/server.cpp

$(CLIENT): $(SRC)/client.cpp
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(SRC)/client.cpp

clean:
	rm -f $(BIN) $(CLIENT)
