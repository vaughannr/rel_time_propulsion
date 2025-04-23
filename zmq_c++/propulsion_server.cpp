#include <iostream>
#include <string>
#include <atomic>
#include <sstream>
#include <thread>
#include <vector>
#include <zmq_addon.hpp>

#include "propulsion_server.hpp"


static std::atomic<bool> exitflag(false);

// Ctrl-C Signal handler to exit
void exitSignalHandler (int signal) {
    std::cerr << "Signal: " << signal << std::endl;
    exitflag = true;
}

// Thread function for checking the time vs. pending command time
// to see if it meets the fire condition
void PropulsionServer::checkFire() {
    while (!exitflag){
        if (pendingCmd_.has_value()) {
            auto now = std::chrono::steady_clock::now();
            if (pendingCmd_.value() <= now) {
                // Send firing message through socket. This is a multipart message
                // with the client ID and the message
                std::vector<zmq::const_buffer> message_frames = {
                    zmq::buffer(client_id_),
                    zmq::buffer("firing now!")
                };
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    zmq::send_result_t result = zmq::send_multipart(socket_, message_frames, zmq::send_flags::none);
                    if (!result) {
                        std::cerr << "Failed to send firing message" << std::endl;
                    }
                }
                pendingCmd_ = std::nullopt;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Thread function for listening for commands. If one is received,
// process it with sendCommand
void PropulsionServer::listenForCommands() {
    std::vector<zmq::message_t> message_frames;

    // thread: receive bytes from client as commands
    while (!exitflag) {

        // Receive a multipart message: identity of sender and the command
        zmq::recv_result_t result = zmq::recv_multipart(socket_, std::back_inserter(message_frames), zmq::recv_flags::none);
        if (!result) {
            std::cerr << "Failed to receive command" << std::endl;
            continue;
        }
        client_id_ = message_frames[0].to_string();

        // convert string message to int
        int relativeTime;
        std::istringstream iss(message_frames[1].to_string());
        iss >> relativeTime;
        sendCommand(relativeTime);

        // Clear the message frames for the next command
        message_frames.clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Process a command received from the client by updating the pending command variable
void PropulsionServer::sendCommand(int relativeTime) {
    if (relativeTime == -1) {
        std::cerr << "Received stop command" << std::endl;
        pendingCmd_ = std::nullopt;
        return;
    } else if (relativeTime < 0) {
        zmq::message_t msg(std::string("Invalid command: ") + std::to_string(relativeTime));
        {
            std::lock_guard<std::mutex> lock(mtx_);
            socket_.send(msg, zmq::send_flags::none);
        }
        return;
    } else {
        auto absTime = std::chrono::steady_clock::now() + std::chrono::seconds(relativeTime);
        pendingCmd_ = absTime;
    }
}