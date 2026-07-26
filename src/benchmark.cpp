#include "Database.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std;
// Usamos un alias para no escribir todo el tocho de chrono cada vez
using namespace std::chrono; 

int main() {
    cout << "========================================" << endl;
    cout << "      MemCore - Latency Benchmark       " << endl;
    cout << "========================================" << endl;

    Database db;
    const int NUM_OPS = 1000000; // 1 millón de operaciones

    // 1. PREPARACIÓN (Setup)
    // TODO: Declara un std::vector<long long> para guardar las latencias
    // TODO: Usa el método .reserve() del vector para reservar espacio para NUM_OPS

    std :: vector <long long> latencias_set;
    latencias_set.reserve(NUM_OPS);


    
    cout << "[INFO] Calentando la caché de la CPU..." << endl;
    // 2. CALENTAMIENTO (Warm-up)
    for (int i = 0; i < 10000; i++) {
        db.set("warmup_key", "warmup_value");
    }

    cout << "[INFO] Ejecutando " << NUM_OPS << " operaciones SET..." << endl;
    
    // 3. LA CARRERA (El bucle de medición)
    for (int i = 0; i < NUM_OPS; i++) {
        string key = "key_" + to_string(i);
        
        auto inicio= std :: chrono:: high_resolution_clock:: now();
        
        db.set(key, "data_payload"); // La operación a medir
        
        auto end= std :: chrono:: high_resolution_clock:: now();
       

        auto diferencia= end-inicio;
               // TODO: Mete el resultado en el vector usando .push_back()
               long long nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(diferencia).count();
               
               latencias_set.push_back(nanos);
    }

    // 4. ANÁLISIS (Number Crunching)
    cout << "[INFO] Calculando percentiles..." << endl;
    // TODO: Usa std::sort() para ordenar el vector de menor a mayor

    std :: sort(latencias_set.begin(), latencias_set.end());


  

    
    // TODO: Extrae los datos (ej: el p90 es el elemento en la posición NUM_OPS * 0.90)
     long long p50 = latencias_set[NUM_OPS*0.50];
     long long p90 = latencias_set[NUM_OPS*0.90];
     long long p99 = latencias_set[NUM_OPS*0.99];
     long long p10 = latencias_set[NUM_OPS*0.10];



     //BENCHMARK DE LECTURAS (GET)

       cout << "\n[INFO] Preparando 1.000.000 de operaciones GET..." << endl;
       std :: vector <long long> latencias_get;
       latencias_get.reserve(NUM_OPS);

       cout << "[INFO] Ejecutando " << NUM_OPS << " operaciones GET..." << endl;

    // 2. El bucle de lectura
    for (int i = 0; i < NUM_OPS; i++) {
        // Leemos las mismas claves que acabamos de insertar en la Fase 1
        string key = "key_" + to_string(i);


          auto inicio= std :: chrono:: high_resolution_clock:: now();


        
        // Operación a medir:
        db.get(key); 
        
        // TODO: Toma el tiempo de fin T2
        // TODO: Calcula la diferencia en nanosegundos y métela en 'latencias_get'
                
        auto end= std :: chrono:: high_resolution_clock:: now();
       

        auto diferencia= end-inicio;

         long long nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(diferencia).count();

        latencias_get.push_back(nanos);
    }

    std :: sort(latencias_get.begin(), latencias_get.end());

    long long p50_get = latencias_get[NUM_OPS*0.50];
     long long p90_get = latencias_get[NUM_OPS*0.90];
     long long p99_get = latencias_get[NUM_OPS*0.99];
     long long p10_get = latencias_get[NUM_OPS*0.10];





       





    // 5. REPORTE
    cout << "\n--- RESULTADOS DE LATENCIA SET (Nanosegundos) ---" << endl;
    cout << "\n--- Percentil 10: " << p10 << "ns" <<endl;
    cout << "\n--- Percentil 50: " << p50 << "ns" <<endl;
    cout << "\n--- Percentil 90: " << p90 << "ns" <<endl;
    cout << "\n--- Percentil 99: " << p99 << "ns" <<endl;

       cout << "\n--- RESULTADOS DE LATENCIA GET (Nanosegundos) ---" << endl;
    cout << "\n--- Percentil 10: " << p10_get << "ns" <<endl;
    cout << "\n--- Percentil 50: " << p50_get << "ns" <<endl;
    cout << "\n--- Percentil 90: " << p90_get << "ns" <<endl;
    cout << "\n--- Percentil 99: " << p99_get << "ns" <<endl;


    
    
    return 0;
}