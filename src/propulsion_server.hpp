#include <chrono>
#include <mutex>
#include <optional>
#include <tuple>


class PropulsionServer {
public:
    PropulsionServer() :
        server_fd_(-1),
        client_fd_(-1),
        pendingCmd_(std::nullopt),
        mtx_() {}
    ~PropulsionServer() {
        close(client_fd_);
        close(server_fd_);
        std::cout << "Shutting down server" << std::endl;
    }

    void checkFire();
    int setupTCPSocket();
    void listenForCommands();
    void sendCommand(int relativeTime);

    private:
        const int BUF_SIZE = 1024;
        int server_fd_, client_fd_;
        std::optional<std::chrono::steady_clock::time_point> pendingCmd_;
        std::mutex mtx_;
};