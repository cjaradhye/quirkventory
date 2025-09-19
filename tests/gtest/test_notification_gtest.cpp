#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include "../../include/NotificationSystem.hpp"

using namespace quirkventory;

// Test Fixture for Notification System Tests
class NotificationSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        notification_manager = std::make_unique<NotificationManager>();
    }
    
    void TearDown() override {
        notification_manager.reset();
    }
    
    std::unique_ptr<NotificationManager> notification_manager;
};

// Email Notification Tests
TEST_F(NotificationSystemTest, CreateEmailNotification) {
    auto email_notification = std::make_unique<EmailNotification>(
        "test@example.com",
        "Test Email",
        "This is a test email notification"
    );
    
    EXPECT_EQ(email_notification->getTitle(), "Test Email");
    EXPECT_EQ(email_notification->getMessage(), "This is a test email notification");
    EXPECT_EQ(email_notification->getType(), "Email");
    EXPECT_EQ(email_notification->getPriority(), NotificationPriority::MEDIUM);
}

TEST_F(NotificationSystemTest, EmailNotificationSend) {
    auto email_notification = std::make_unique<EmailNotification>(
        "recipient@example.com",
        "Important Alert",
        "System maintenance scheduled"
    );
    
    // Calling send should not throw (it's simulated)
    EXPECT_NO_THROW(email_notification->send());
}

// System Notification Tests
TEST_F(NotificationSystemTest, CreateSystemNotification) {
    auto system_notification = std::make_unique<SystemNotification>(
        "System Alert",
        "Low disk space detected",
        NotificationPriority::HIGH
    );
    
    EXPECT_EQ(system_notification->getTitle(), "System Alert");
    EXPECT_EQ(system_notification->getMessage(), "Low disk space detected");
    EXPECT_EQ(system_notification->getType(), "System");
    EXPECT_EQ(system_notification->getPriority(), NotificationPriority::HIGH);
}

TEST_F(NotificationSystemTest, SystemNotificationDefaultPriority) {
    auto system_notification = std::make_unique<SystemNotification>(
        "Default Priority",
        "This should have medium priority"
    );
    
    EXPECT_EQ(system_notification->getPriority(), NotificationPriority::MEDIUM);
}

TEST_F(NotificationSystemTest, SystemNotificationSend) {
    auto system_notification = std::make_unique<SystemNotification>(
        "Test Alert",
        "Test message",
        NotificationPriority::CRITICAL
    );
    
    EXPECT_NO_THROW(system_notification->send());
}

// Notification Manager Tests
TEST_F(NotificationSystemTest, AddNotificationToManager) {
    auto notification = std::make_unique<SystemNotification>(
        "Test",
        "Test message"
    );
    
    notification_manager->addNotification(std::move(notification));
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 1);
    EXPECT_EQ(pending[0]->getTitle(), "Test");
}

TEST_F(NotificationSystemTest, AddMultipleNotifications) {
    auto email_notif = std::make_unique<EmailNotification>(
        "user@example.com", "Email Test", "Email message"
    );
    auto system_notif = std::make_unique<SystemNotification>(
        "System Test", "System message"
    );
    
    notification_manager->addNotification(std::move(email_notif));
    notification_manager->addNotification(std::move(system_notif));
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 2);
}

TEST_F(NotificationSystemTest, ProcessNotifications) {
    auto notification1 = std::make_unique<SystemNotification>("Test 1", "Message 1");
    auto notification2 = std::make_unique<EmailNotification>("test@example.com", "Test 2", "Message 2");
    
    notification_manager->addNotification(std::move(notification1));
    notification_manager->addNotification(std::move(notification2));
    
    // Should have 2 pending before processing
    auto pending_before = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending_before.size(), 2);
    
    // Process notifications
    EXPECT_NO_THROW(notification_manager->processNotifications());
    
    // Should have 0 pending after processing
    auto pending_after = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending_after.size(), 0);
}

