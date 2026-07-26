cmake_minimum_required(VERSION 3.10)

project(MemCore VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- INICIO DE LAS OPTIMIZACIONES ---
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")
# --- FIN DE LAS OPTIMIZACIONES ---

include_directories(include)

# 1. Tu servidor principal (Une el main, la red y la base de datos)
add_executable(MemCore src/main.cpp src/TcpServer.cpp src/Database.cpp)

# 2. Tu programa de pruebas de rendimiento (Benchmark)
add_executable(MemCore_Benchmark src/benchmark.cpp src/Database.cpp)