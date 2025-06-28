#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace std;

// Function to receive messages from the server
void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break; // Exit on disconnect or error
        buffer[bytes_received] = '\0'; // Null-terminate the message
        std::cout << "\n" << buffer;
        std::cout.flush();
    }
}

int main() {
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    // Configure server address
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // Server port

    // Convert IP address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;
    }

    // Connect to server
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return -1;
    }

    // Start thread to handle incoming messages
    std::thread receiver(receive_messages, sock);

    // Welcome message
    std::string welcomemsg = 
    "\033[1;36m\n" // Cyan bold
    "----------WELCOME TO SOCKET_CHAT----------\n"
    "\033[0m"       // Reset
    "This is a TCP-based socket chat app. Enter your name and start chatting :)\n"
    "\033[1;33mUse the command /help for commands\033[0m\n";

    cout << welcomemsg;

    // Prompt for user name
    string name = "NAME:";
    std::cout << "Enter your name" << endl;
    string temp;
    getline(std::cin, temp);
    name += temp;
    cout << name << endl;
    send(sock, name.c_str(), name.length(), 0);

    // Main message loop
    std::string msg;
    while (true) {
        std::cout << "\033[1;32m> \033[0m"; 
        std::getline(std::cin, msg);
        send(sock, msg.c_str(), msg.length(), 0);
        if (msg == "/exit") {
            break;
        }
    }

    // Cleanup
    close(sock);
    receiver.detach(); // Detach receiver thread
    return 0;
}