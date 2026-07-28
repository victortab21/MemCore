#include "Database.h"

void Database::set(const std::string& key, const std::string& value) {
    size_t idx = getShardIndex(key);
    std::unique_lock<std::shared_mutex> lock(shards[idx].mtx);
    shards[idx].storage[key] = value;
}

std::string Database::get(const std::string& key) {
    size_t idx = getShardIndex(key);
    std::shared_lock<std::shared_mutex> lock(shards[idx].mtx);
    
    auto it = shards[idx].storage.find(key);
    if (it != shards[idx].storage.end()) {
        return it->second;
    }
    return "NULL";
}

bool Database::remove(const std::string& key) {
    size_t idx = getShardIndex(key);
    std::unique_lock<std::shared_mutex> lock(shards[idx].mtx);
    return shards[idx].storage.erase(key) > 0;
}