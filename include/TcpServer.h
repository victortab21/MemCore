#pragma once
#include "Database.h"
#include <string>
#include <vector>
#include <sys/epoll.h>

class TcpServer {
private:
    int port;
    int server_fd;
    int epoll_fd;
    Database& db; // Referencia a la BBDD
    std::vector<epoll_event> events;

    void setNonBlocking(int fd);
    void handleNewConnection();
    void handleClientData(int client_fd);
   std::string processCommand(std::string_view raw_data);

public:
    TcpServer(int port, Database& db);
    ~TcpServer();
    int run();
};