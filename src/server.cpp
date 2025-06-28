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
//We can implement hash map here mapping client fd with client name
unordered_map<int,string>clients;

//for unicasting we can implement a reverse lookup table of clients
unordered_map<string,int>who_is;

//mutex locks to synchronize threads updating clients, who_is or those which are sending messages
mutex clients_mutex;



void broadcast(const std::string& message, int sender_fd) ;
void unicast(const string& message,int receiver_fd) ;
int find_msg_position(const string& message) ;
void handle_client(int client_socket) ;
void setup_client(int client_socket) ;




void start_server(int port) {
    //1. Make a socket and obtain its file descriptor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
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


    //2. Bind the socket with the IP and port using the sockkaddr_in struct and socket's file descriptor 
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    //3. Start listening for connections
    listen(server_fd, 5);
    std::cout << "Server listening on port " << port << std::endl;


    //busy wait to accept clients...
    while (true) {
        socklen_t addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen); //accepting clients
        //accept assigns a socket for the current client request so that the server socket can continue listening
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        std::thread(setup_client, client_socket).detach(); // set up and handle client on a different thread
    }
}










//function for broadcasting
void broadcast(const std::string& message, int sender_fd) {
    
    lock_guard<mutex> lock(clients_mutex);
    for (auto client : clients) {
        if (client.first != sender_fd) {
            send(client.first, message.c_str(), message.size(), 0); //sending message except for the sender
        }
    }

}

//function for unicasting
void unicast(const string& message,int receiver_fd){
    lock_guard<mutex> lock(clients_mutex);
    send(receiver_fd,message.c_str(), message.size(), 0);

}

/*
A function useful for parsing [extracts starting position of the actual message content]
This function is used specefically when there is a argument after a command.
Currently, it's only used for parsing /mesg command
*/
int find_msg_position(const string& message) {

    //indices 0-4 will be command, 5 should be whitespace
    //From index 6 it will be argument + message content

    string::size_type pos = 6; //to match with the message.length() comparison

    while (pos < message.length() && message[pos] != ' ') {
        pos++;
    }
    return static_cast<int>(pos + 1); 
}

/*
Function to hnadle client messages once they are set up
*/

void handle_client(int client_socket) {
    char buffer[1024]; //for storing message

    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer)); //reading
        if (bytes_read <= 0) continue;


        string message(buffer, bytes_read);

        string command = message.substr(0,5);//extract command 

        /*Bunch of if-else branches to evaluate and respond to commands*/
        if(command == "/mesg"){

            int position = find_msg_position(message);//finds position of message

            //if any of the arguments are missing send error message and continue
            if(position > (static_cast<int> (message.size()))){
                string error = "[Server]: Insufficient arguments (/mesg NAME MESSAGE) \n";

                //send invalid name error msg
                lock_guard<mutex> lock(clients_mutex);//dont forget to lock
                send(client_socket,error.c_str(), error.size(), 0);

                continue;
            }

            //extract and trim name
            string name = message.substr(6,(position-7));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            name.erase(0, name.find_first_not_of(" \t\r\n"));


            //lookup the name to find the receiver's file descriptor 
            /*
            if found, unicast, else send error message and continue
            */
            if(who_is.find(name)!= who_is.end()){
                unicast("\033[1;33m" + clients[client_socket] + "\033[0m: " + message.substr(position), who_is[name]);
            }
            else{
                string error = "[Server]: Invalid name, try /list command to see users\n";
                //send invalid name error msg
                lock_guard<mutex> lock(clients_mutex);//dont forget to lock
                send(client_socket,error.c_str(), error.size(), 0);
            }
        }
        else if(command == "/list")
        {
            string list="";

            //append names of users into the list string
            lock_guard<mutex> lock(clients_mutex);//dont forget to lock
            for(auto client:clients){
                if(client.first==client_socket) list.append(">");
                list.append(client.second+"\n");
            }
            list.append("\n");

            send(client_socket,list.c_str(), list.size(), 0); //send the list
        }
        else if (command == "/help") 
        {
            string help = 
                "\033[1;36mAvailable commands:\033[0m\n"
                "\033[1;33m/list         \033[0m - \033[0;37mList all connected users\033[0m\n"
                "\033[1;32m/mesg NAME MSG\033[0m - \033[0;37mSend a private message to NAME\033[0m\n"
                "\033[1;31m/exit         \033[0m - \033[0;37mLeave the chatroom\033[0m\n"
                "\033[1;34m/help         \033[0m - \033[0;37mShow this help message\033[0m\n\n"
            ;

            send(client_socket, help.c_str(), help.size(), 0);
        }
        else if (command == "/exit") 
        {
            string exitmsg = 
                "\033[1;32m\nThank you for using SocketChat!\033[0m\n"
                "\033[1;36mWe hope to see you again soon. Goodbye!\033[0m\n";
            send(client_socket, exitmsg.c_str(), exitmsg.size(), 0);
            break;
        }else{
            string tagged;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                tagged = "\033[1;35m[" + clients[client_socket] + "]\033[0m: " + message;
            }
            /*
            The above block is necessary because if the lock was outside it
            the broadcast function will not be able to execute since it also needs the lock
            so by encapsulating it within a block, we can release it once clients is accessed
            */
            broadcast(tagged, client_socket);

        }

    }

    close(client_socket);
    string name;
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

//this function sets up client and creates a thread to handle them afterwards
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



