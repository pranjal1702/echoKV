#include "../include/linear_probing_hash_table.hpp"
#include<stdexcept>
#include<mutex>
#include <xxhash.h> 
#include<iostream>
template<typename KeyType,typename ValueType>
LinearProbingHashTable<KeyType,ValueType>::LinearProbingHashTable(size_t initial_capacity,float load_factor):load_factor_(load_factor) {
    if(initial_capacity<=0||load_factor<=0.0f||load_factor>=1.0f){
        throw std::invalid_argument("Invalid capacity or load factor");
    }
    table_.resize(initial_capacity);
}


template<typename KeyType,typename ValueType>
bool LinearProbingHashTable<KeyType,ValueType>::put(const KeyType& key,const ValueType& value){
    if(key.empty()) return false;
    std::unique_lock lock(mutex_); // Exclusive write lock
    if (static_cast<float>(size_ + 1) / table_.size() > load_factor_) {
        resize();
    }

    size_t index=find_slot(key);  
    // std::cout<<index<<" "<<table_.size()<<'\n';  
    if(index==table_.size()) {
        // try resizing as slots not available
        resize();
        index=find_slot(key);
    }

    if(index==table_.size()){
        // if still can not put return false, but this will not be the case (most probably)
        return false;
    }

    if(!table_[index]){
        // nothing was stored at this index
        table_[index]=std::make_unique<HashEntry>();  // pointer that stores HashEntry
        table_[index]->key=key;
        table_[index]->value=value;
        table_[index]->is_deleted=false;
        ++size_;
        return true;
    }else if(table_[index]->key==key&&!table_[index]->is_deleted){
        //  only value is to be updated
        table_[index]->value=value;  // updating existing key
        return true;
    }else if(table_[index]->is_deleted){
        // the value has been deleted logically but some other value present
        table_[index]->key=key;
        table_[index]->value=value;
        table_[index]->is_deleted=false;
        size_++;
        return true;
    }
    return false; // find_slot and other logic is implemented in a way that program should not reach here
}

template<typename KeyType,typename ValueType>
std::optional<ValueType> LinearProbingHashTable<KeyType,ValueType>::get(const KeyType& key) const{
    // multiple readers at a time allowed
    std::shared_lock lock(mutex_);
    size_t index=find_slot(key);
    if(table_[index]&&table_[index]->key==key&&!table_[index]->is_deleted){
        return table_[index]->value;
    }else{
        return std::nullopt;
    }
}

template<typename KeyType,typename ValueType>
bool LinearProbingHashTable<KeyType,ValueType>::remove(const KeyType& key) {
    if(key.empty()) return false;
    std::unique_lock lock(mutex_); // only 1 writer allowed
    size_t index=find_slot(key);
    if (table_[index] && table_[index]->key == key && !table_[index]->is_deleted) {
        table_[index]->is_deleted = true;
        --size_;
        return true;
    }
    return false;
} 

template<typename KeyType, typename ValueType>
size_t LinearProbingHashTable<KeyType, ValueType>::size() const {
    std::shared_lock lock(mutex_);  // readers are allowed, writers are not allowed so that size_ do not get changed
    return size_;
}

template<typename KeyType, typename ValueType>
void LinearProbingHashTable<KeyType, ValueType>::resize() {
    // Remove the unique_lock here, assume caller (put/remove) already holds it
    std::vector<std::unique_ptr<HashEntry>> old_table = std::move(table_);
    table_.resize(old_table.size() * 2);
    size_ = 0;

    for (const auto& entry : old_table) {
        if (entry && !entry->is_deleted) {
            size_t index = find_slot(entry->key);
            if (!table_[index]) {
                table_[index] = std::make_unique<HashEntry>();
                table_[index]->key = entry->key;
                table_[index]->value = entry->value;
                table_[index]->is_deleted = false;
                ++size_;
            } else if (table_[index]->is_deleted) {
                table_[index]->key = entry->key;
                table_[index]->value = entry->value;
                table_[index]->is_deleted = false;
                ++size_;
            }
        }
    }
}


template<typename KeyType, typename ValueType>
size_t LinearProbingHashTable<KeyType, ValueType>::find_slot(const KeyType& key) const {    
    if(key.empty()) return false;
    // size_t hash = XXH64(key.c_str(), key.size(), 0);
    size_t hash = std::hash<KeyType>{}(key);
    size_t index = hash % table_.size();
    // now the step of linear probing
    size_t start_index=index;
    do {
        if (!table_[index] || (table_[index]->key == key && !table_[index]->is_deleted) || table_[index]->is_deleted) {
            return index;
        }
        index = (index + 1) % table_.size();
    } while (index != start_index);

    return table_.size(); // Table is full
}

template class LinearProbingHashTable<std::string, VariantValueType>;