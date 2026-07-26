#include "TcpServer.h"
#include "Database.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    //  Puerto por defecto o por consola
    int port = 6379; 
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::cout << "[INFO] Starting MemCore Engine..." << std::endl;
    std::cout << "[INFO] Listening on port: " << port << std::endl;

    // Instanciamos BBDD y Servidor
    Database db;
    TcpServer server(port, db);

    
    return server.run();
}