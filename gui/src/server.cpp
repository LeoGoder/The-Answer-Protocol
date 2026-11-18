#include "inc/server.hpp"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdio>
#include <string>

int send_message(std::string command, std::string param, int fd) {
    std::string msg = command + param + '\n';
    // MSG_NOSIGNAL: a closed server gives an error, not a SIGPIPE that kills
    // the GUI.
    ssize_t res = send(fd, msg.data(), msg.size(), MSG_NOSIGNAL);

    return res;
}

std::string receiv_msg(int fd) {
    char buffer[MAX_DATA];
    ssize_t msg_get = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (msg_get > 0) {
        buffer[msg_get] = '\0';
        printf("[SERVER] %s", buffer);
        return buffer;
    } else if (msg_get == 0) {
        printf("[INFO] Connection close\n");
        return "CLOSE";
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        printf("[INFO] Erreur with recv: %ld\n", msg_get);
    }
    return "";
}
