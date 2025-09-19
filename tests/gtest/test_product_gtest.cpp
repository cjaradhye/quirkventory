#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <memory>
#include "../../include/Product.hpp"

using namespace quirkventory;
using namespace std::chrono;

// Test Fixture for Product Tests
class ProductTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test data
        product_id = "P001";
        product_name = "Test Laptop";
        product_category = "Electronics";
        product_price = 999.99;
        product_quantity = 10;
        
        // Create test product
        product = std::make_unique<Product>(
            product_id, product_name, product_category, 
            product_price, product_quantity
        );
        
        // Set up perishable product test data
        expiry_date = system_clock::now() + hours(72); // 3 days from now
        past_expiry_date = system_clock::now() - hours(24); // 1 day ago
    }
    
    void TearDown() override {
        // Clean up after each test
        product.reset();
    }
    
    // Test data
    std::string product_id;
    std::string product_name;
    std::string product_category;
    double product_price;
    int product_quantity;
    std::unique_ptr<Product> product;
    
    // Perishable product test data
    system_clock::time_point expiry_date;
    system_clock::time_point past_expiry_date;
};

// Basic Product Tests
TEST_F(ProductTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(product->getId(), product_id);
    EXPECT_EQ(product->getName(), product_name);
    EXPECT_EQ(product->getCategory(), product_category);
    EXPECT_DOUBLE_EQ(product->getPrice(), product_price);
    EXPECT_EQ(product->getQuantity(), product_quantity);
}

TEST_F(ProductTest, GettersReturnCorrectValues) {
    EXPECT_EQ(product->getId(), "P001");
    EXPECT_EQ(product->getName(), "Test Laptop");
    EXPECT_EQ(product->getCategory(), "Electronics");
    EXPECT_DOUBLE_EQ(product->getPrice(), 999.99);
    EXPECT_EQ(product->getQuantity(), 10);
}

TEST_F(ProductTest, SetPriceUpdatesCorrectly) {
    double new_price = 1299.99;
    product->setPrice(new_price);
    EXPECT_DOUBLE_EQ(product->getPrice(), new_price);
}

TEST_F(ProductTest, SetQuantityUpdatesCorrectly) {
    int new_quantity = 25;
    product->setQuantity(new_quantity);
    EXPECT_EQ(product->getQuantity(), new_quantity);
}

TEST_F(ProductTest, AddQuantityIncreasesStock) {
    int initial_quantity = product->getQuantity();
    int add_amount = 15;
    
    product->addQuantity(add_amount);
    EXPECT_EQ(product->getQuantity(), initial_quantity + add_amount);
}

TEST_F(ProductTest, RemoveQuantityDecreasesStock) {
    int initial_quantity = product->getQuantity();
    int remove_amount = 5;
    
    bool result = product->removeQuantity(remove_amount);
    EXPECT_TRUE(result);
    EXPECT_EQ(product->getQuantity(), initial_quantity - remove_amount);
}

TEST_F(ProductTest, RemoveQuantityFailsWhenInsufficientStock) {
    int remove_amount = product->getQuantity() + 1; // More than available
    
    bool result = product->removeQuantity(remove_amount);
    EXPECT_FALSE(result);
    EXPECT_EQ(product->getQuantity(), product_quantity); // Should remain unchanged
}

TEST_F(ProductTest, BasicProductIsNotExpired) {
    EXPECT_FALSE(product->isExpired());
}

TEST_F(ProductTest, BasicProductExpiryInfoIsEmpty) {
    std::string expiry_info = product->getExpiryInfo();
    EXPECT_EQ(expiry_info, "No expiry date");
}

TEST_F(ProductTest, ProductInfoContainsAllDetails) {
    std::string info = product->getInfo();
    
    EXPECT_THAT(info, ::testing::HasSubstr(product_id));
    EXPECT_THAT(info, ::testing::HasSubstr(product_name));
    EXPECT_THAT(info, ::testing::HasSubstr(product_category));
    EXPECT_THAT(info, ::testing::HasSubstr(std::to_string(product_price)));
    EXPECT_THAT(info, ::testing::HasSubstr(std::to_string(product_quantity)));
}

