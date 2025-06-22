#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace std;

void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';
        std::cout << "\n[Server]: " << buffer;
        std::cout.flush();
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;
    }

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return -1;
    }

    std::thread receiver(receive_messages, sock);
    string name="NAME:";
    
    std::cout << "Enter your name"<< endl ;
    string temp;
    getline(std::cin,temp);
    name += temp;
    cout<<name<<endl;
    send(sock, name.c_str(), name.length(), 0);

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        if (msg == "exit") break;
        send(sock, msg.c_str(), msg.length(), 0);
    }

    close(sock);
    receiver.detach();
    return 0;
}
