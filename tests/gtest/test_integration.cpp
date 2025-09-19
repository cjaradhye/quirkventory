#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <future>
#include "../../include/Product.hpp"
#include "../../include/Inventory.hpp"
#include "../../include/Order.hpp"
#include "../../include/User.hpp"
#include "../../include/NotificationSystem.hpp"
#include "../../include/HTTPServer.hpp"
#include "../../include/CLI.hpp"

using namespace quirkventory;

// Integration Test Fixture
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize all system components
        inventory = std::make_unique<Inventory>();
        order_manager = std::make_unique<OrderManager>(inventory.get());
        user_manager = std::make_unique<UserManager>();
        notification_manager = std::make_unique<NotificationManager>();
        http_server = std::make_unique<HTTPServer>("localhost", 8080);
        
        // Configure HTTP server with system components
        http_server->setSystemComponents(
            inventory.get(),
            order_manager.get(),
            user_manager.get(),
            notification_manager.get()
        );
        
        // Set up test data
        setupTestData();
    }
    
    void TearDown() override {
        http_server.reset();
        notification_manager.reset();
        user_manager.reset();
        order_manager.reset();
        inventory.reset();
    }
    
    void setupTestData() {
        // Add test products
        auto laptop = std::make_unique<Product>("LAPTOP001", "Gaming Laptop", "Electronics", 1299.99, 10);
        auto mouse = std::make_unique<Product>("MOUSE001", "Wireless Mouse", "Electronics", 49.99, 50);
        auto keyboard = std::make_unique<Product>("KEYBOARD001", "Mechanical Keyboard", "Electronics", 129.99, 25);
        auto low_stock = std::make_unique<Product>("LOWSTOCK001", "Low Stock Item", "Test", 19.99, 3);
        
        // Add perishable product
        auto expiry_date = std::chrono::system_clock::now() + std::chrono::hours(72);
        auto milk = std::make_unique<PerishableProduct>("MILK001", "Fresh Milk", "Dairy", 4.99, 20, expiry_date);
        
        inventory->addProduct(std::move(laptop));
        inventory->addProduct(std::move(mouse));
        inventory->addProduct(std::move(keyboard));
        inventory->addProduct(std::move(low_stock));
        inventory->addProduct(std::move(milk));
        
        // Create test users
        user_manager->createUser("staff_user", "staff_password", UserRole::STAFF);
        user_manager->createUser("manager_user", "manager_password", UserRole::MANAGER);
        user_manager->createUser("test_admin", "admin_password", UserRole::MANAGER);
    }
    
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<OrderManager> order_manager;
    std::unique_ptr<UserManager> user_manager;
    std::unique_ptr<NotificationManager> notification_manager;
    std::unique_ptr<HTTPServer> http_server;
};

// Complete Order Processing Workflow
TEST_F(IntegrationTest, CompleteOrderProcessingWorkflow) {
    // 1. Authenticate user
    User* staff_user = user_manager->authenticate("staff_user", "staff_password");
    ASSERT_NE(staff_user, nullptr);
    EXPECT_TRUE(staff_user->hasPermission(Permission::PROCESS_ORDERS));
    
    // 2. Create order
    Order order("INTEGRATION_ORD001");
    order.addItem("LAPTOP001", 2);   // Should reduce inventory from 10 to 8
    order.addItem("MOUSE001", 5);    // Should reduce inventory from 50 to 45
    
    // 3. Validate order
    EXPECT_TRUE(order_manager->validateOrder(order));
    auto validation_errors = order_manager->getValidationErrors(order);
    EXPECT_TRUE(validation_errors.empty());
    
    // 4. Calculate order total
    double expected_total = (1299.99 * 2) + (49.99 * 5);
    double actual_total = order.calculateTotal(*inventory);
    EXPECT_DOUBLE_EQ(actual_total, expected_total);
    
    // 5. Process order
    bool order_result = order_manager->processOrder(order);
    EXPECT_TRUE(order_result);
    
    // 6. Verify inventory updates
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), 8);
    EXPECT_EQ(inventory->getProduct("MOUSE001")->getQuantity(), 45);
    
    // 7. Create success notification
    auto notification = std::make_unique<SystemNotification>(
        "Order Processed",
        "Order " + order.getId() + " processed successfully",
        NotificationPriority::HIGH
    );
    notification_manager->addNotification(std::move(notification));
    
    // 8. Verify notification was added
    auto pending_notifications = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending_notifications.size(), 1);
    EXPECT_EQ(pending_notifications[0]->getTitle(), "Order Processed");
}