TEST_F(ProductTest, CloneCreatesIdenticalProduct) {
    auto cloned_product = product->clone();
    
    EXPECT_EQ(cloned_product->getId(), product->getId());
    EXPECT_EQ(cloned_product->getName(), product->getName());
    EXPECT_EQ(cloned_product->getCategory(), product->getCategory());
    EXPECT_DOUBLE_EQ(cloned_product->getPrice(), product->getPrice());
    EXPECT_EQ(cloned_product->getQuantity(), product->getQuantity());
    
    // Ensure it's a different object
    EXPECT_NE(cloned_product.get(), product.get());
}

// Edge Cases and Error Conditions
TEST_F(ProductTest, NegativePriceIsAccepted) {
    // Test that negative prices are accepted (could be discounts)
    product->setPrice(-10.0);
    EXPECT_DOUBLE_EQ(product->getPrice(), -10.0);
}

TEST_F(ProductTest, ZeroQuantityIsValid) {
    product->setQuantity(0);
    EXPECT_EQ(product->getQuantity(), 0);
}

TEST_F(ProductTest, LargeQuantityIsHandled) {
    int large_quantity = 1000000;
    product->setQuantity(large_quantity);
    EXPECT_EQ(product->getQuantity(), large_quantity);
}

// PerishableProduct Tests
class PerishableProductTest : public ::testing::Test {
protected:
    void SetUp() override {
        future_expiry = system_clock::now() + hours(72);
        past_expiry = system_clock::now() - hours(24);
        
        fresh_product = std::make_unique<PerishableProduct>(
            "MILK001", "Fresh Milk", "Dairy", 4.99, 20, future_expiry
        );
        
        expired_product = std::make_unique<PerishableProduct>(
            "MILK002", "Old Milk", "Dairy", 4.99, 5, past_expiry
        );
    }
    
    system_clock::time_point future_expiry;
    system_clock::time_point past_expiry;
    std::unique_ptr<PerishableProduct> fresh_product;
    std::unique_ptr<PerishableProduct> expired_product;
};

TEST_F(PerishableProductTest, FreshProductIsNotExpired) {
    EXPECT_FALSE(fresh_product->isExpired());
}

TEST_F(PerishableProductTest, ExpiredProductIsExpired) {
    EXPECT_TRUE(expired_product->isExpired());
}

TEST_F(PerishableProductTest, ExpiryDateIsCorrect) {
    auto retrieved_expiry = fresh_product->getExpiryDate();
    auto duration_diff = abs(duration_cast<seconds>(retrieved_expiry - future_expiry).count());
    EXPECT_LT(duration_diff, 1); // Within 1 second tolerance
}

TEST_F(PerishableProductTest, DaysUntilExpiryIsCorrect) {
    int days = fresh_product->getDaysUntilExpiry();
    EXPECT_GE(days, 2); // Should be at least 2 days (we set 72 hours)
    EXPECT_LE(days, 3); // Should be at most 3 days
}

TEST_F(PerishableProductTest, ExpiredProductHasNegativeDaysUntilExpiry) {
    int days = expired_product->getDaysUntilExpiry();
    EXPECT_LT(days, 0); // Should be negative for expired products
}

TEST_F(PerishableProductTest, ExpiryInfoContainsDateInformation) {
    std::string expiry_info = fresh_product->getExpiryInfo();
    EXPECT_THAT(expiry_info, ::testing::HasSubstr("days"));
    
    std::string expired_info = expired_product->getExpiryInfo();
    EXPECT_THAT(expired_info, ::testing::HasSubstr("Expired"));
}

TEST_F(PerishableProductTest, PerishableProductInfoIncludesExpiryDetails) {
    std::string info = fresh_product->getInfo();
    
    EXPECT_THAT(info, ::testing::HasSubstr("Fresh Milk"));
    EXPECT_THAT(info, ::testing::HasSubstr("Dairy"));
    EXPECT_THAT(info, ::testing::HasSubstr("Expires"));
}

