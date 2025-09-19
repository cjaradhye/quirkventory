#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <future>
#include <vector>
#include <memory>
#include "../../include/Order.hpp"
#include "../../include/Inventory.hpp"
#include "../../include/Product.hpp"

using namespace quirkventory;

// Mock class for testing Order dependencies
class MockInventory {
public:
    MOCK_METHOD(const Product*, getProduct, (const std::string& product_id), (const));
    MOCK_METHOD(bool, updateQuantity, (const std::string& product_id, int new_quantity), ());
    MOCK_METHOD(bool, removeQuantity, (const std::string& product_id, int quantity), ());
};

// Test Fixture for Order Tests
class OrderTest : public ::testing::Test {
protected:
    void SetUp() override {
        order = std::make_unique<Order>("ORD001");
        
        // Set up real inventory for integration tests
        inventory = std::make_unique<Inventory>();
        
        // Add test products to inventory
        auto laptop = std::make_unique<Product>("LAPTOP001", "Gaming Laptop", "Electronics", 1299.99, 10);
        auto mouse = std::make_unique<Product>("MOUSE001", "Wireless Mouse", "Electronics", 49.99, 50);
        auto keyboard = std::make_unique<Product>("KEYBOARD001", "Mechanical Keyboard", "Electronics", 129.99, 25);
        
        inventory->addProduct(std::move(laptop));
        inventory->addProduct(std::move(mouse));
        inventory->addProduct(std::move(keyboard));
        
        order_manager = std::make_unique<OrderManager>(inventory.get());
    }
    
    void TearDown() override {
        order.reset();
        order_manager.reset();
        inventory.reset();
    }
    
    std::unique_ptr<Order> order;
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<OrderManager> order_manager;
};

// Basic Order Tests
TEST_F(OrderTest, OrderCreationWithValidId) {
    EXPECT_EQ(order->getId(), "ORD001");
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
    EXPECT_TRUE(order->getItems().empty());
}

TEST_F(OrderTest, AddItemToOrder) {
    order->addItem("LAPTOP001", 2);
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].first, "LAPTOP001");
    EXPECT_EQ(items[0].second, 2);
}

TEST_F(OrderTest, AddMultipleItemsToOrder) {
    order->addItem("LAPTOP001", 1);
    order->addItem("MOUSE001", 3);
    order->addItem("KEYBOARD001", 2);
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 3);
}

TEST_F(OrderTest, AddDuplicateItemUpdatesQuantity) {
    order->addItem("LAPTOP001", 2);
    order->addItem("LAPTOP001", 3); // Should update quantity to 3, not add new item
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].second, 3);
}

TEST_F(OrderTest, RemoveItemFromOrder) {
    order->addItem("LAPTOP001", 2);
    order->addItem("MOUSE001", 1);
    
    order->removeItem("LAPTOP001");
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].first, "MOUSE001");
}

TEST_F(OrderTest, RemoveNonExistentItem) {
    order->addItem("LAPTOP001", 2);
    
    // Removing non-existent item should not crash
    order->removeItem("NONEXISTENT");
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
}

TEST_F(OrderTest, UpdateItemQuantity) {
    order->addItem("LAPTOP001", 2);
    order->updateItemQuantity("LAPTOP001", 5);
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].second, 5);
}

TEST_F(OrderTest, UpdateNonExistentItemQuantity) {
    order->addItem("LAPTOP001", 2);
    
    // Updating non-existent item should add it
    order->updateItemQuantity("MOUSE001", 3);
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 2);
}

TEST_F(OrderTest, SetOrderStatus) {
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
    
    order->setStatus(OrderStatus::PROCESSING);
    EXPECT_EQ(order->getStatus(), OrderStatus::PROCESSING);
    
    order->setStatus(OrderStatus::COMPLETED);
    EXPECT_EQ(order->getStatus(), OrderStatus::COMPLETED);
    
    order->setStatus(OrderStatus::CANCELLED);
    EXPECT_EQ(order->getStatus(), OrderStatus::CANCELLED);
}

TEST_F(OrderTest, CalculateTotalWithValidInventory) {
    order->addItem("LAPTOP001", 1);  // 1299.99 * 1
    order->addItem("MOUSE001", 2);   // 49.99 * 2
    
    double expected_total = 1299.99 + (49.99 * 2);
    double actual_total = order->calculateTotal(*inventory);
    
    EXPECT_DOUBLE_EQ(actual_total, expected_total);
}

