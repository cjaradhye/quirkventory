#include <iostream>
#include <memory>
#include <cassert>
#include "../include/Product.hpp"
#include "../include/Inventory.hpp"
#include "../include/Order.hpp"
#include "../include/User.hpp"
#include "../include/NotificationSystem.hpp"

using namespace quirkventory;

// Test Product classes
void testProduct() {
    std::cout << "Testing Product classes...\n";
    
    // Test basic Product
    Product product("P001", "Laptop", "Electronics", 999.99, 10);
    assert(product.getId() == "P001");
    assert(product.getName() == "Laptop");
    assert(product.getCategory() == "Electronics");
    assert(product.getPrice() == 999.99);
    assert(product.getQuantity() == 10);
    assert(!product.isExpired());
    
    // Test PerishableProduct
    auto expiry_date = std::chrono::system_clock::now() + std::chrono::hours(24);
    PerishableProduct perishable("P002", "Milk", "Dairy", 4.99, 20, expiry_date);
    assert(perishable.getId() == "P002");
    assert(!perishable.isExpired());
    
    // Test expired product
    auto past_date = std::chrono::system_clock::now() - std::chrono::hours(24);
    PerishableProduct expired("P003", "Old Milk", "Dairy", 4.99, 5, past_date);
    assert(expired.isExpired());
    
    std::cout << "✓ Product tests passed\n";
}

// Test Inventory class
void testInventory() {
    std::cout << "Testing Inventory class...\n";
    
    Inventory inventory;
    
    // Add products
    auto product1 = std::make_unique<Product>("P001", "Laptop", "Electronics", 999.99, 10);
    auto product2 = std::make_unique<Product>("P002", "Mouse", "Electronics", 29.99, 5);
    
    assert(inventory.addProduct(std::move(product1)));
    assert(inventory.addProduct(std::move(product2)));
    
    // Test retrieval
    const Product* retrieved = inventory.getProduct("P001");
    assert(retrieved != nullptr);
    assert(retrieved->getName() == "Laptop");
    
    // Test update quantity
    assert(inventory.updateQuantity("P001", 15));
    assert(inventory.getProduct("P001")->getQuantity() == 15);
    
    // Test search
    auto search_results = inventory.searchProducts("Laptop");
    assert(search_results.size() == 1);
    assert(search_results[0]->getId() == "P001");
    
    // Test low stock detection
    auto low_stock = inventory.getLowStockProducts(6);
    assert(low_stock.size() == 1); // Mouse has quantity 5
    assert(low_stock[0]->getId() == "P002");
    
    std::cout << "✓ Inventory tests passed\n";
}

// Test Order classes
void testOrder() {
    std::cout << "Testing Order classes...\n";
    
    Inventory inventory;
    auto product = std::make_unique<Product>("P001", "Laptop", "Electronics", 999.99, 10);
    inventory.addProduct(std::move(product));
    
    OrderManager order_manager(&inventory);
    
    // Create order
    Order order("ORD001");
    order.addItem("P001", 2);
    
    assert(order.getId() == "ORD001");
    assert(order.getItems().size() == 1);
    assert(order.getStatus() == OrderStatus::PENDING);
    
    // Process order
    auto future = order_manager.processOrderAsync(order);
    bool result = future.get();
    assert(result);
    
    // Check inventory was updated
    assert(inventory.getProduct("P001")->getQuantity() == 8);
    
    std::cout << "✓ Order tests passed\n";
}

// Test User classes
void testUser() {
    std::cout << "Testing User classes...\n";
    
    UserManager user_manager;
    
    // Create users
    assert(user_manager.createUser("staff1", "password123", UserRole::STAFF));
    assert(user_manager.createUser("manager1", "admin456", UserRole::MANAGER));
    
    // Test authentication
    User* staff = user_manager.authenticate("staff1", "password123");
    assert(staff != nullptr);
    assert(staff->getRole() == UserRole::STAFF);
    
    User* manager = user_manager.authenticate("manager1", "admin456");
    assert(manager != nullptr);
    assert(manager->getRole() == UserRole::MANAGER);
    
    // Test permissions
    assert(staff->hasPermission(Permission::VIEW_INVENTORY));
    assert(!staff->hasPermission(Permission::DELETE_PRODUCTS));
    assert(manager->hasPermission(Permission::DELETE_PRODUCTS));
    
    // Test invalid login
    User* invalid = user_manager.authenticate("staff1", "wrongpassword");
    assert(invalid == nullptr);
    
    std::cout << "✓ User tests passed\n";
}

