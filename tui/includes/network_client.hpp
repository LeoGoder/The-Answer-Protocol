#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <ftxui/component/screen_interactive.hpp>

#include "../includes/app_state.hpp"
#include "../includes/models.hpp"

class NetworkClient {
 public:
    NetworkClient();
    ~NetworkClient();

    // Socket lifecycle
    bool connectToServer(const ConnectionConfig& config,
                         std::string& error_message);
    void disconnect();
    bool isConnected() const;

    // Send line to server
    bool sendLine(const std::string& line);

    // Background listener thread
    void startListener(ftxui::ScreenInteractive& screen, AppState& state);
    void stopListener();

 private:
    void listenerLoop(ftxui::ScreenInteractive& screen, AppState& state);

    int socket_fd_;
    std::mutex send_mutex_;
    std::unique_ptr<std::thread> listener_thread_;
};
