#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <vector>
#include <future>
#include <memory>
#include "../../include/Inventory.hpp"
#include "../../include/Product.hpp"

using namespace quirkventory;
using namespace std::chrono;

// Test Fixture for Inventory Tests
class InventoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
        
        // Create test products
        laptop = std::make_unique<Product>("LAPTOP001", "Gaming Laptop", "Electronics", 1299.99, 15);
        mouse = std::make_unique<Product>("MOUSE001", "Wireless Mouse", "Electronics", 49.99, 100);
        low_stock_item = std::make_unique<Product>("LOWSTOCK001", "Low Stock Item", "Test", 19.99, 3);
        
        // Create perishable products
        auto future_expiry = system_clock::now() + hours(72);
        auto past_expiry = system_clock::now() - hours(24);
        
        fresh_milk = std::make_unique<PerishableProduct>("MILK001", "Fresh Milk", "Dairy", 4.99, 20, future_expiry);
        expired_milk = std::make_unique<PerishableProduct>("MILK002", "Expired Milk", "Dairy", 4.99, 5, past_expiry);
    }
    
    void TearDown() override {
        inventory.reset();
    }
    
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<Product> laptop;
    std::unique_ptr<Product> mouse;
    std::unique_ptr<Product> low_stock_item;
    std::unique_ptr<PerishableProduct> fresh_milk;
    std::unique_ptr<PerishableProduct> expired_milk;
};

// Basic Inventory Operations
TEST_F(InventoryTest, AddProductSucceeds) {
    bool result = inventory->addProduct(std::move(laptop));
    EXPECT_TRUE(result);
    
    const Product* retrieved = inventory->getProduct("LAPTOP001");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "Gaming Laptop");
}

TEST_F(InventoryTest, AddDuplicateProductFails) {
    // Add first product
    inventory->addProduct(std::move(laptop));
    
    // Try to add product with same ID
    auto duplicate = std::make_unique<Product>("LAPTOP001", "Another Laptop", "Electronics", 999.99, 10);
    bool result = inventory->addProduct(std::move(duplicate));
    EXPECT_FALSE(result);
}

TEST_F(InventoryTest, GetNonExistentProductReturnsNull) {
    const Product* product = inventory->getProduct("NONEXISTENT");
    EXPECT_EQ(product, nullptr);
}

TEST_F(InventoryTest, RemoveProductSucceeds) {
    inventory->addProduct(std::move(laptop));
    
    bool result = inventory->removeProduct("LAPTOP001");
    EXPECT_TRUE(result);
    
    const Product* retrieved = inventory->getProduct("LAPTOP001");
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(InventoryTest, RemoveNonExistentProductFails) {
    bool result = inventory->removeProduct("NONEXISTENT");
    EXPECT_FALSE(result);
}

TEST_F(InventoryTest, UpdateQuantitySucceeds) {
    inventory->addProduct(std::move(laptop));
    
    bool result = inventory->updateQuantity("LAPTOP001", 25);
    EXPECT_TRUE(result);
    
    const Product* product = inventory->getProduct("LAPTOP001");
    EXPECT_EQ(product->getQuantity(), 25);
}

TEST_F(InventoryTest, UpdateQuantityNonExistentProductFails) {
    bool result = inventory->updateQuantity("NONEXISTENT", 10);
    EXPECT_FALSE(result);
}

// Search and Retrieval Tests
TEST_F(InventoryTest, GetAllProductsReturnsCorrectCount) {
    inventory->addProduct(std::move(laptop));
    inventory->addProduct(std::move(mouse));
    
    auto all_products = inventory->getAllProducts();
    EXPECT_EQ(all_products.size(), 2);
}

TEST_F(InventoryTest, SearchProductsByNameFindsResults) {
    inventory->addProduct(std::move(laptop));
    inventory->addProduct(std::move(mouse));
    
    auto results = inventory->searchProducts("Gaming");
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getId(), "LAPTOP001");
    
    auto mouse_results = inventory->searchProducts("Mouse");
    EXPECT_EQ(mouse_results.size(), 1);
    EXPECT_EQ(mouse_results[0]->getId(), "MOUSE001");
}

TEST_F(InventoryTest, SearchProductsNonExistentReturnsEmpty) {
    inventory->addProduct(std::move(laptop));
    
    auto results = inventory->searchProducts("NonExistent");
    EXPECT_TRUE(results.empty());
}

