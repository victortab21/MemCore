#include "TcpServer.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sstream>

using namespace std;

const int MAX_EVENTS = 64;
const int BUFFER_SIZE = 1024;

// --- CONSTRUCTOR: Configuración del socket y epoll ---
TcpServer::TcpServer(int port, Database& db) : port(port), db(db) {
    server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    int ipv6_only = 1;
    setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

    setNonBlocking(server_fd);

    sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any; 
    server_addr.sin6_port = htons(port);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, SOMAXCONN);

    epoll_fd = epoll_create1(0);
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    
    events.resize(MAX_EVENTS);
}

// --- DESTRUCTOR: Limpieza ---
TcpServer::~TcpServer() {
    close(server_fd);
    close(epoll_fd);
}

void TcpServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int TcpServer::run() {
    while (true) {
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        
        for (int i = 0; i < num_events; ++i) {
            int current_fd = events[i].data.fd;

            if (current_fd == server_fd) {
                handleNewConnection();
            } else {
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                    close(current_fd);
                    continue;
                }
                handleClientData(current_fd);
            }
        }
    }
    return 0;
}

//aceptar nuevas conexiones
void TcpServer::handleNewConnection() {
    while (true) {
        sockaddr_in6 client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd == -1) break; // Se han acabado las conexiones pendientes

        setNonBlocking(client_fd);
        epoll_event client_ev;
        client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        client_ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
    }
}

// --- PROCESAR DATOS DE CLIENTES ---
void TcpServer::handleClientData(int client_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_recv = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_recv == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
            close(client_fd);
            break;
        } else if (bytes_recv == 0) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
            close(client_fd);
            break;
        }

        string raw_data(buffer, bytes_recv);
        string response = processCommand(raw_data);
        send(client_fd, response.c_str(), response.length(), 0);
    }
}

// --- PARSER DE COMANDOS ---
string TcpServer::processCommand(const string& raw_data) {
    stringstream ss(raw_data);
    string comando, key, value, respuesta;
    
    ss >> comando;

    if (comando == "SET") {
        ss >> key >> value;
        if (!key.empty() && !value.empty()) {
            db.set(key, value);
            respuesta = "+OK\r\n";
        } else {
            respuesta = "-ERR Usage: SET <key> <value>\r\n";
        }
    } 
    else if (comando == "GET") {
        ss >> key;
        if (!key.empty()) {
            string res = db.get(key);
            respuesta = (res != "NULL") ? "$" + res + "\r\n" : "-ERR Key not found\r\n";
        } else {
            respuesta = "-ERR Usage: GET <key>\r\n";
        }
    } 
    else if (comando == "DEL" || comando == "REMOVE") {
        ss >> key;
        if (!key.empty()) {
            bool borrado = db.remove(key);
            respuesta = borrado ? "+OK\r\n" : "-ERR Key not found\r\n";
        } else {
            respuesta = "-ERR Usage: DEL <key>\r\n";
        }
    } 
    else {
        respuesta = "-ERR Unknown command\r\n";
    }

    return respuesta;
}