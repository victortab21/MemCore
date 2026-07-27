#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


const int NUM_THREADS = 50;       // Número de clientes concurrentes
const int REQ_PER_THREAD = 2000;  // Operaciones por cada cliente

std::atomic<int> exitos(0);
std::atomic<int> fallos(0);

void simular_cliente(int thread_id) {
    
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) { 
        fallos += REQ_PER_THREAD; 
        return; 
    }

    sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(6379);
    inet_pton(AF_INET6, "::1", &server_addr.sin6_addr);


    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fallos += REQ_PER_THREAD;
        close(sock);
        return;
    }

  
    char buffer[1024];
    for (int i = 0; i < REQ_PER_THREAD; i++) {
        std::string cmd = "SET key_th" + std::to_string(thread_id) + "_" + std::to_string(i) + " payload_super_pesado\r\n";
        

        if (send(sock, cmd.c_str(), cmd.length(), 0) < 0) {
            fallos++;
            continue;
        }
        
     
        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, sizeof(buffer), 0) > 0) {
            exitos++;
        } else {
            fallos++;
        }
    }
    
    close(sock);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    MemCore - TCP Stress & Load Tester  " << std::endl;
    std::cout << "========================================" << std::endl;
    
    int total_esperado = NUM_THREADS * REQ_PER_THREAD;
    std::cout << "[INFO] Lanzando " << NUM_THREADS << " hilos concurrentes." << std::endl;
    std::cout << "[INFO] Operaciones totales a inyectar: " << total_esperado << "..." << std::endl;


    auto inicio = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> hilos;
    hilos.reserve(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        hilos.push_back(std::thread(simular_cliente, i));
    }


    for (auto& h : hilos) {
        h.join();
    }

    auto fin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracion = fin - inicio;

    int total_ejecutadas = exitos + fallos;
    double rps = total_ejecutadas / duracion.count(); // Requests Per Second

    // Reporte Final
    std::cout << "\n--- RESULTADOS DEL STRESS TEST ---" << std::endl;
    std::cout << "Tiempo total empleado:   " << duracion.count() << " segundos" << std::endl;
    std::cout << "Operaciones exitosas:    " << exitos << std::endl;
    std::cout << "Operaciones fallidas:    " << fallos << std::endl;
    std::cout << "Rendimiento (Throughput): " << rps << " RPS (Requests/sec)" << std::endl;

    return 0;
}