TEST_F(PerishableProductTest, CloneCreatesCorrectPerishableProduct) {
    auto cloned = fresh_product->clone();
    auto* perishable_clone = dynamic_cast<PerishableProduct*>(cloned.get());
    
    ASSERT_NE(perishable_clone, nullptr);
    EXPECT_EQ(perishable_clone->getId(), fresh_product->getId());
    EXPECT_EQ(perishable_clone->isExpired(), fresh_product->isExpired());
    
    auto original_expiry = fresh_product->getExpiryDate();
    auto cloned_expiry = perishable_clone->getExpiryDate();
    auto diff = abs(duration_cast<seconds>(original_expiry - cloned_expiry).count());
    EXPECT_LT(diff, 1); // Within 1 second tolerance
}

// Polymorphism Tests
class PolymorphismTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto future_date = system_clock::now() + hours(48);
        
        products.push_back(std::make_unique<Product>("P001", "Laptop", "Electronics", 999.99, 10));
        products.push_back(std::make_unique<PerishableProduct>("P002", "Milk", "Dairy", 4.99, 20, future_date));
    }
    
    std::vector<std::unique_ptr<Product>> products;
};

TEST_F(PolymorphismTest, VirtualMethodsWorkCorrectly) {
    for (const auto& product : products) {
        // All products should have valid info
        std::string info = product->getInfo();
        EXPECT_FALSE(info.empty());
        
        // Expiry behavior should be polymorphic
        bool is_expired = product->isExpired();
        std::string expiry_info = product->getExpiryInfo();
        EXPECT_FALSE(expiry_info.empty());
        
        // Clone should work polymorphically
        auto clone = product->clone();
        EXPECT_EQ(clone->getId(), product->getId());
        EXPECT_EQ(clone->getName(), product->getName());
    }
}

// Performance Tests
class ProductPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a large number of products for performance testing
        const int num_products = 10000;
        products.reserve(num_products);
        
        for (int i = 0; i < num_products; ++i) {
            products.push_back(std::make_unique<Product>(
                "P" + std::to_string(i),
                "Product " + std::to_string(i),
                "Category",
                99.99,
                100
            ));
        }
    }
    
    std::vector<std::unique_ptr<Product>> products;
};

TEST_F(ProductPerformanceTest, LargeScaleOperationsPerformWell) {
    auto start = high_resolution_clock::now();
    
    // Perform operations on all products
    for (auto& product : products) {
        product->addQuantity(10);
        product->removeQuantity(5);
        product->setPrice(product->getPrice() * 1.1); // 10% price increase
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    // Should complete within reasonable time (adjust based on your requirements)
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
}

TEST_F(ProductPerformanceTest, CloningPerformanceIsAcceptable) {
    auto start = high_resolution_clock::now();
    
    std::vector<std::unique_ptr<Product>> cloned_products;
    cloned_products.reserve(products.size());
    
    for (const auto& product : products) {
        cloned_products.push_back(product->clone());
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    // Cloning should be efficient
    EXPECT_LT(duration.count(), 2000); // Less than 2 seconds for 10k products
    EXPECT_EQ(cloned_products.size(), products.size());
}

// Thread Safety Tests (if applicable)
TEST(ProductThreadSafetyTest, ConcurrentPriceUpdatesAreSafe) {
    auto product = std::make_unique<Product>("P001", "Test", "Category", 100.0, 10);
    
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int updates_per_thread = 100;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&product, updates_per_thread]() {
            for (int j = 0; j < updates_per_thread; ++j) {
                product->setPrice(product->getPrice() + 0.01);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // The final price should reflect all updates
    // Note: This test assumes the Product class has some form of thread safety
    // If not, this test might fail due to race conditions
    double expected_final_price = 100.0 + (num_threads * updates_per_thread * 0.01);
    EXPECT_NEAR(product->getPrice(), expected_final_price, 0.1); // Small tolerance for floating-point errors
}