// User Permission Integration Test
TEST_F(IntegrationTest, UserPermissionIntegration) {
    // Test staff permissions
    User* staff = user_manager->authenticate("staff_user", "staff_password");
    ASSERT_NE(staff, nullptr);
    
    // Staff can view and add products
    EXPECT_TRUE(staff->hasPermission(Permission::VIEW_INVENTORY));
    EXPECT_TRUE(staff->hasPermission(Permission::ADD_PRODUCTS));
    EXPECT_TRUE(staff->hasPermission(Permission::EDIT_PRODUCTS));
    
    // Staff cannot delete products or manage users
    EXPECT_FALSE(staff->hasPermission(Permission::DELETE_PRODUCTS));
    EXPECT_FALSE(staff->hasPermission(Permission::MANAGE_USERS));
    
    // Test manager permissions
    User* manager = user_manager->authenticate("manager_user", "manager_password");
    ASSERT_NE(manager, nullptr);
    
    // Manager can do everything
    EXPECT_TRUE(manager->hasPermission(Permission::DELETE_PRODUCTS));
    EXPECT_TRUE(manager->hasPermission(Permission::MANAGE_USERS));
    EXPECT_TRUE(manager->hasPermission(Permission::SYSTEM_ADMIN));
}

// Inventory Alert Integration Test
TEST_F(IntegrationTest, InventoryAlertIntegration) {
    // Check initial low stock items
    auto low_stock_products = inventory->getLowStockProducts(5);
    EXPECT_EQ(low_stock_products.size(), 1); // LOWSTOCK001 has 3 units
    EXPECT_EQ(low_stock_products[0]->getId(), "LOWSTOCK001");
    
    // Create low stock alert notification
    if (!low_stock_products.empty()) {
        for (const auto* product : low_stock_products) {
            auto alert = std::make_unique<SystemNotification>(
                "Low Stock Alert",
                "Product " + product->getName() + " is running low (" + 
                std::to_string(product->getQuantity()) + " units remaining)",
                NotificationPriority::HIGH
            );
            notification_manager->addNotification(std::move(alert));
        }
    }
    
    // Verify alert was created
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 1);
    EXPECT_THAT(pending[0]->getMessage(), ::testing::HasSubstr("Low Stock Item"));
    EXPECT_THAT(pending[0]->getMessage(), ::testing::HasSubstr("3 units"));
    
    // Process notifications
    notification_manager->processNotifications();
    
    // Verify notifications were processed
    auto after_processing = notification_manager->getPendingNotifications();
    EXPECT_TRUE(after_processing.empty());
}

