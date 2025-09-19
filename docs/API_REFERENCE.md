# Quirkventory API Reference

This document provides a comprehensive reference for the Quirkventory C++ inventory management system API.

## Table of Contents
1. [Core Classes](#core-classes)
2. [Product Management](#product-management)
3. [Inventory System](#inventory-system)
4. [Order Processing](#order-processing)
5. [User Management](#user-management)
6. [Notification System](#notification-system)
7. [REST API](#rest-api)
8. [Examples](#examples)

## Core Classes

### Product Class Hierarchy

```cpp
class Product {
public:
    Product(const std::string& id, const std::string& name, 
            const std::string& category, double price, int quantity);
    
    // Virtual methods for polymorphism
    virtual std::string getInfo() const;
    virtual bool isExpired() const;
    virtual std::string getExpiryInfo() const;
    virtual std::unique_ptr<Product> clone() const;
    
    // Getters (const methods)
    const std::string& getId() const;
    const std::string& getName() const;
    const std::string& getCategory() const;
    double getPrice() const;
    int getQuantity() const;
    
    // Setters
    void setPrice(double price);
    void setQuantity(int quantity);
    void addQuantity(int amount);
    bool removeQuantity(int amount);
};
```

```cpp
class PerishableProduct : public Product {
public:
    PerishableProduct(const std::string& id, const std::string& name,
                     const std::string& category, double price, int quantity,
                     const std::chrono::system_clock::time_point& expiry_date);
    
    // Override virtual methods
    std::string getInfo() const override;
    bool isExpired() const override;
    std::string getExpiryInfo() const override;
    std::unique_ptr<Product> clone() const override;
    
    // Specific to perishable products
    std::chrono::system_clock::time_point getExpiryDate() const;
    int getDaysUntilExpiry() const;
};
```

## Product Management

### Creating Products

```cpp
// Basic product
auto laptop = std::make_unique<Product>("P001", "Laptop", "Electronics", 999.99, 10);

// Perishable product
auto expiry = std::chrono::system_clock::now() + std::chrono::hours(72);
auto milk = std::make_unique<PerishableProduct>("P002", "Milk", "Dairy", 4.99, 20, expiry);
```

### Product Operations

```cpp
// Check product status
if (product->isExpired()) {
    std::cout << "Product expired: " << product->getExpiryInfo() << std::endl;
}

// Update quantities
product->setQuantity(50);
product->addQuantity(10);    // Add 10 units
product->removeQuantity(5);  // Remove 5 units (returns false if insufficient)

// Clone products
auto copy = product->clone();
```

## Inventory System

### Inventory Class

```cpp
class Inventory {
public:
    // Product management (thread-safe)
    bool addProduct(std::unique_ptr<Product> product);
    bool removeProduct(const std::string& product_id);
    bool updateQuantity(const std::string& product_id, int new_quantity);
    const Product* getProduct(const std::string& product_id) const;
    
    // Search and retrieval
    std::vector<const Product*> getAllProducts() const;
    std::vector<const Product*> searchProducts(const std::string& query) const;
    std::vector<const Product*> getProductsByCategory(const std::string& category) const;
    
    // Stock alerts
    std::vector<const Product*> getLowStockProducts(int threshold = 10) const;
    std::vector<const Product*> getExpiredProducts() const;
    
    // Statistics
    int getTotalProductCount() const;
    int getTotalQuantity() const;
    double getTotalValue() const;
};
```

### Inventory Operations

```cpp
Inventory inventory;

// Add products
auto product = std::make_unique<Product>("P001", "Widget", "Hardware", 29.99, 100);
inventory.addProduct(std::move(product));

// Search products
auto results = inventory.searchProducts("Widget");
auto electronics = inventory.getProductsByCategory("Electronics");

// Check stock levels
auto low_stock = inventory.getLowStockProducts(5);  // Products with ≤5 units
auto expired = inventory.getExpiredProducts();

// Get statistics
int total_products = inventory.getTotalProductCount();
double total_value = inventory.getTotalValue();
```

## Order Processing

### Order Classes

```cpp
enum class OrderStatus {
    PENDING,
    PROCESSING,
    COMPLETED,
    CANCELLED
};

class Order {
public:
    Order(const std::string& id);
    
    // Order item management
    void addItem(const std::string& product_id, int quantity);
    void removeItem(const std::string& product_id);
    void updateItemQuantity(const std::string& product_id, int quantity);
    
    // Order information
    const std::string& getId() const;
    OrderStatus getStatus() const;
    void setStatus(OrderStatus status);
    const std::vector<std::pair<std::string, int>>& getItems() const;
    double calculateTotal(const Inventory& inventory) const;
};
```

```cpp
class OrderManager {
public:
    OrderManager(Inventory* inventory);
    
    // Synchronous processing
    bool processOrder(const Order& order);
    
    // Asynchronous processing
    std::future<bool> processOrderAsync(const Order& order);
    
    // Order validation
    bool validateOrder(const Order& order) const;
    std::vector<std::string> getValidationErrors(const Order& order) const;
};
```

### Order Processing Examples

```cpp
Inventory inventory;
OrderManager order_manager(&inventory);

// Create order
Order order("ORD001");
order.addItem("P001", 2);
order.addItem("P002", 1);

// Validate order
if (order_manager.validateOrder(order)) {
    // Process synchronously
    bool success = order_manager.processOrder(order);
    
    // Or process asynchronously
    auto future = order_manager.processOrderAsync(order);
    bool async_success = future.get();
}

// Check for validation errors
auto errors = order_manager.getValidationErrors(order);
for (const auto& error : errors) {
    std::cout << "Validation error: " << error << std::endl;
}
```

## User Management

### User Classes

```cpp
enum class UserRole {
    STAFF,
    MANAGER
};

enum class Permission {
    VIEW_INVENTORY,
    ADD_PRODUCTS,
    EDIT_PRODUCTS,
    DELETE_PRODUCTS,
    PROCESS_ORDERS,
    VIEW_REPORTS,
    MANAGE_USERS,
    SYSTEM_ADMIN
};

class User {
public:
    User(const std::string& username, const std::string& password, UserRole role);
    
    // Virtual methods for polymorphism
    virtual bool hasPermission(Permission permission) const = 0;
    virtual std::vector<Permission> getAllPermissions() const = 0;
    
    // User information
    const std::string& getUsername() const;
    UserRole getRole() const;
    bool verifyPassword(const std::string& password) const;
};
```

```cpp
class Staff : public User {
public:
    Staff(const std::string& username, const std::string& password);
    bool hasPermission(Permission permission) const override;
    std::vector<Permission> getAllPermissions() const override;
};

class Manager : public User {
public:
    Manager(const std::string& username, const std::string& password);
    bool hasPermission(Permission permission) const override;
    std::vector<Permission> getAllPermissions() const override;
};
```

### User Management Operations

```cpp
UserManager user_manager;

// Create users
user_manager.createUser("john_doe", "secure123", UserRole::STAFF);
user_manager.createUser("jane_admin", "admin456", UserRole::MANAGER);

// Authenticate users
User* user = user_manager.authenticate("john_doe", "secure123");
if (user) {
    // Check permissions
    if (user->hasPermission(Permission::ADD_PRODUCTS)) {
        // User can add products
    }
    
    // Get all permissions
    auto permissions = user->getAllPermissions();
}

// User session management
user_manager.createSession(user);
bool is_logged_in = user_manager.isLoggedIn("john_doe");
user_manager.logout("john_doe");
```

## Notification System

### Notification Classes

```cpp
enum class NotificationPriority {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

class Notification {
public:
    virtual void send() = 0;  // Pure virtual method
    virtual std::string getType() const = 0;
    
    const std::string& getTitle() const;
    const std::string& getMessage() const;
    NotificationPriority getPriority() const;
};

class EmailNotification : public Notification {
public:
    EmailNotification(const std::string& recipient, 
                     const std::string& title,
                     const std::string& message);
    void send() override;
    std::string getType() const override;
};

class SystemNotification : public Notification {
public:
    SystemNotification(const std::string& title,
                      const std::string& message,
                      NotificationPriority priority = NotificationPriority::MEDIUM);
    void send() override;
    std::string getType() const override;
};
```

### Report Classes

```cpp
class Report {
public:
    virtual std::string generateReport() = 0;  // Pure virtual
    virtual std::string getReportType() const = 0;
    
    void setDateRange(const std::chrono::system_clock::time_point& start,
                     const std::chrono::system_clock::time_point& end);
};

class SalesReport : public Report {
public:
    SalesReport(const std::vector<Order>& orders = {});
    std::string generateReport() override;
    std::string getReportType() const override;
};

class InventoryReport : public Report {
public:
    InventoryReport(const Inventory* inventory = nullptr);
    std::string generateReport() override;
    std::string getReportType() const override;
};
```

### Notification System Usage

```cpp
NotificationManager notification_manager;

// Create notifications
auto email_alert = std::make_unique<EmailNotification>(
    "manager@store.com",
    "Low Stock Alert",
    "Product P001 is running low (5 units remaining)"
);

auto system_alert = std::make_unique<SystemNotification>(
    "Order Processed",
    "Order ORD001 completed successfully",
    NotificationPriority::HIGH
);

// Add to notification queue
notification_manager.addNotification(std::move(email_alert));
notification_manager.addNotification(std::move(system_alert));

// Process all pending notifications
notification_manager.processNotifications();

// Generate reports
SalesReport sales_report;
InventoryReport inventory_report;

std::string sales_data = sales_report.generateReport();
std::string inventory_data = inventory_report.generateReport();
```

## REST API

### HTTP Server

```cpp
class HTTPServer {
public:
    HTTPServer(const std::string& host = "localhost", int port = 8080);
    
    // Server lifecycle
    bool start();
    void stop();
    std::string getServerUrl() const;
    
    // System integration
    void setSystemComponents(Inventory* inventory,
                           OrderManager* order_manager,
                           UserManager* user_manager,
                           NotificationManager* notification_manager);
    
    // Request handling
    HTTPResponse handleRequest(const std::string& request_data);
};
```

### API Endpoints

#### Product Endpoints
- `GET /api/products` - Get all products
- `GET /api/products/{id}` - Get specific product
- `POST /api/products` - Create new product
- `PUT /api/products/{id}` - Update product
- `DELETE /api/products/{id}` - Delete product

#### Inventory Endpoints
- `GET /api/inventory/status` - Get inventory statistics
- `GET /api/inventory/alerts/low-stock` - Get low stock alerts
- `GET /api/inventory/alerts/expiry` - Get expiry alerts

#### Order Endpoints
- `GET /api/orders` - Get all orders
- `GET /api/orders/{id}` - Get specific order
- `POST /api/orders` - Create new order

#### Report Endpoints
- `GET /api/reports/sales` - Generate sales report
- `GET /api/reports/inventory` - Generate inventory report

#### System Endpoints
- `GET /api/system/status` - Get system status

### API Usage Example

```cpp
HTTPServer server("localhost", 8080);
server.setSystemComponents(&inventory, &order_manager, &user_manager, &notification_manager);

if (server.start()) {
    std::cout << "Server running on " << server.getServerUrl() << std::endl;
    
    // Server handles requests automatically
    // Example request simulation:
    std::string request = "GET /api/products HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HTTPResponse response = server.handleRequest(request);
    
    server.stop();
}
```

## Examples

### Complete System Setup

```cpp
#include "Inventory.hpp"
#include "Order.hpp"
#include "User.hpp"
#include "NotificationSystem.hpp"
#include "HTTPServer.hpp"

int main() {
    // Initialize system components
    Inventory inventory;
    OrderManager order_manager(&inventory);
    UserManager user_manager;
    NotificationManager notification_manager;
    HTTPServer server;
    
    // Configure system
    server.setSystemComponents(&inventory, &order_manager, 
                              &user_manager, &notification_manager);
    
    // Add sample products
    auto laptop = std::make_unique<Product>("LAPTOP001", "Gaming Laptop", 
                                           "Electronics", 1299.99, 15);
    auto mouse = std::make_unique<Product>("MOUSE001", "Wireless Mouse", 
                                         "Electronics", 49.99, 100);
    
    inventory.addProduct(std::move(laptop));
    inventory.addProduct(std::move(mouse));
    
    // Create users
    user_manager.createUser("staff", "password", UserRole::STAFF);
    user_manager.createUser("admin", "admin123", UserRole::MANAGER);
    
    // Process sample order
    Order order("ORD001");
    order.addItem("LAPTOP001", 1);
    order.addItem("MOUSE001", 2);
    
    if (order_manager.validateOrder(order)) {
        auto future = order_manager.processOrderAsync(order);
        bool success = future.get();
        
        if (success) {
            // Send notification
            auto notification = std::make_unique<SystemNotification>(
                "Order Processed",
                "Order " + order.getId() + " completed successfully",
                NotificationPriority::HIGH
            );
            notification_manager.addNotification(std::move(notification));
        }
    }
    
    // Start HTTP server
    server.start();
    
    // System is now ready for use
    std::cout << "Quirkventory system initialized successfully!" << std::endl;
    
    return 0;
}
```

### Error Handling Patterns

```cpp
// Safe product retrieval
const Product* product = inventory.getProduct("P001");
if (!product) {
    std::cerr << "Product not found" << std::endl;
    return;
}

// Order validation
if (!order_manager.validateOrder(order)) {
    auto errors = order_manager.getValidationErrors(order);
    for (const auto& error : errors) {
        std::cerr << "Order error: " << error << std::endl;
    }
    return;
}

// User authentication
User* user = user_manager.authenticate(username, password);
if (!user) {
    std::cerr << "Invalid credentials" << std::endl;
    return;
}

// Permission checking
if (!user->hasPermission(Permission::ADD_PRODUCTS)) {
    std::cerr << "Insufficient permissions" << std::endl;
    return;
}
```

### Thread Safety

All major components are designed to be thread-safe:

```cpp
// Concurrent inventory operations
std::vector<std::thread> threads;

for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&inventory, i]() {
        auto product = std::make_unique<Product>(
            "P" + std::to_string(i),
            "Product " + std::to_string(i),
            "Category", 99.99, 100
        );
        inventory.addProduct(std::move(product));
    });
}

for (auto& thread : threads) {
    thread.join();
}

// Concurrent order processing
std::vector<std::future<bool>> futures;

for (int i = 0; i < 5; ++i) {
    Order order("ORD" + std::to_string(i));
    order.addItem("P0", 10);
    futures.push_back(order_manager.processOrderAsync(order));
}

for (auto& future : futures) {
    bool success = future.get();
    // Handle result
}
```

This API reference demonstrates the comprehensive object-oriented design of the Quirkventory system, showcasing inheritance, polymorphism, encapsulation, and modern C++ practices.