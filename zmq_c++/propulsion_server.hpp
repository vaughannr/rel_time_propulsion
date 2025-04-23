#include <chrono>
#include <string>
#include <mutex>
#include <optional>
#include <iostream>
#include <zmq.hpp>


class PropulsionServer {
public:
    PropulsionServer() :
        context_(1), // 1 i/o thread. Enough for this simple server
        socket_(context_, zmq::socket_type::router),
        pendingCmd_(std::nullopt),
        mtx_() {
            socket_.bind("tcp://*:" + std::to_string(PORT));
        }
    ~PropulsionServer() {
        std::cout << "Shutting down server" << std::endl;
    }

    void checkFire();
    int setupTCPSocket();
    void listenForCommands();
    void sendCommand(int relativeTime);

    private:
        zmq::context_t context_;
        zmq::socket_t socket_;
        std::optional<std::chrono::steady_clock::time_point> pendingCmd_;
        std::mutex mtx_;
        std::string client_id_;
        const u_int16_t PORT = 8124;
};