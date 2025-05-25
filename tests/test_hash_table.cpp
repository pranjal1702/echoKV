#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "../include/linear_probing_hash_table.hpp"
#include <thread>
#include <vector>

TEST_CASE("LinearProbingHashTable basic operations", "[hash_table]") {
    LinearProbingHashTable<std::string, VariantValueType> ht(4, 0.75);

    SECTION("Put and Get") {
        REQUIRE(ht.put("key1", "value1"));
        REQUIRE(ht.get("key1") == std::optional<VariantValueType>{"value1"});
        REQUIRE_FALSE(ht.get("key2").has_value());
    }

    SECTION("Update existing key") {
        ht.put("key1", "value1");
        REQUIRE(ht.put("key1", "value2"));
        REQUIRE(ht.get("key1") == std::optional<VariantValueType>{"value2"});
    }

    SECTION("Remove key") {
        ht.put("key1", "value1");
        REQUIRE(ht.remove("key1"));
        REQUIRE_FALSE(ht.get("key1").has_value());
        REQUIRE_FALSE(ht.remove("key2"));
    }

    SECTION("Size tracking") {
        REQUIRE(ht.size() == 0);
        ht.put("key1", "value1");
        REQUIRE(ht.size() == 1);
        ht.put("key2", "value2");
        REQUIRE(ht.size() == 2);
        ht.remove("key1");
        REQUIRE(ht.size() == 1);
    }
}



TEST_CASE("LinearProbingHashTable collision handling", "[hash_table]") {
    LinearProbingHashTable<std::string, VariantValueType> ht(4, 0.75);
    ht.put("key1", "value1");
    ht.put("key2", "value2"); // Potential collision
    REQUIRE(ht.get("key1") == std::optional<VariantValueType>{"value1"});
    REQUIRE(ht.get("key2") == std::optional<VariantValueType>{"value2"});
}

TEST_CASE("LinearProbingHashTable concurrency", "[concurrency]") {
    LinearProbingHashTable<std::string, VariantValueType> ht(16, 0.75);
    std::vector<std::thread> threads;

    // Increase to 8 threads for robust concurrency testing
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&ht, i]() {
            for (int j = 0; j < 100; ++j) { // Insert multiple keys per thread
                std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
                std::string value = "value" + std::to_string(i) + "_" + std::to_string(j);
                ht.put(key, value);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    // Verify all inserted keys
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 100; ++j) {
            std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
            std::string expected = "value" + std::to_string(i) + "_" + std::to_string(j);
            REQUIRE(ht.get(key) == std::optional<VariantValueType>{expected});
        }
    }
    REQUIRE(ht.size() == 8 * 100); 
}