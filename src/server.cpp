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

//for unicasting we can implement a reverse lookup table of clients
unordered_map<string,int>who_is;


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

void unicast(const string& message,int receiver_fd){
    // string messageToBeSent = clients[sender_fd]+message; //can be processed before
    lock_guard<mutex> lock(clients_mutex);
    send(receiver_fd,message.c_str(), message.size(), 0);
}

int find_msg_position(const string& message) {
    string::size_type pos = 6; //to match with the fisrt comparison
    while (pos < message.length() && message[pos] != ' ') {
        pos++;
    }
    return static_cast<int>(pos + 1); 
}



void handle_client(int client_socket) {
    char buffer[1024];

    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;
        

        std::string message(buffer, bytes_read);

        string command = message.substr(0,5);
        
        if(command == "/mesg"){
            int position = find_msg_position(message);
            if(position > (static_cast<int> (message.size()))){
                string error = "[Server]: Insufficient parameters (/mesg NAME MESSAGE) \n";
                //send invalid name error msg
                lock_guard<mutex> lock(clients_mutex);//dont forget to lock
                send(client_socket,error.c_str(), error.size(), 0);

                continue;
            }
            string name = message.substr(6,(position-7));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            name.erase(0, name.find_first_not_of(" \t\r\n"));

            if(who_is.find(name)!= who_is.end()){
                unicast("\033[1;33m" + clients[client_socket] + "\033[0m: " + message.substr(position), who_is[name]);
            }
            else{
                string error = "[Server]: Invalid name, try /list command to see users\n";
                //send invalid name error msg
                lock_guard<mutex> lock(clients_mutex);//dont forget to lock
                send(client_socket,error.c_str(), error.size(), 0);
            }
        }else if(command == "/list"){
            string list="";
            lock_guard<mutex> lock(clients_mutex);//dont forget to lock
            for(auto client:clients){
                if(client.first==client_socket) list.append(">");
                list.append(client.second+"\n");
            }
            list.append("\n");
            send(client_socket,list.c_str(), list.size(), 0);
        }else if (command == "/help") {
            string help = 
                "\033[1;36mAvailable commands:\033[0m\n"
                "\033[1;33m/list         \033[0m - \033[0;37mList all connected users\033[0m\n"
                "\033[1;32m/mesg NAME MSG\033[0m - \033[0;37mSend a private message to NAME\033[0m\n"
                "\033[1;31m/exit         \033[0m - \033[0;37mLeave the chatroom\033[0m\n"
                "\033[1;34m/help         \033[0m - \033[0;37mShow this help message\033[0m\n\n"
            ;

            send(client_socket, help.c_str(), help.size(), 0);
        }else if (command == "/exit") {
            string exitmsg = 
                "\033[1;32m\nThank you for using SocketChat!\033[0m\n"
                "\033[1;36mWe hope to see you again soon. Goodbye!\033[0m\n";
            send(client_socket, exitmsg.c_str(), exitmsg.size(), 0);
            break;
        }else{
            std::string tagged;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                tagged = "\033[1;35m[" + clients[client_socket] + "]\033[0m: " + message;
            }
            std::cout << tagged;
            broadcast(tagged, client_socket);

        }

    }

    close(client_socket);
    std::string name;
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        name = clients[client_socket];
        clients.erase(client_socket);
        who_is.erase(name);
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
    if(clients.find(client_socket)==clients.end()){
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[client_socket] = name;
            who_is[name]=client_socket;
        }
        std::string welcome = name + " joined the chat.\n";
        broadcast(welcome, client_socket);
        std::cout << welcome;
    }
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