TEST_F(OrderTest, CalculateTotalWithNonExistentProduct) {
    order->addItem("LAPTOP001", 1);     // 1299.99
    order->addItem("NONEXISTENT", 1);   // Should be ignored (price 0)
    
    double expected_total = 1299.99;
    double actual_total = order->calculateTotal(*inventory);
    
    EXPECT_DOUBLE_EQ(actual_total, expected_total);
}

// OrderManager Tests
TEST_F(OrderTest, ValidateValidOrder) {
    order->addItem("LAPTOP001", 2);   // Available: 10
    order->addItem("MOUSE001", 5);    // Available: 50
    
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_TRUE(is_valid);
    
    auto errors = order_manager->getValidationErrors(*order);
    EXPECT_TRUE(errors.empty());
}

TEST_F(OrderTest, ValidateOrderWithInsufficientStock) {
    order->addItem("LAPTOP001", 15);  // Available: 10, Requested: 15
    
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
    
    auto errors = order_manager->getValidationErrors(*order);
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors[0], ::testing::HasSubstr("Insufficient stock"));
    EXPECT_THAT(errors[0], ::testing::HasSubstr("LAPTOP001"));
}

TEST_F(OrderTest, ValidateOrderWithNonExistentProduct) {
    order->addItem("NONEXISTENT", 1);
    
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
    
    auto errors = order_manager->getValidationErrors(*order);
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors[0], ::testing::HasSubstr("Product not found"));
    EXPECT_THAT(errors[0], ::testing::HasSubstr("NONEXISTENT"));
}

TEST_F(OrderTest, ValidateEmptyOrder) {
    // Empty order should be invalid
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
    
    auto errors = order_manager->getValidationErrors(*order);
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors[0], ::testing::HasSubstr("empty"));
}

TEST_F(OrderTest, ProcessValidOrderSynchronously) {
    order->addItem("LAPTOP001", 2);   // Will reduce from 10 to 8
    order->addItem("MOUSE001", 5);    // Will reduce from 50 to 45
    
    bool result = order_manager->processOrder(*order);
    EXPECT_TRUE(result);
    
    // Check inventory was updated
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), 8);
    EXPECT_EQ(inventory->getProduct("MOUSE001")->getQuantity(), 45);
}

TEST_F(OrderTest, ProcessInvalidOrderFails) {
    order->addItem("LAPTOP001", 15);  // More than available
    
    bool result = order_manager->processOrder(*order);
    EXPECT_FALSE(result);
    
    // Inventory should remain unchanged
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), 10);
}

TEST_F(OrderTest, ProcessOrderAsynchronously) {
    order->addItem("KEYBOARD001", 3);
    
    auto future = order_manager->processOrderAsync(*order);
    bool result = future.get();
    
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory->getProduct("KEYBOARD001")->getQuantity(), 22);
}

// Concurrency Tests
class OrderConcurrencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
        
        // Add products with high stock for concurrent testing
        for (int i = 0; i < 10; ++i) {
            auto product = std::make_unique<Product>(
                "PROD" + std::to_string(i),
                "Product " + std::to_string(i),
                "Category",
                100.0,
                1000  // High stock
            );
            inventory->addProduct(std::move(product));
        }
        
        order_manager = std::make_unique<OrderManager>(inventory.get());
    }
    
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<OrderManager> order_manager;
};

TEST_F(OrderConcurrencyTest, ConcurrentOrderProcessing) {
    std::vector<std::future<bool>> futures;
    std::atomic<int> successful_orders{0};
    
    // Create multiple orders and process them concurrently
    for (int i = 0; i < 50; ++i) {
        Order order("ORD" + std::to_string(i));
        order.addItem("PROD" + std::to_string(i % 10), 10);
        
        futures.push_back(order_manager->processOrderAsync(order));
    }
    
    // Wait for all orders to complete
    for (auto& future : futures) {
        if (future.get()) {
            successful_orders++;
        }
    }
    
    EXPECT_EQ(successful_orders.load(), 50);
    
    // Verify inventory consistency
    for (int i = 0; i < 10; ++i) {
        const Product* product = inventory->getProduct("PROD" + std::to_string(i));
        EXPECT_LE(product->getQuantity(), 1000); // Should be reduced
        EXPECT_GE(product->getQuantity(), 950);   // But not below reasonable amount
    }
}

TEST_F(OrderConcurrencyTest, ConcurrentOrderValidation) {
    std::vector<std::future<bool>> validation_futures;
    
    // Create and validate multiple orders concurrently
    for (int i = 0; i < 100; ++i) {
        validation_futures.push_back(std::async(std::launch::async, [&, i]() {
            Order order("VALIDATION_ORD" + std::to_string(i));
            order.addItem("PROD" + std::to_string(i % 10), 5);
            return order_manager->validateOrder(order);
        }));
    }
    
    // All validations should succeed
    for (auto& future : validation_futures) {
        EXPECT_TRUE(future.get());
    }
}

