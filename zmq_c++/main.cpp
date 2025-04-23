#include <future>
#include <csignal>
#include "propulsion_server.hpp"

extern void exitSignalHandler (int signal);

//TCP server
int main() {

    // Register signal handler
    std::signal(SIGINT, exitSignalHandler);

    // Server to hide boilerplate implementation
    PropulsionServer propulsionServer;

    // Spool thread for checking fire condition
    auto fire_thread = std::async(std::launch::async, &PropulsionServer::checkFire, &propulsionServer);
    // Spool thread for listening for commands
    auto cmd_thread = std::async(std::launch::async, &PropulsionServer::listenForCommands, &propulsionServer);

    fire_thread.wait();
    cmd_thread.wait();

    return 0;
}