# SocketChat: A TCP-Based Multi-Client Chat Server in C++

SocketChat is a multi-client chat application implemented in C++ using POSIX sockets. It allows multiple users to join a shared chatroom, broadcast messages to all users, or send private (unicast) messages to specific users.

Built with thread support and a command-driven interface, SocketChat is designed to demonstrate real-world network programming concepts such as socket communication, concurrent handling using threads, and basic server-client architecture.

---

## 🌟 Features

- ✅ **Multi-client support** — Multiple users can join and chat simultaneously.
- ✅ **Broadcast messaging** — Messages are sent to all connected users by default.
- ✅ **Private messaging (unicast)** — Send a message to a specific user using `/mesg` command.
- ✅ **List connected users** — Use `/list` to see who is online, with your name highlighted.
- ✅ **Help command** — `/help` shows all available commands.
- ✅ **Clean exit** — `/exit` gracefully leaves the chatroom and informs others.
- ✅ **Colored terminal output** — Usernames, system messages, and commands are color-coded for better readability.
- ✅ **Thread-safe implementation** — Uses mutexes to safely manage shared data.

---

## 💻 Build Instructions

Make sure you have `g++` and `make` installed.

```bash
make
```

---

## ▶️ Run Instructions

### Start the Server

```bash
./chat_server
```

By default, the server listens on port **8080**.

### Start the Clients

In separate terminals (or using VSCode split terminals):

```bash
./chat_client
```

---

## 🗨️ Client Usage

When you run the client, you’ll be prompted to enter your name. After that, you can start chatting!

### Available Commands

| Command          | Description                                               |
| ---------------- | --------------------------------------------------------- |
| `/list`          | List all connected users. Your name is prefixed with `>`. |
| `/mesg NAME MSG` | Send a private (unicast) message to `NAME`.               |
| `/help`          | Show available commands.                                  |
| `/exit`          | Leave the chatroom gracefully.                            |

---

## 💬 Example Flow

```plaintext
----------WELCOME TO SOCKET_CHAT----------
This is a TCP-based socket chat app. Enter your name and start chatting :)
Use the command /help for commands

Enter your name
Aswin
NAME:Aswin

> hi
[lolcat]: hi ppl

> /list
anon
lolcat
>Aswin

> /mesg anon hey, this is a secret
anon: ok, got it. lolcat doesn’t need to know 

> /exit

Thank you for using SocketChat!
We hope to see you again soon. Goodbye!
```

---

## 🗂️ Project Structure

```
.
├── chat_server        # Compiled server executable
├── chat_client        # Compiled client executable
├── src/
│   ├── client.cpp     # Client source code
│   ├── server.cpp     # Server source code
│   └── main.cpp       # Server entry point
├── include/
│   └── server.h       # Server header
├── .vscode/
│   └── settings.json  # VSCode configuration
├── Makefile           # Build configuration
├── README.md          # Project documentation

```

---

## ⚙️ Technical Details

- **Language**: C++ (C++11 or higher)
- **Networking**: POSIX sockets
- **Threads**: std::thread (client handler threads)
- **Synchronization**: std::mutex for safe client list operations
- **Port**: 8080 (modifiable)

---

## 💡 Known Limitations

- No authentication or encryption (plaintext TCP).
- Private messages are only supported via username and require users to know each other's names exactly.
- Basic error handling; production-level robustness would require more checks.

---

## 🤝 Contributing

Contributions are welcome! You can:

- Submit a pull request with improvements (new features, code cleanups, etc.)
- Open an issue for bugs or suggestions

---


## 🛡️ Disclaimer

SocketChat is intended for learning and demonstration purposes. It does not implement secure communication and should not be used as-is in production environments.

---

## ✉️ Contact

If you have questions or want to share feedback, feel free to open an issue or reach out via GitHub.

---

