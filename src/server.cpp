#include <iostream>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include "../include/server.h"

using namespace std;

// std::vector<int> clients;
unordered_map<int,string>clients;
//We can implement hash map here mapping client fd with client name


std::mutex clients_mutex;

void broadcast(const std::string& message, int sender_fd) {
    
    std::lock_guard<std::mutex> lock(clients_mutex);


    //modify the for loop based on the modification to the client data structure
    for (auto client : clients) {
        if (client.first != sender_fd) {
            send(client.first, message.c_str(), message.size(), 0);
        }
    }
}


void handle_client(int client_socket) {
    char buffer[1024];

    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;

        std::string message(buffer, bytes_read);

        std::string tagged;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            tagged = "[" + clients[client_socket] + "]: " + message;
        }

        std::cout << tagged;
        broadcast(tagged, client_socket);
    }

    close(client_socket);
    std::string name;
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        name = clients[client_socket];
        clients.erase(client_socket);
    }

    std::string leave_msg = name + " left the chat.\n";
    broadcast(leave_msg, client_socket);
    std::cout << leave_msg;
}


void setup_client(int client_socket) {
    char buffer[1024];
    int bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    std::string msg(buffer, bytes_read);
    std::string name = "Anonymous";

    if (msg.rfind("NAME:", 0) == 0) {
        name = msg.substr(5);
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = name;
    }

    std::string welcome = name + " joined the chat.\n";
    broadcast(welcome, client_socket);
    std::cout << welcome;

    std::thread(handle_client, client_socket).detach();
}


void start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); //make a socket and obtain its file descriptor
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //configure some socket options

    sockaddr_in address{}; //instantiate the internet socket addr struct as empty

    //configure the struct
    address.sin_family = AF_INET; //IPV4
    address.sin_addr.s_addr = INADDR_ANY; //any IP
    address.sin_port = htons(port); //port number

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 5);
    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        socklen_t addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        std::thread(setup_client, client_socket).detach();
    }
}
