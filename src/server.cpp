#include "Database.h"
#include <iostream>
#include <string>
#include <cstring>      // Para memset
#include <unistd.h>     // Para close()
#include <sys/socket.h> // La API principal de sockets
#include <netinet/in.h> // Para la estructura sockaddr_in
#include <sys/epoll.h>
#include <sstream>
#include <fcntl.h>  // Para fcntl(), F_GETFL, F_SETFL, y O_NONBLOCK
#include <vector>   // Para std::vector

using namespace std;

const int MAX_EVENTS = 64;
const int BUFFER_SIZE = 1024;


// Función crítica para configurar sockets en modo NO BLOQUEANTE
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        cerr << "[ERROR] fcntl F_GETFL falló" << endl;
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "[ERROR] fcntl F_SETFL falló" << endl;
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "      MemCore - TCP Server (Port 6379)  " << endl;
    cout << "========================================" << endl;

    Database db;
    int port = 6379;

    // 1. CREAR EL SOCKET (El "teléfono")
    int server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "[ERROR] No se pudo crear el socket." << endl;
        return 1;
    }

    //FORZAMOS QUE NO SE QUEDE ESPERANDO PAQUETES DESPUÉS DE APAGAR EL SERVIDOR
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Forzar a que solo acepte IPv6 (desactivando el mapeo de IPv4 si se prefiere un canal puro)
    int ipv6_only = 1;
    setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

    // Hacer que el socket de escucha sea NO BLOQUEANTE
    set_nonblocking(server_fd);

    // 2. CONFIGURAR DIRECCIÓN IPv6
    sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any; // Escucha en "::"
    server_addr.sin6_port = htons(port);

    // 3. BIND Y LISTEN
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "[ERROR] Fallo en el bind IPv6." << endl;
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) { // SOMAXCONN es el máximo recomendado por el sistema
        cerr << "[ERROR] Fallo en el listen." << endl;
        close(server_fd);
        return 1;
    }

    cout << "[INFO] Servidor IPv6 No Bloqueante escuchando en el puerto " << port << "..." << endl;


    //Fase DOS 

   int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        cerr << "[ERROR] No se pudo crear la instancia epoll." << endl;
        close(server_fd);
        return 1;
    }

    // Registrar nuestro socket servidor en epoll para detectar nuevas conexiones (EPOLLIN)
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // Usamos Edge-Triggered (EPOLLET) para rendimiento óptimo
    ev.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        cerr << "[ERROR] Fallo al añadir el servidor a epoll." << endl;
        close(epoll_fd);
        close(server_fd);
        return 1;
    }

    vector<epoll_event> events(MAX_EVENTS);
    // 5. BUCLE INFINITO DE CONEXIONES
    while (true) {
        // epoll_wait bloquea el hilo eficientemente hasta que ocurra un evento registrado.
        // Consume 0% de CPU mientras no haya actividad.
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        if (num_events == -1) {
            cerr << "[ERROR] epoll_wait falló." << endl;
            break;
        }

        // Iterar solo sobre los descriptores que tienen eventos listos
        for (int i = 0; i < num_events; ++i) {
            int current_fd = events[i].data.fd;

            // CASO A: Evento en el socket del servidor -> Nueva conexión entrante
            if (current_fd == server_fd) {
                // Al estar en modo Edge-Triggered, debemos aceptar todas las conexiones pendientes
                // en un bucle hasta que accept() devuelva EAGAIN o EWOULDBLOCK.
                while (true) {
                    sockaddr_in6 client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Hemos vaciado la cola de conexiones pendientes por ahora
                            break;
                        } else {
                            cerr << "[ERROR] Error al aceptar cliente." << endl;
                            break;
                        }
                    }

                    cout << "[INFO] Nuevo cliente conectado (IPv6)." << endl;

                    // El socket del cliente también debe ser No Bloqueante
                    set_nonblocking(client_fd);

                    // Registrar el nuevo cliente en epoll para vigilar cuándo nos envía datos
                    // Usamos EPOLLET (Edge-Triggered) y EPOLLIN (Lectura)
                    epoll_event client_ev;
                    client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP; // EPOLLRDHUP detecta desconexión ordenada
                    client_ev.data.fd = client_fd;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
                        cerr << "[ERROR] No se pudo añadir cliente a epoll." << endl;
                        close(client_fd);
                    }
                }
            } 
// CASO B: Evento en un socket de cliente -> Datos listos para leer o desconexión
            else {
                // Si el cliente se desconectó
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    cout << "[INFO] Cliente desconectado." << endl;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                    close(current_fd);
                    continue;
                }

                // Leer datos del cliente (bucle cerrado por usar Edge-Triggered)
                char buffer[BUFFER_SIZE];
                while (true) {
                    memset(buffer, 0, BUFFER_SIZE);
                    ssize_t bytes_recv = recv(current_fd, buffer, BUFFER_SIZE - 1, 0);

                    if (bytes_recv == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Ya no quedan más datos por leer en el buffer de red del kernel
                            break;
                        }
                        cerr << "[ERROR] Fallo en la lectura del socket." << endl;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                        close(current_fd);
                        break;
                    } else if (bytes_recv == 0) {
                        // El cliente cerró la conexión
                        cout << "[INFO] Cliente cerró conexión." << endl;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                        close(current_fd);
                        break;
                    }

                    // --- INICIO DEL PARSER Y CONEXIÓN CON BBDD ---
                    cout << "[DATOS RECIBIDOS]: " << buffer;

                    string mensaje_cliente(buffer, bytes_recv);
                    stringstream ss(mensaje_cliente);
                    string comando, key, value;

                    ss >> comando;
                    string respuesta;

                    if (comando == "SET") {
                        ss >> key >> value;
                        if (!key.empty() && !value.empty()) {
                            db.set(key, value);
                            respuesta = "+OK\r\n";
                        } else {
                            respuesta = "-ERR Uso: SET <clave> <valor>\r\n";
                        }
                    } 
                    else if (comando == "GET") {
                        ss >> key;
                        if (!key.empty()) {
                            string res = db.get(key);
                            if (res != "NULL") {
                                respuesta = "$" + res + "\r\n";
                            } else {
                                respuesta = "-ERR Clave no encontrada\r\n";
                            }
                        } else {
                            respuesta = "-ERR Uso: GET <clave>\r\n";
                        }
                    } 
                    else if (comando == "DEL" || comando == "REMOVE") {
                        ss >> key;
                        if (!key.empty()) {
                            bool borrado = db.remove(key);
                            respuesta = borrado ? "+OK\r\n" : "-ERR Clave no encontrada\r\n";
                        } else {
                            respuesta = "-ERR Uso: DEL <clave>\r\n";
                        }
                    } 
                    else {
                        respuesta = "-ERR Comando desconocido\r\n";
                    }

                    // Responder al cliente con el resultado real de la base de datos
                    send(current_fd, respuesta.c_str(), respuesta.length(), 0);
                    // --- FIN DEL PARSER ---
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;

   
}