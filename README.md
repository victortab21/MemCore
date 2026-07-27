# MemCore

An in-memory, thread-safe key-value store written in C++17. Uses an asynchronous event loop with Linux epoll (Edge-Triggered) and non-blocking I/O to handle TCP connections over IPv6.

---

## Technical Highlights

- **Event-Driven Networking:** Uses `epoll` in Edge-Triggered (`EPOLLET`) mode instead of spawning a thread per connection. Connections are handled non-blocking via `fcntl`.
- **Thread Safety:** The core key-value map is synchronized using `std::lock_guard<std::mutex>` to handle concurrent reads/writes safely.
- **Minimalist Protocol:** Simple text-based TCP protocol (`SET <key> <val>`, `GET <key>`, `DEL <key>`).
- **Benchmarking Tool:** Includes a dedicated benchmark binary that measures raw in-memory operation latencies (P10, P50, P90, P99) across 1M operations.

---

## How to Build & Run

### 1. Build using CMake
Requires `g++` (C++17) and `cmake`.

```bash
mkdir -p build && cd build
cmake ..
make