TEST_F(InventoryTest, GetProductsByCategoryFiltersCorrectly) {
    inventory->addProduct(std::move(laptop));
    inventory->addProduct(std::move(mouse));
    inventory->addProduct(std::move(fresh_milk));
    
    auto electronics = inventory->getProductsByCategory("Electronics");
    EXPECT_EQ(electronics.size(), 2);
    
    auto dairy = inventory->getProductsByCategory("Dairy");
    EXPECT_EQ(dairy.size(), 1);
    EXPECT_EQ(dairy[0]->getId(), "MILK001");
}

// Stock Alert Tests
TEST_F(InventoryTest, GetLowStockProductsReturnsCorrectItems) {
    inventory->addProduct(std::move(laptop));      // 15 units
    inventory->addProduct(std::move(mouse));       // 100 units
    inventory->addProduct(std::move(low_stock_item)); // 3 units
    
    auto low_stock = inventory->getLowStockProducts(10);
    EXPECT_EQ(low_stock.size(), 1);
    EXPECT_EQ(low_stock[0]->getId(), "LOWSTOCK001");
}

TEST_F(InventoryTest, GetLowStockProductsWithDifferentThresholds) {
    inventory->addProduct(std::move(laptop));      // 15 units
    inventory->addProduct(std::move(low_stock_item)); // 3 units
    
    auto low_stock_5 = inventory->getLowStockProducts(5);
    EXPECT_EQ(low_stock_5.size(), 1); // Only low_stock_item
    
    auto low_stock_20 = inventory->getLowStockProducts(20);
    EXPECT_EQ(low_stock_20.size(), 2); // Both items
}

TEST_F(InventoryTest, GetExpiredProductsReturnsExpiredItems) {
    inventory->addProduct(std::move(fresh_milk));
    inventory->addProduct(std::move(expired_milk));
    
    auto expired = inventory->getExpiredProducts();
    EXPECT_EQ(expired.size(), 1);
    EXPECT_EQ(expired[0]->getId(), "MILK002");
}

// Statistics Tests
TEST_F(InventoryTest, GetTotalProductCountIsCorrect) {
    EXPECT_EQ(inventory->getTotalProductCount(), 0);
    
    inventory->addProduct(std::move(laptop));
    EXPECT_EQ(inventory->getTotalProductCount(), 1);
    
    inventory->addProduct(std::move(mouse));
    EXPECT_EQ(inventory->getTotalProductCount(), 2);
}

TEST_F(InventoryTest, GetTotalQuantityCalculatesCorrectly) {
    inventory->addProduct(std::move(laptop));    // 15 units
    inventory->addProduct(std::move(mouse));     // 100 units
    
    EXPECT_EQ(inventory->getTotalQuantity(), 115);
}

TEST_F(InventoryTest, GetTotalValueCalculatesCorrectly) {
    inventory->addProduct(std::move(laptop));    // 1299.99 * 15
    inventory->addProduct(std::move(mouse));     // 49.99 * 100
    
    double expected_value = (1299.99 * 15) + (49.99 * 100);
    EXPECT_DOUBLE_EQ(inventory->getTotalValue(), expected_value);
}

// Thread Safety Tests
class InventoryThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
        
        // Pre-populate with some products for concurrent operations
        for (int i = 0; i < 10; ++i) {
            auto product = std::make_unique<Product>(
                "P" + std::to_string(i),
                "Product " + std::to_string(i),
                "Category",
                99.99,
                100
            );
            inventory->addProduct(std::move(product));
        }
    }
    
    std::unique_ptr<Inventory> inventory;
};