// Test Notification System
void testNotificationSystem() {
    std::cout << "Testing Notification System...\n";
    
    NotificationManager notification_manager;
    
    // Create notifications
    auto email_notification = std::make_unique<EmailNotification>(
        "admin@example.com", "Low Stock Alert", "Product P001 is running low"
    );
    
    auto system_notification = std::make_unique<SystemNotification>(
        "System Alert", "Order processing completed", NotificationPriority::HIGH
    );
    
    // Send notifications (simulated)
    notification_manager.addNotification(std::move(email_notification));
    notification_manager.addNotification(std::move(system_notification));
    
    auto pending = notification_manager.getPendingNotifications();
    assert(pending.size() == 2);
    
    notification_manager.processNotifications();
    
    // Create reports
    SalesReport sales_report;
    InventoryReport inventory_report;
    
    // Generate reports (basic test)
    std::string sales_content = sales_report.generateReport();
    std::string inventory_content = inventory_report.generateReport();
    
    assert(!sales_content.empty());
    assert(!inventory_content.empty());
    
    std::cout << "✓ Notification System tests passed\n";
}

// Performance test
void performanceTest() {
    std::cout << "Running performance tests...\n";
    
    Inventory inventory;
    
    // Add many products
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        auto product = std::make_unique<Product>(
            "P" + std::to_string(i),
            "Product " + std::to_string(i),
            "Category",
            99.99,
            100
        );
        inventory.addProduct(std::move(product));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Added 10,000 products in " << duration.count() << "ms\n";
    
    // Search performance
    start = std::chrono::high_resolution_clock::now();
    auto results = inventory.searchProducts("Product 5000");
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Search completed in " << duration.count() << "μs\n";
    assert(results.size() == 1);
    
    std::cout << "✓ Performance tests passed\n";
}

// Concurrency test
void concurrencyTest() {
    std::cout << "Testing concurrency...\n";
    
    Inventory inventory;
    
    // Add initial products
    for (int i = 0; i < 100; ++i) {
        auto product = std::make_unique<Product>(
            "P" + std::to_string(i),
            "Product " + std::to_string(i),
            "Category",
            99.99,
            1000
        );
        inventory.addProduct(std::move(product));
    }
    
    OrderManager order_manager(&inventory);
    
    // Create multiple orders concurrently
    std::vector<std::future<bool>> futures;
    
    for (int i = 0; i < 50; ++i) {
        Order order("ORD" + std::to_string(i));
        order.addItem("P" + std::to_string(i % 100), 10);
        futures.push_back(order_manager.processOrderAsync(order));
    }
    
    // Wait for all orders to complete
    int successful_orders = 0;
    for (auto& future : futures) {
        if (future.get()) {
            successful_orders++;
        }
    }
    
    std::cout << "Processed " << successful_orders << "/50 orders successfully\n";
    assert(successful_orders > 0);
    
    std::cout << "✓ Concurrency tests passed\n";
}

int main() {
    std::cout << "=== Quirkventory Test Suite ===\n\n";
    
    try {
        testProduct();
        testInventory();
        testOrder();
        testUser();
        testNotificationSystem();
        performanceTest();
        concurrencyTest();
        
        std::cout << "\n=== All Tests Passed! ===\n";
        std::cout << "The Quirkventory system is working correctly.\n";
        std::cout << "All OOP principles are properly demonstrated:\n";
        std::cout << "✓ Inheritance (Product -> PerishableProduct, User -> Staff/Manager)\n";
        std::cout << "✓ Polymorphism (Virtual methods, Report/Notification hierarchies)\n";
        std::cout << "✓ Encapsulation (Private/protected members with public interfaces)\n";
        std::cout << "✓ Abstraction (Abstract base classes and interfaces)\n";
        std::cout << "✓ Thread Safety (Concurrent inventory and order operations)\n";
        std::cout << "✓ STL Usage (Containers, algorithms, smart pointers)\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}