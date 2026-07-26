#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

class Database {
private:
   
    std::unordered_map<std::string, std::string> storage;
    std :: mutex mtx;

public:
    Database() = default;
    ~Database() = default;


    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool remove(const std::string& key);
};
