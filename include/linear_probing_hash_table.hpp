#pragma once
#include<memory>
#include<optional>
#include<shared_mutex>
#include<string>
#include<vector>
#include<concepts>

template<typename T>
concept Hashable = requires(T a){
    {std::hash<T>{}(a)} -> std::convertible_to<std::size_t>;
};

struct alignas(64) HashEntry{
    std::string key;
    std:: string value;
    bool is_deleted{false};
    HashEntry() = default;
};

template<typename KeyType,typename ValueType>
requires Hashable<KeyType>
class LinearProbingHashTable{
public:
    explicit LinearProbingHashTable(size_t initial_capacity = 16, float load_factor = 0.75);
    ~LinearProbingHashTable() = default;

    // Non-copyable to prevent accidental copies
    LinearProbingHashTable(const LinearProbingHashTable&) = delete;
    LinearProbingHashTable& operator=(const LinearProbingHashTable&) = delete;

    bool put(const KeyType& key,const ValueType& value);
    std::optional<ValueType> get(const KeyType& key) const;

    bool remove(const KeyType& key);

    size_t size() const;
private:
    void resize();

    size_t find_slot(const KeyType& key) const;

    std::vector<std::unique_ptr<HashEntry>> table_;
    size_t size_{0};   // number of key value pairs
    const float load_factor_;
    mutable std::shared_mutex mutex_;
};