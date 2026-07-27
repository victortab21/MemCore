#include "Database.h"

void Database::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    storage[key] = value;
}

std::string Database::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = storage.find(key);
    
    if (it != storage.end()) {
        return it->second; 
    }
    
    return "NULL"; 
}

bool Database::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    return storage.erase(key) > 0;
}