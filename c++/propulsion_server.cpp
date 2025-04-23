#include <iostream>
#include <string>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <thread>

#include "propulsion_server.hpp"

constexpr u_int16_t PORT = 8124;
constexpr int MAX_CLIENTS = 2;

static std::atomic<bool> exitflag(false);

// Ctrl-C Signal handler to exit
void exitSignalHandler (int signal) {
    std::cerr << "Signal: " << signal << std::endl;
    exitflag = true;
}

// Boilerplate TCP server setup function
 int PropulsionServer::setupTCPSocket() {
    // Socket variables
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create a socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd_);
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd_, MAX_CLIENTS) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_fd_);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Accept incoming connection
    client_fd_ = accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd_ < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        close(server_fd_);
        return -1;
    }

    std::cout << "Client connected" << std::endl;

    return 0;

}

// Thread function for checking the time vs. pending command time
// to see if it meets the fire condition
void PropulsionServer::checkFire() {
    while (!exitflag){
        std::unique_lock<std::mutex> lock(mtx_);
        if (pendingCmd_.has_value()) {
            auto now = std::chrono::steady_clock::now();
            if (pendingCmd_.value() <= now) {
                // Send firing message through socket
                std::string msg("firing now!");
                int bytes_sent = send(client_fd_, msg.c_str(), msg.size(), 0);
                if (bytes_sent < 0) {
                    std::cerr << "Error sending data" << std::endl;
                }
                pendingCmd_ = std::nullopt;
            }
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Thread function for listening for commands. If one is received,
// process it with sendCommand
void PropulsionServer::listenForCommands() {
    char buffer[BUF_SIZE];
    int bytes_received;

    // thread: receive bytes from client as commands
    while (!exitflag && (bytes_received = recv(client_fd_, static_cast<void*>(buffer), BUF_SIZE, 0)) > 0 ) {

        // Expecting format of int relative time
        std::string msg(buffer, bytes_received);
        int relativeTime;
        std::istringstream iss(msg);
        iss >> relativeTime;
        sendCommand(relativeTime);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    
    if(bytes_received < 0) {
        std::cerr << "Error receiving data" << std::endl;
    }
}

// Process a command received from the client by updating the pending command variable
void PropulsionServer::sendCommand(int relativeTime) {
    if (relativeTime == -1) {
        std::cerr << "Received stop command" << std::endl;
        pendingCmd_ = std::nullopt;
        return;
    } else if (relativeTime < 0) {
        std::string msg("Invalid command: " + std::to_string(relativeTime));
        send(client_fd_, msg.c_str(), msg.size(), 0);
        return;
    } else {
        auto absTime = std::chrono::steady_clock::now() + std::chrono::seconds(relativeTime);
        pendingCmd_ = absTime;
    }
}