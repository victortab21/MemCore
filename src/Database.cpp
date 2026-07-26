#include "Database.h"

// SET
void Database::set(const std::string& key, const std::string& value) {
  std :: lock_guard<std::mutex> cerrojo_1(mtx);
    storage[key] = value;

}

//GET
std::string Database::get(const std::string& key) {
  std :: lock_guard<std::mutex> cerrojo_2(mtx);
    auto it = storage.find(key);
    
    if (it != storage.end()) {
        return it->second; 
    }
    
    return "NULL"; 
}

//REMOVE
bool Database::remove(const std::string& key) {
      std :: lock_guard<std::mutex> cerrojo_3(mtx);
    return storage.erase(key) > 0;
}