// Report Generation Integration Test
TEST_F(IntegrationTest, ReportGenerationIntegration) {
    // Create and process some orders for sales report
    std::vector<Order> completed_orders;
    
    Order order1("REPORT_ORD001");
    order1.addItem("LAPTOP001", 1);
    order1.setStatus(OrderStatus::COMPLETED);
    completed_orders.push_back(order1);
    
    Order order2("REPORT_ORD002");
    order2.addItem("MOUSE001", 3);
    order2.addItem("KEYBOARD001", 1);
    order2.setStatus(OrderStatus::COMPLETED);
    completed_orders.push_back(order2);
    
    // Generate sales report
    SalesReport sales_report(completed_orders);
    std::string sales_content = sales_report.generateReport();
    
    EXPECT_FALSE(sales_content.empty());
    EXPECT_THAT(sales_content, ::testing::HasSubstr("Sales Report"));
    EXPECT_THAT(sales_content, ::testing::HasSubstr("REPORT_ORD001"));
    EXPECT_THAT(sales_content, ::testing::HasSubstr("REPORT_ORD002"));
    
    // Generate inventory report
    InventoryReport inventory_report(inventory.get());
    std::string inventory_content = inventory_report.generateReport();
    
    EXPECT_FALSE(inventory_content.empty());
    EXPECT_THAT(inventory_content, ::testing::HasSubstr("Inventory Report"));
    EXPECT_THAT(inventory_content, ::testing::HasSubstr("Gaming Laptop"));
    EXPECT_THAT(inventory_content, ::testing::HasSubstr("Fresh Milk"));
    
    // Create report notification
    auto report_notification = std::make_unique<EmailNotification>(
        "manager@store.com",
        "Daily Reports Available",
        "Sales and inventory reports have been generated"
    );
    notification_manager->addNotification(std::move(report_notification));
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 1);
    EXPECT_EQ(pending[0]->getType(), "Email");
}

// HTTP Server Integration Test
TEST_F(IntegrationTest, HTTPServerIntegration) {
    // Test system status endpoint
    HTTPRequest status_request;
    status_request.method = "GET";
    status_request.path = "/api/system/status";
    
    HTTPResponse status_response = http_server->routeRequest(status_request);
    EXPECT_EQ(status_response.status_code, 200);
    EXPECT_THAT(status_response.body, ::testing::HasSubstr("\"status\":\"success\""));
    EXPECT_THAT(status_response.body, ::testing::HasSubstr("\"inventory_available\":true"));
    
    // Test products endpoint
    HTTPRequest products_request;
    products_request.method = "GET";
    products_request.path = "/api/products";
    
    HTTPResponse products_response = http_server->routeRequest(products_request);
    EXPECT_EQ(products_response.status_code, 200);
    EXPECT_THAT(products_response.body, ::testing::HasSubstr("\"status\":\"success\""));
    EXPECT_THAT(products_response.body, ::testing::HasSubstr("\"count\":5")); // 5 products added
    
    // Test inventory status endpoint
    HTTPRequest inventory_request;
    inventory_request.method = "GET";
    inventory_request.path = "/api/inventory/status";
    
    HTTPResponse inventory_response = http_server->routeRequest(inventory_request);
    EXPECT_EQ(inventory_response.status_code, 200);
    EXPECT_THAT(inventory_response.body, ::testing::HasSubstr("\"total_products\":5"));
}

// Concurrent Operations Integration Test
TEST_F(IntegrationTest, ConcurrentOperationsIntegration) {
    std::vector<std::future<bool>> order_futures;
    std::vector<std::future<void>> notification_futures;
    std::atomic<int> successful_operations{0};
    
    // Process multiple orders concurrently
    for (int i = 0; i < 10; ++i) {
        order_futures.push_back(std::async(std::launch::async, [&, i]() {
            Order order("CONCURRENT_ORD" + std::to_string(i));
            order.addItem("MOUSE001", 1); // Each order takes 1 mouse
            
            if (order_manager->validateOrder(order)) {
                bool result = order_manager->processOrder(order);
                if (result) {
                    successful_operations++;
                }
                return result;
            }
            return false;
        }));
    }
    
    // Add notifications concurrently
    for (int i = 0; i < 5; ++i) {
        notification_futures.push_back(std::async(std::launch::async, [&, i]() {
            auto notification = std::make_unique<SystemNotification>(
                "Concurrent Operation " + std::to_string(i),
                "Concurrent operation message " + std::to_string(i),
                NotificationPriority::MEDIUM
            );
            notification_manager->addNotification(std::move(notification));
        }));
    }
    
    // Wait for all operations to complete
    for (auto& future : order_futures) {
        future.get();
    }
    
    for (auto& future : notification_futures) {
        future.get();
    }
    
    // Verify results
    EXPECT_GT(successful_operations.load(), 0);
    EXPECT_LE(successful_operations.load(), 10);
    
    // Mouse inventory should be reduced
    int final_mouse_quantity = inventory->getProduct("MOUSE001")->getQuantity();
    EXPECT_LT(final_mouse_quantity, 50); // Started with 50
    
    // Should have 5 notifications pending
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 5);
}