TEST_F(InventoryThreadSafetyTest, ConcurrentAddOperationsAreSafe) {
    std::vector<std::thread> threads;
    std::atomic<int> successful_adds{0};
    
    // Try to add products concurrently
    for (int i = 0; i < 50; ++i) {
        threads.emplace_back([&, i]() {
            auto product = std::make_unique<Product>(
                "THREAD_P" + std::to_string(i),
                "Thread Product " + std::to_string(i),
                "ThreadCategory",
                50.0,
                10
            );
            
            if (inventory->addProduct(std::move(product))) {
                successful_adds++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_adds.load(), 50);
    EXPECT_GE(inventory->getTotalProductCount(), 60); // 10 initial + 50 added
}

TEST_F(InventoryThreadSafetyTest, ConcurrentQuantityUpdatesAreSafe) {
    std::vector<std::thread> threads;
    const int updates_per_thread = 100;
    const int num_threads = 10;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < updates_per_thread; ++j) {
                // Update different products to avoid contention on same product
                std::string product_id = "P" + std::to_string(j % 10);
                inventory->updateQuantity(product_id, 50 + j);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify inventory is still in a consistent state
    EXPECT_EQ(inventory->getTotalProductCount(), 10);
    
    // All products should have been updated
    for (int i = 0; i < 10; ++i) {
        const Product* product = inventory->getProduct("P" + std::to_string(i));
        ASSERT_NE(product, nullptr);
        // Quantity should be the last update for this product
    }
}

TEST_F(InventoryThreadSafetyTest, ConcurrentSearchOperationsAreSafe) {
    std::vector<std::future<std::vector<const Product*>>> futures;
    
    // Perform concurrent searches
    for (int i = 0; i < 20; ++i) {
        futures.push_back(std::async(std::launch::async, [&]() {
            return inventory->searchProducts("Product");
        }));
    }
    
    // All searches should complete successfully
    for (auto& future : futures) {
        auto results = future.get();
        EXPECT_GE(results.size(), 10); // Should find at least the 10 pre-populated products
    }
}

TEST_F(InventoryThreadSafetyTest, ConcurrentMixedOperationsAreSafe) {
    std::vector<std::thread> threads;
    std::atomic<bool> stop_flag{false};
    
    // Reader thread
    threads.emplace_back([&]() {
        int iterations = 0;
        while (!stop_flag.load() && iterations < 1000) {
            auto products = inventory->getAllProducts();
            auto low_stock = inventory->getLowStockProducts();
            auto total_count = inventory->getTotalProductCount();
            iterations++;
        }
    });
    
    // Writer thread
    threads.emplace_back([&]() {
        int iterations = 0;
        while (!stop_flag.load() && iterations < 100) {
            auto product = std::make_unique<Product>(
                "MIXED_P" + std::to_string(iterations),
                "Mixed Product",
                "Mixed",
                25.0,
                5
            );
            inventory->addProduct(std::move(product));
            iterations++;
        }
    });
    
    // Updater thread
    threads.emplace_back([&]() {
        int iterations = 0;
        while (!stop_flag.load() && iterations < 500) {
            inventory->updateQuantity("P0", 100 + iterations);
            iterations++;
        }
    });
    
    // Let threads run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop_flag.store(true);
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify inventory is still consistent
    EXPECT_GE(inventory->getTotalProductCount(), 10);
}

// Performance Tests
class InventoryPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
    }
    
    std::unique_ptr<Inventory> inventory;
};

TEST_F(InventoryPerformanceTest, LargeScaleAddPerformance) {
    const int num_products = 10000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < num_products; ++i) {
        auto product = std::make_unique<Product>(
            "PERF_P" + std::to_string(i),
            "Performance Product " + std::to_string(i),
            "Performance",
            99.99,
            100
        );
        inventory->addProduct(std::move(product));
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    EXPECT_EQ(inventory->getTotalProductCount(), num_products);
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
}

TEST_F(InventoryPerformanceTest, LargeScaleSearchPerformance) {
    // Add products first
    const int num_products = 1000;
    for (int i = 0; i < num_products; ++i) {
        auto product = std::make_unique<Product>(
            "SEARCH_P" + std::to_string(i),
            "Search Product " + std::to_string(i % 100), // Some duplicates
            "SearchCategory",
            99.99,
            100
        );
        inventory->addProduct(std::move(product));
    }
    
    auto start = high_resolution_clock::now();
    
    // Perform many searches
    for (int i = 0; i < 1000; ++i) {
        auto results = inventory->searchProducts("Search Product " + std::to_string(i % 100));
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}

// Edge Cases and Error Conditions
TEST_F(InventoryTest, EmptyInventoryOperations) {
    // Operations on empty inventory should handle gracefully
    EXPECT_EQ(inventory->getTotalProductCount(), 0);
    EXPECT_EQ(inventory->getTotalQuantity(), 0);
    EXPECT_DOUBLE_EQ(inventory->getTotalValue(), 0.0);
    
    auto all_products = inventory->getAllProducts();
    EXPECT_TRUE(all_products.empty());
    
    auto low_stock = inventory->getLowStockProducts();
    EXPECT_TRUE(low_stock.empty());
    
    auto expired = inventory->getExpiredProducts();
    EXPECT_TRUE(expired.empty());
}

TEST_F(InventoryTest, NullProductHandling) {
    // Adding null product should be handled gracefully
    std::unique_ptr<Product> null_product;
    bool result = inventory->addProduct(std::move(null_product));
    EXPECT_FALSE(result);
}

TEST_F(InventoryTest, LargeQuantityOperations) {
    auto product = std::make_unique<Product>("LARGE001", "Large Quantity", "Test", 1.0, INT_MAX);
    inventory->addProduct(std::move(product));
    
    EXPECT_EQ(inventory->getProduct("LARGE001")->getQuantity(), INT_MAX);
    EXPECT_EQ(inventory->getTotalQuantity(), INT_MAX);
}