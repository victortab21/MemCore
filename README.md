# 🚀 MemCore

**Ultra-Low Latency In-Memory Key-Value Store**

MemCore is a high-performance, concurrent, in-memory key-value database written in modern C++17. It is designed to act as a highly efficient caching layer or volatile storage engine, prioritizing sub-millisecond response times and resource efficiency.

## 🧠 Core Design Advantages

* **Asynchronous Network Engine (`epoll`):** Built strictly on Linux sockets using the `epoll` API in **Edge-Triggered (`EPOLLET`)** mode. This allows the server to handle thousands of concurrent connections with zero busy-waiting and minimal CPU overhead.
* **Non-Blocking I/O:** All sockets (server and clients) are configured via `fcntl` as non-blocking, ensuring the event loop never stalls on read/write operations.
* **Thread-Safe Architecture:** Memory operations are safeguarded against race conditions under high concurrency using granular `std::lock_guard<std::mutex>` synchronization.
* **Custom Protocol & Parser:** Implements a lightweight TCP text protocol supporting core operations (`SET`, `GET`, `DEL`).
* **Latency Profiling:** Includes a custom benchmarking suite measuring raw in-memory operation latency, calculating distribution across p10, p50, p90, and p99 percentiles.

---

## 🛠️ Build & Installation / Compilación y Uso

### Prerequisites
* GCC/Clang with C++17 support
* CMake 3.10+
* Linux environment (or WSL)

### 1. Compilation
```bash
# Clone the repository / Clonar el repositorio
git clone [https://github.com/victortab21/MemCore.git](https://github.com/victortab21/MemCore.git)
cd MemCore

# Create build directory and compile / Crear directorio de build y compilar
mkdir build && cd build
cmake ..
make

*(Spanish Version)*

# 🚀 MemCore

**Base de Datos Clave-Valor en Memoria de Ultra-Baja Latencia**

MemCore es una base de datos clave-valor en memoria, concurrente y de alto rendimiento escrita en C++17 moderno. Está diseñada para actuar como una capa de caché altamente eficiente o un motor de almacenamiento volátil, priorizando tiempos de respuesta por debajo del milisegundo y la eficiencia de recursos.

## 🧠 Ventajas de Diseño Core

* **Motor de Red Asíncrono (`epoll`):** Construido estrictamente sobre sockets de Linux usando la API `epoll` en modo **Edge-Triggered (`EPOLLET`)**. Esto permite al servidor manejar miles de conexiones concurrentes sin *busy-waiting* y con un consumo de CPU mínimo.
* **I/O No Bloqueante:** Todos los sockets (servidor y clientes) están configurados mediante `fcntl` como no bloqueantes, garantizando que el bucle de eventos nunca se detenga en operaciones de lectura/escritura.
* **Arquitectura Thread-Safe:** Las operaciones de memoria están protegidas contra condiciones de carrera bajo alta concurrencia usando sincronización granular con `std::lock_guard<std::mutex>`.
* **Protocolo y Parser Propios:** Implementa un protocolo TCP de texto ligero que soporta las operaciones principales (`SET`, `GET`, `DEL`).
* **Perfilado de Latencia:** Incluye una suite de *benchmarking* personalizada que mide la latencia bruta de las operaciones en memoria, calculando la distribución en los percentiles p10, p50, p90 y p99.