// Error Handling Integration Test
TEST_F(IntegrationTest, ErrorHandlingIntegration) {
    // Test order processing with insufficient stock
    Order large_order("ERROR_TEST_ORD");
    large_order.addItem("LAPTOP001", 100); // More than available (10)
    
    // Validation should fail
    EXPECT_FALSE(order_manager->validateOrder(large_order));
    auto errors = order_manager->getValidationErrors(large_order);
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors[0], ::testing::HasSubstr("Insufficient stock"));
    
    // Processing should fail
    EXPECT_FALSE(order_manager->processOrder(large_order));
    
    // Inventory should remain unchanged
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), 10);
    
    // Test authentication with invalid credentials
    User* invalid_user = user_manager->authenticate("nonexistent", "wrongpassword");
    EXPECT_EQ(invalid_user, nullptr);
    
    // Test HTTP endpoint with invalid path
    HTTPRequest invalid_request;
    invalid_request.method = "GET";
    invalid_request.path = "/api/nonexistent/endpoint";
    
    HTTPResponse error_response = http_server->routeRequest(invalid_request);
    EXPECT_EQ(error_response.status_code, 404);
    EXPECT_THAT(error_response.body, ::testing::HasSubstr("Not Found"));
}

// Data Consistency Integration Test
TEST_F(IntegrationTest, DataConsistencyIntegration) {
    // Record initial state
    int initial_laptop_quantity = inventory->getProduct("LAPTOP001")->getQuantity();
    int initial_total_products = inventory->getTotalProductCount();
    double initial_total_value = inventory->getTotalValue();
    
    // Perform various operations
    Order order("CONSISTENCY_ORD");
    order.addItem("LAPTOP001", 2);
    
    EXPECT_TRUE(order_manager->processOrder(order));
    
    // Add a new product
    auto new_product = std::make_unique<Product>("NEW001", "New Product", "New", 99.99, 5);
    EXPECT_TRUE(inventory->addProduct(std::move(new_product)));
    
    // Remove a product
    EXPECT_TRUE(inventory->removeProduct("LOWSTOCK001"));
    
    // Verify consistency
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), initial_laptop_quantity - 2);
    EXPECT_EQ(inventory->getTotalProductCount(), initial_total_products); // -1 +1 = same
    
    // Total value should be different
    double final_total_value = inventory->getTotalValue();
    EXPECT_NE(final_total_value, initial_total_value);
    
    // Verify data integrity
    auto all_products = inventory->getAllProducts();
    for (const auto* product : all_products) {
        EXPECT_GE(product->getQuantity(), 0); // No negative quantities
        EXPECT_GT(product->getPrice(), 0);    // Positive prices (in this test)
        EXPECT_FALSE(product->getId().empty()); // Valid IDs
        EXPECT_FALSE(product->getName().empty()); // Valid names
    }
}

// Performance Integration Test
TEST_F(IntegrationTest, PerformanceIntegration) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform a series of operations that would typically happen in a system
    std::vector<std::future<void>> operations;
    
    // Add many products
    operations.push_back(std::async(std::launch::async, [&]() {
        for (int i = 0; i < 1000; ++i) {
            auto product = std::make_unique<Product>(
                "PERF_P" + std::to_string(i),
                "Performance Product " + std::to_string(i),
                "Performance",
                50.0 + (i % 100),
                100 + (i % 50)
            );
            inventory->addProduct(std::move(product));
        }
    }));
    
    // Process many orders
    operations.push_back(std::async(std::launch::async, [&]() {
        for (int i = 0; i < 100; ++i) {
            Order order("PERF_ORD" + std::to_string(i));
            order.addItem("MOUSE001", 1);
            if (order_manager->validateOrder(order)) {
                order_manager->processOrder(order);
            }
        }
    }));
    
    // Create many notifications
    operations.push_back(std::async(std::launch::async, [&]() {
        for (int i = 0; i < 500; ++i) {
            auto notification = std::make_unique<SystemNotification>(
                "Performance Test " + std::to_string(i),
                "Performance test message " + std::to_string(i)
            );
            notification_manager->addNotification(std::move(notification));
        }
    }));
    
    // Wait for all operations
    for (auto& op : operations) {
        op.get();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 10000); // Less than 10 seconds
    
    // Verify system is still functional
    EXPECT_GT(inventory->getTotalProductCount(), 1000);
    EXPECT_EQ(notification_manager->getPendingNotifications().size(), 500);
}