TEST_F(NotificationSystemTest, ProcessEmptyNotificationQueue) {
    // Processing empty queue should not crash
    EXPECT_NO_THROW(notification_manager->processNotifications());
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_TRUE(pending.empty());
}

// Notification Priority Tests
class NotificationPriorityTest : public ::testing::Test {
protected:
    void SetUp() override {
        notification_manager = std::make_unique<NotificationManager>();
    }
    
    std::unique_ptr<NotificationManager> notification_manager;
};

TEST_F(NotificationPriorityTest, DifferentPriorityLevels) {
    auto low_priority = std::make_unique<SystemNotification>(
        "Low Priority", "Low priority message", NotificationPriority::LOW
    );
    auto medium_priority = std::make_unique<SystemNotification>(
        "Medium Priority", "Medium priority message", NotificationPriority::MEDIUM
    );
    auto high_priority = std::make_unique<SystemNotification>(
        "High Priority", "High priority message", NotificationPriority::HIGH
    );
    auto critical_priority = std::make_unique<SystemNotification>(
        "Critical Priority", "Critical priority message", NotificationPriority::CRITICAL
    );
    
    EXPECT_EQ(low_priority->getPriority(), NotificationPriority::LOW);
    EXPECT_EQ(medium_priority->getPriority(), NotificationPriority::MEDIUM);
    EXPECT_EQ(high_priority->getPriority(), NotificationPriority::HIGH);
    EXPECT_EQ(critical_priority->getPriority(), NotificationPriority::CRITICAL);
}

// Sales Report Tests
class SalesReportTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create sample orders for testing
        order1.setStatus(OrderStatus::COMPLETED);
        order1.addItem("LAPTOP001", 2);
        
        order2.setStatus(OrderStatus::COMPLETED);
        order2.addItem("MOUSE001", 5);
        order2.addItem("KEYBOARD001", 1);
        
        orders = {order1, order2};
    }
    
    Order order1{"ORD001"};
    Order order2{"ORD002"};
    std::vector<Order> orders;
};

TEST_F(SalesReportTest, GenerateBasicSalesReport) {
    SalesReport sales_report(orders);
    
    std::string report = sales_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_EQ(sales_report.getReportType(), "Sales Report");
    
    // Report should contain order information
    EXPECT_THAT(report, ::testing::HasSubstr("Sales Report"));
    EXPECT_THAT(report, ::testing::HasSubstr("Orders"));
}

TEST_F(SalesReportTest, GenerateEmptySalesReport) {
    SalesReport empty_report({});
    
    std::string report = empty_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_THAT(report, ::testing::HasSubstr("No orders"));
}

TEST_F(SalesReportTest, SalesReportWithDateRange) {
    SalesReport sales_report(orders);
    
    auto start_date = std::chrono::system_clock::now() - std::chrono::hours(24);
    auto end_date = std::chrono::system_clock::now();
    
    sales_report.setDateRange(start_date, end_date);
    std::string report = sales_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_THAT(report, ::testing::HasSubstr("Date Range"));
}

// Inventory Report Tests
class InventoryReportTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory = std::make_unique<Inventory>();
        
        // Add test products
        auto laptop = std::make_unique<Product>("LAPTOP001", "Gaming Laptop", "Electronics", 1299.99, 15);
        auto mouse = std::make_unique<Product>("MOUSE001", "Wireless Mouse", "Electronics", 49.99, 100);
        auto low_stock = std::make_unique<Product>("LOW001", "Low Stock Item", "Test", 19.99, 2);
        
        inventory->addProduct(std::move(laptop));
        inventory->addProduct(std::move(mouse));
        inventory->addProduct(std::move(low_stock));
    }
    
    std::unique_ptr<Inventory> inventory;
};

