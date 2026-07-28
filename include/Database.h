#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>      // Necesario para std::unique_lock
#include <vector>
#include <functional>
#include <cstddef>    // Necesario para size_t

class Database {
private:
    struct Shard {
        std::unordered_map<std::string, std::string> storage;
        mutable std::shared_mutex mtx; 
    };

    static constexpr size_t NUM_SHARDS = 16; 
    std::vector<Shard> shards;

    size_t getShardIndex(const std::string& key) const {
        return std::hash<std::string>{}(key) % NUM_SHARDS;
    }

public:
    Database() : shards(NUM_SHARDS) {}

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool remove(const std::string& key);
};

#endif