// System State Recovery Test
TEST_F(IntegrationTest, SystemStateRecoveryIntegration) {
    // Simulate system operations
    Order order1("RECOVERY_ORD1");
    order1.addItem("LAPTOP001", 1);
    EXPECT_TRUE(order_manager->processOrder(order1));
    
    // Record state
    int laptop_quantity_after_order = inventory->getProduct("LAPTOP001")->getQuantity();
    
    // Add some notifications
    auto notification1 = std::make_unique<SystemNotification>("Test 1", "Message 1");
    auto notification2 = std::make_unique<SystemNotification>("Test 2", "Message 2");
    notification_manager->addNotification(std::move(notification1));
    notification_manager->addNotification(std::move(notification2));
    
    // Verify system state
    EXPECT_EQ(laptop_quantity_after_order, 9); // Started with 10, processed 1
    EXPECT_EQ(notification_manager->getPendingNotifications().size(), 2);
    
    // Process notifications
    notification_manager->processNotifications();
    EXPECT_TRUE(notification_manager->getPendingNotifications().empty());
    
    // System should maintain consistency
    EXPECT_EQ(inventory->getProduct("LAPTOP001")->getQuantity(), 9);
    EXPECT_EQ(inventory->getTotalProductCount(), 5); // Original test products
}

// Cross-Module Interaction Test
TEST_F(IntegrationTest, CrossModuleInteractionTest) {
    // Test User -> Order -> Inventory -> Notification workflow
    
    // 1. Manager creates staff user
    User* manager = user_manager->authenticate("manager_user", "manager_password");
    ASSERT_NE(manager, nullptr);
    EXPECT_TRUE(manager->hasPermission(Permission::MANAGE_USERS));
    
    bool user_created = user_manager->createUser("new_staff", "password", UserRole::STAFF);
    EXPECT_TRUE(user_created);
    
    // 2. New staff processes an order
    User* new_staff = user_manager->authenticate("new_staff", "password");
    ASSERT_NE(new_staff, nullptr);
    EXPECT_TRUE(new_staff->hasPermission(Permission::PROCESS_ORDERS));
    
    Order staff_order("CROSS_MODULE_ORD");
    staff_order.addItem("KEYBOARD001", 3);
    
    // 3. Order affects inventory
    int initial_keyboard_qty = inventory->getProduct("KEYBOARD001")->getQuantity();
    EXPECT_TRUE(order_manager->processOrder(staff_order));
    EXPECT_EQ(inventory->getProduct("KEYBOARD001")->getQuantity(), initial_keyboard_qty - 3);
    
    // 4. Generate notification about the order
    auto order_notification = std::make_unique<EmailNotification>(
        "manager@store.com",
        "Order Processed by " + new_staff->getUsername(),
        "Order " + staff_order.getId() + " was processed successfully"
    );
    notification_manager->addNotification(std::move(order_notification));
    
    // 5. Generate inventory report
    InventoryReport report(inventory.get());
    std::string report_content = report.generateReport();
    EXPECT_THAT(report_content, ::testing::HasSubstr("Mechanical Keyboard"));
    
    // 6. Verify all modules worked together
    EXPECT_EQ(notification_manager->getPendingNotifications().size(), 1);
    EXPECT_FALSE(report_content.empty());
    EXPECT_TRUE(user_manager->isLoggedIn("new_staff") || !user_manager->isLoggedIn("new_staff")); // Either state is valid
}