TEST_F(InventoryReportTest, GenerateBasicInventoryReport) {
    InventoryReport inventory_report(inventory.get());
    
    std::string report = inventory_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_EQ(inventory_report.getReportType(), "Inventory Report");
    
    // Report should contain inventory information
    EXPECT_THAT(report, ::testing::HasSubstr("Inventory Report"));
    EXPECT_THAT(report, ::testing::HasSubstr("Total Products"));
    EXPECT_THAT(report, ::testing::HasSubstr("Gaming Laptop"));
    EXPECT_THAT(report, ::testing::HasSubstr("Wireless Mouse"));
}

TEST_F(InventoryReportTest, GenerateEmptyInventoryReport) {
    Inventory empty_inventory;
    InventoryReport empty_report(&empty_inventory);
    
    std::string report = empty_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_THAT(report, ::testing::HasSubstr("No products"));
}

TEST_F(InventoryReportTest, InventoryReportWithNullInventory) {
    InventoryReport null_report(nullptr);
    
    std::string report = null_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_THAT(report, ::testing::HasSubstr("No inventory"));
}

TEST_F(InventoryReportTest, InventoryReportWithDateRange) {
    InventoryReport inventory_report(inventory.get());
    
    auto start_date = std::chrono::system_clock::now() - std::chrono::hours(24);
    auto end_date = std::chrono::system_clock::now();
    
    inventory_report.setDateRange(start_date, end_date);
    std::string report = inventory_report.generateReport();
    
    EXPECT_FALSE(report.empty());
    EXPECT_THAT(report, ::testing::HasSubstr("Date Range"));
}

// Polymorphism Tests
class NotificationPolymorphismTest : public ::testing::Test {
protected:
    void SetUp() override {
        notifications.push_back(std::make_unique<EmailNotification>(
            "poly@example.com", "Polymorphic Email", "Test polymorphism"
        ));
        notifications.push_back(std::make_unique<SystemNotification>(
            "Polymorphic System", "Test polymorphism", NotificationPriority::HIGH
        ));
    }
    
    std::vector<std::unique_ptr<Notification>> notifications;
};

TEST_F(NotificationPolymorphismTest, PolymorphicNotificationBehavior) {
    for (const auto& notification : notifications) {
        // Test virtual method calls
        EXPECT_FALSE(notification->getTitle().empty());
        EXPECT_FALSE(notification->getMessage().empty());
        EXPECT_FALSE(notification->getType().empty());
        
        // Test that send() method can be called polymorphically
        EXPECT_NO_THROW(notification->send());
        
        // Test different types return different type strings
        std::string type = notification->getType();
        EXPECT_TRUE(type == "Email" || type == "System");
    }
}

class ReportPolymorphismTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data
        inventory = std::make_unique<Inventory>();
        auto product = std::make_unique<Product>("P001", "Test Product", "Test", 100.0, 10);
        inventory->addProduct(std::move(product));
        
        Order order("ORD001");
        order.addItem("P001", 1);
        orders.push_back(order);
        
        // Create reports
        reports.push_back(std::make_unique<SalesReport>(orders));
        reports.push_back(std::make_unique<InventoryReport>(inventory.get()));
    }
    
    std::unique_ptr<Inventory> inventory;
    std::vector<Order> orders;
    std::vector<std::unique_ptr<Report>> reports;
};

TEST_F(ReportPolymorphismTest, PolymorphicReportBehavior) {
    for (const auto& report : reports) {
        // Test virtual method calls
        std::string generated_report = report->generateReport();
        std::string report_type = report->getReportType();
        
        EXPECT_FALSE(generated_report.empty());
        EXPECT_FALSE(report_type.empty());
        
        // Test different types return different type strings
        EXPECT_TRUE(report_type == "Sales Report" || report_type == "Inventory Report");
        
        // Test date range setting (should not crash)
        auto start_date = std::chrono::system_clock::now() - std::chrono::hours(24);
        auto end_date = std::chrono::system_clock::now();
        
        EXPECT_NO_THROW(report->setDateRange(start_date, end_date));
    }
}

