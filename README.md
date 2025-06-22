# TCP Chat Server in C++

## Features
- Multi-client TCP chat using POSIX sockets
- Clients can send and receive messages concurrently
- Built with C++ and standard Linux socket APIs

## Build Instructions
```bash
make
```

## Run Instructions
Start the server:
```bash
./chat_server
```

Start multiple clients in separate terminals:
```bash
./chat_client
```

To exit a client, type:
```text
exit
```

## Notes
- Server listens on port 8080
- All messages are broadcast to all clients except sender
# tcp-chat