TEST_F(OrderConcurrencyTest, StressTestOrderProcessing) {
    const int num_orders = 200;
    std::vector<std::future<bool>> futures;
    std::atomic<int> successful_orders{0};
    std::atomic<int> failed_orders{0};
    
    // Create stress test orders
    for (int i = 0; i < num_orders; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            Order order("STRESS_ORD" + std::to_string(i));
            order.addItem("PROD" + std::to_string(i % 10), 5);
            return order_manager->processOrder(order);
        }));
    }
    
    // Collect results
    for (auto& future : futures) {
        if (future.get()) {
            successful_orders++;
        } else {
            failed_orders++;
        }
    }
    
    // Some orders might fail due to stock depletion, but system should remain stable
    EXPECT_GT(successful_orders.load(), 0);
    EXPECT_EQ(successful_orders.load() + failed_orders.load(), num_orders);
    
    // Verify inventory integrity
    for (int i = 0; i < 10; ++i) {
        const Product* product = inventory->getProduct("PROD" + std::to_string(i));
        EXPECT_GE(product->getQuantity(), 0); // Should never go negative
    }
}

// Performance Tests
class OrderPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
        
        // Add many products for performance testing
        for (int i = 0; i < 1000; ++i) {
            auto product = std::make_unique<Product>(
                "PERF_PROD" + std::to_string(i),
                "Performance Product " + std::to_string(i),
                "Performance",
                50.0,
                500
            );
            inventory->addProduct(std::move(product));
        }
        
        order_manager = std::make_unique<OrderManager>(inventory.get());
    }
    
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<OrderManager> order_manager;
};

TEST_F(OrderPerformanceTest, LargeOrderProcessingPerformance) {
    Order large_order("LARGE_ORDER");
    
    // Add many items to the order
    for (int i = 0; i < 100; ++i) {
        large_order.addItem("PERF_PROD" + std::to_string(i), 5);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    bool result = order_manager->processOrder(large_order);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(result);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}

TEST_F(OrderPerformanceTest, ManySmallOrdersPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    int successful_orders = 0;
    for (int i = 0; i < 1000; ++i) {
        Order order("SMALL_ORDER_" + std::to_string(i));
        order.addItem("PERF_PROD" + std::to_string(i % 100), 1);
        
        if (order_manager->processOrder(order)) {
            successful_orders++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_GT(successful_orders, 900); // Most should succeed
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
}

// Edge Cases and Error Conditions
TEST_F(OrderTest, OrderWithZeroQuantity) {
    order->addItem("LAPTOP001", 0);
    
    // Zero quantity should be handled gracefully
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].second, 0);
    
    // Validation should fail for zero quantity
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
}

TEST_F(OrderTest, OrderWithNegativeQuantity) {
    order->addItem("LAPTOP001", -5);
    
    // Negative quantity should be handled
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].second, -5);
    
    // Validation should fail for negative quantity
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
}

TEST_F(OrderTest, EmptyProductId) {
    order->addItem("", 5);
    
    const auto& items = order->getItems();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items[0].first, "");
    
    // Should fail validation
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
}

TEST_F(OrderTest, VeryLargeQuantity) {
    order->addItem("LAPTOP001", INT_MAX);
    
    // Should fail validation due to insufficient stock
    bool is_valid = order_manager->validateOrder(*order);
    EXPECT_FALSE(is_valid);
    
    auto errors = order_manager->getValidationErrors(*order);
    EXPECT_THAT(errors[0], ::testing::HasSubstr("Insufficient stock"));
}

// Order State Management Tests
TEST_F(OrderTest, OrderStatusTransitions) {
    // Test valid status transitions
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
    
    order->setStatus(OrderStatus::PROCESSING);
    EXPECT_EQ(order->getStatus(), OrderStatus::PROCESSING);
    
    order->setStatus(OrderStatus::COMPLETED);
    EXPECT_EQ(order->getStatus(), OrderStatus::COMPLETED);
}

TEST_F(OrderTest, OrderStatusFromCancelledToPending) {
    order->setStatus(OrderStatus::CANCELLED);
    order->setStatus(OrderStatus::PENDING);
    
    // Should allow status change back to pending
    EXPECT_EQ(order->getStatus(), OrderStatus::PENDING);
}