// Performance Tests
class NotificationPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        notification_manager = std::make_unique<NotificationManager>();
    }
    
    std::unique_ptr<NotificationManager> notification_manager;
};

TEST_F(NotificationPerformanceTest, LargeScaleNotificationProcessing) {
    const int num_notifications = 10000;
    
    // Add many notifications
    for (int i = 0; i < num_notifications; ++i) {
        if (i % 2 == 0) {
            notification_manager->addNotification(std::make_unique<SystemNotification>(
                "System " + std::to_string(i),
                "Message " + std::to_string(i)
            ));
        } else {
            notification_manager->addNotification(std::make_unique<EmailNotification>(
                "user" + std::to_string(i) + "@example.com",
                "Email " + std::to_string(i),
                "Message " + std::to_string(i)
            ));
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Process all notifications
    notification_manager->processNotifications();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should process within reasonable time
    EXPECT_LT(duration.count(), 5000); // Less than 5 seconds
    
    // All notifications should be processed
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_TRUE(pending.empty());
}

// Thread Safety Tests
class NotificationConcurrencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        notification_manager = std::make_unique<NotificationManager>();
    }
    
    std::unique_ptr<NotificationManager> notification_manager;
};

TEST_F(NotificationConcurrencyTest, ConcurrentNotificationAddition) {
    std::vector<std::thread> threads;
    std::atomic<int> notifications_added{0};
    
    // Add notifications concurrently
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([&, i]() {
            auto notification = std::make_unique<SystemNotification>(
                "Concurrent " + std::to_string(i),
                "Concurrent message " + std::to_string(i)
            );
            notification_manager->addNotification(std::move(notification));
            notifications_added++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(notifications_added.load(), 100);
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_EQ(pending.size(), 100);
}

TEST_F(NotificationConcurrencyTest, ConcurrentProcessing) {
    // Add some notifications first
    for (int i = 0; i < 50; ++i) {
        notification_manager->addNotification(std::make_unique<SystemNotification>(
            "Test " + std::to_string(i),
            "Message " + std::to_string(i)
        ));
    }
    
    std::vector<std::thread> threads;
    
    // Process notifications concurrently (should be thread-safe)
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            notification_manager->processNotifications();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All notifications should be processed (no duplicates or missed notifications)
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_TRUE(pending.empty());
}

// Edge Cases and Error Conditions
TEST_F(NotificationSystemTest, EmptyNotificationContent) {
    auto empty_notification = std::make_unique<SystemNotification>("", "");
    
    EXPECT_EQ(empty_notification->getTitle(), "");
    EXPECT_EQ(empty_notification->getMessage(), "");
    EXPECT_NO_THROW(empty_notification->send());
}

TEST_F(NotificationSystemTest, VeryLongNotificationContent) {
    std::string long_title(10000, 'T');
    std::string long_message(50000, 'M');
    
    auto long_notification = std::make_unique<SystemNotification>(long_title, long_message);
    
    EXPECT_EQ(long_notification->getTitle().length(), 10000);
    EXPECT_EQ(long_notification->getMessage().length(), 50000);
    EXPECT_NO_THROW(long_notification->send());
}

TEST_F(NotificationSystemTest, EmailWithInvalidAddress) {
    auto invalid_email = std::make_unique<EmailNotification>(
        "invalid-email-address",
        "Test",
        "Test message"
    );
    
    // Should handle gracefully (validation would be in real implementation)
    EXPECT_NO_THROW(invalid_email->send());
}

TEST_F(NotificationSystemTest, AddNullNotification) {
    std::unique_ptr<Notification> null_notification;
    
    // Adding null notification should be handled gracefully
    EXPECT_NO_THROW(notification_manager->addNotification(std::move(null_notification)));
    
    auto pending = notification_manager->getPendingNotifications();
    EXPECT_TRUE(pending.empty());
}