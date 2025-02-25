# Peer-to-Peer-Chat-Application-with-FAQ-Chatbot-and-GPT-2-Integration
Client-Server Chat Application with FAQ Chatbot

# 📡 Peer-to-Peer Chat Application with Chatbot & History Management  

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))  
[![Networking](https://img.shields.io/badge/Networking-TCP/IP-green.svg)](https://en.wikipedia.org/wiki/Transmission_Control_Protocol)  
[![Chatbot](https://img.shields.io/badge/Chatbot-GPT--2-orange.svg)](https://huggingface.co/gpt2)  

## 📌 Overview  
This project is a **peer-to-peer chat application** implemented in C, allowing multiple clients to connect via a central server. It supports:  
✔️ **Unique client identification using UUID**  
✔️ **Real-time messaging between clients**  
✔️ **Chatbot integration (FAQ & GPT-2-based chatbot)**  
✔️ **Chat history management (view, delete, and clear history)**  

---

## 📌 Features  

### 🚀 **Peer-to-Peer Chat System**  
- Clients **connect to a central server** and receive a **unique UUID**.  
- View **active users** and send **private messages**.  
- The server **manages client connections, message routing, and error handling**.  

### 🤖 **FAQ Chatbot (Predefined Responses)**  
- Implements a **FAQ chatbot** with predefined **questions & answers**.  
- Clients can **enable or disable** the chatbot feature anytime.  
- The server **handles chatbot queries** and responds with relevant messages.  

### 🗂 **Chat History Management**  
- **Stores all messages** sent and received on the server.  
- Clients can:  
  - **View chat history** → `/history <recipient_id>`  
  - **Delete specific chats** → `/history_delete <recipient_id>`  
  - **Clear all chat history** → `/delete_all`  
- Messages are **stored for both sender & recipient**.

## 📌 Compilation
- gcc server.c -o server -luuid
- gcc client.c -o client -luuid

 ## 📌  Running the Programs
- ./server    # Start the server  
- ./client    # Start a client instance  





## 🛠 Requirements & Setup  

Ensure **UUID** is supported on your system before compiling the program.  

### 🔹 Install Dependencies (Linux)  
```bash
sudo apt-get install uuid-dev
