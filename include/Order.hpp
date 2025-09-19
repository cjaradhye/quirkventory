#pragma once

#include "Product.hpp"
#include "Inventory.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <atomic>

namespace quirkventory {

/**
 * @brief Represents an item in an order
 */
struct OrderItem {
    std::string product_id;
    int quantity;
    double unit_price;
    
    OrderItem(const std::string& id, int qty, double price)
        : product_id(id), quantity(qty), unit_price(price) {}
    
    double getTotalPrice() const { return quantity * unit_price; }
};

/**
 * @brief Order status enumeration
 */
enum class OrderStatus {
    PENDING,        // Order created but not yet processed
    PROCESSING,     // Order is being processed
    CONFIRMED,      // Order confirmed and inventory reserved
    SHIPPED,        // Order has been shipped
    DELIVERED,      // Order delivered to customer
    CANCELLED,      // Order cancelled
    FAILED          // Order processing failed
};

/**
 * @brief Convert OrderStatus to string
 */
std::string orderStatusToString(OrderStatus status);

/**
 * @brief Order processing system with multithreading support
 * 
 * Handles order creation, validation, processing, and inventory updates
 * with thread-safe operations and asynchronous processing capabilities.
 */
class Order {
private:
    std::string order_id_;
    std::string customer_id_;
    std::vector<OrderItem> items_;
    OrderStatus status_;
    std::chrono::system_clock::time_point order_date_;
    std::chrono::system_clock::time_point processed_date_;
    double total_amount_;
    std::string notes_;
    
    // Threading support
    mutable std::mutex order_mutex_;
    std::atomic<bool> processing_flag_;
    
    // Processing result
    std::string error_message_;

public:
    /**
     * @brief Constructor
     * @param order_id Unique order identifier
     * @param customer_id Customer identifier
     */
    Order(const std::string& order_id, const std::string& customer_id);

    /**
     * @brief Destructor
     */
    ~Order() = default;

    // Disable copy constructor and assignment operator for thread safety
    Order(const Order&) = delete;
    Order& operator=(const Order&) = delete;

    // Enable move constructor and assignment operator
    Order(Order&&) = default;
    Order& operator=(Order&&) = default;

    // Getters
    const std::string& getOrderId() const { return order_id_; }
    const std::string& getCustomerId() const { return customer_id_; }
    OrderStatus getStatus() const;
    std::chrono::system_clock::time_point getOrderDate() const { return order_date_; }
    std::chrono::system_clock::time_point getProcessedDate() const { return processed_date_; }
    double getTotalAmount() const;
    const std::string& getNotes() const { return notes_; }
    const std::string& getErrorMessage() const { return error_message_; }
    bool isProcessing() const { return processing_flag_.load(); }

    // Setters
    void setNotes(const std::string& notes);
    void setCustomerId(const std::string& customer_id);

    /**
     * @brief Add an item to the order
     * @param product_id Product identifier
     * @param quantity Quantity to order
     * @param unit_price Price per unit
     * @return true if item added successfully
     */
    bool addItem(const std::string& product_id, int quantity, double unit_price);

    /**
     * @brief Remove an item from the order
     * @param product_id Product identifier to remove
     * @return true if item removed successfully
     */
    bool removeItem(const std::string& product_id);

    /**
     * @brief Update quantity for an existing item
     * @param product_id Product identifier
     * @param new_quantity New quantity
     * @return true if updated successfully
     */
    bool updateItemQuantity(const std::string& product_id, int new_quantity);

    /**
     * @brief Get all items in the order
     * @return Vector of order items
     */
    std::vector<OrderItem> getItems() const;

    /**
     * @brief Get specific item by product ID
     * @param product_id Product identifier
     * @return Pointer to OrderItem if found, nullptr otherwise
     */
    const OrderItem* getItem(const std::string& product_id) const;

    /**
     * @brief Validate order against inventory
     * @param inventory Reference to inventory system
     * @return Vector of validation error messages (empty if valid)
     */
    std::vector<std::string> validateOrder(const Inventory& inventory) const;

    /**
     * @brief Process order synchronously
     * @param inventory Reference to inventory system
     * @return true if processing successful
     */
    bool processOrder(Inventory& inventory);

    /**
     * @brief Process order asynchronously
     * @param inventory Reference to inventory system
     * @return Future that will contain the processing result
     */
    std::future<bool> processOrderAsync(Inventory& inventory);

    /**
     * @brief Cancel the order
     * @param reason Cancellation reason
     * @return true if cancelled successfully
     */
    bool cancelOrder(const std::string& reason = "");

    /**
     * @brief Update order status
     * @param new_status New status to set
     * @return true if status updated successfully
     */
    bool updateStatus(OrderStatus new_status);

    /**
     * @brief Calculate order total
     * @return Total amount for all items
     */
    double calculateTotal() const;

    /**
     * @brief Get order summary
     * @return Formatted string with order details
     */
    std::string getOrderSummary() const;

    /**
     * @brief Get detailed order information
     * @return Formatted string with complete order details
     */
    std::string getDetailedInfo() const;

    /**
     * @brief Check if order can be modified
     * @return true if order is in a modifiable state
     */
    bool canModify() const;

    /**
     * @brief Get processing duration
     * @return Duration in milliseconds, -1 if not processed
     */
    long long getProcessingDuration() const;

private:
    /**
     * @brief Internal processing logic
     * @param inventory Reference to inventory system
     * @return true if processing successful
     */
    bool processOrderInternal(Inventory& inventory);

    /**
     * @brief Set error message and update status to FAILED
     * @param message Error message
     */
    void setError(const std::string& message);

    /**
     * @brief Update total amount based on current items
     */
    void updateTotalAmount();
};

/**
 * @brief Order management system for handling multiple orders
 * 
 * Provides centralized order management with thread-safe operations
 * and batch processing capabilities.
 */
class OrderManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Order>> orders_;
    mutable std::mutex orders_mutex_;
    
    // Statistics
    std::atomic<int> total_orders_processed_;
    std::atomic<int> successful_orders_;
    std::atomic<int> failed_orders_;

public:
    /**
     * @brief Constructor
     */
    OrderManager();

    /**
     * @brief Destructor
     */
    ~OrderManager() = default;

    // Disable copy constructor and assignment operator
    OrderManager(const OrderManager&) = delete;
    OrderManager& operator=(const OrderManager&) = delete;

    /**
     * @brief Create a new order
     * @param order_id Unique order identifier
     * @param customer_id Customer identifier
     * @return Pointer to created order, nullptr if order_id already exists
     */
    Order* createOrder(const std::string& order_id, const std::string& customer_id);

    /**
     * @brief Get an order by ID
     * @param order_id Order identifier
     * @return Pointer to order, nullptr if not found
     */
    Order* getOrder(const std::string& order_id);

    /**
     * @brief Get all orders
     * @return Vector of pointers to all orders
     */
    std::vector<Order*> getAllOrders() const;

    /**
     * @brief Get orders by status
     * @param status Order status to filter by
     * @return Vector of orders with the specified status
     */
    std::vector<Order*> getOrdersByStatus(OrderStatus status) const;

    /**
     * @brief Get orders by customer
     * @param customer_id Customer identifier
     * @return Vector of orders for the specified customer
     */
    std::vector<Order*> getOrdersByCustomer(const std::string& customer_id) const;

    /**
     * @brief Process all pending orders
     * @param inventory Reference to inventory system
     * @param max_concurrent Maximum number of concurrent processing threads
     * @return Number of successfully processed orders
     */
    int processAllPendingOrders(Inventory& inventory, int max_concurrent = 4);

    /**
     * @brief Remove an order
     * @param order_id Order identifier
     * @return true if removed successfully
     */
    bool removeOrder(const std::string& order_id);

    /**
     * @brief Get order statistics
     * @return Formatted string with statistics
     */
    std::string getStatistics() const;

    /**
     * @brief Get total number of orders
     * @return Number of orders
     */
    size_t getTotalOrderCount() const;

    /**
     * @brief Clear all completed orders
     * @return Number of orders cleared
     */
    int clearCompletedOrders();

private:
    /**
     * @brief Update statistics after order processing
     * @param success Whether the order was processed successfully
     */
    void updateStatistics(bool success);
};

} // namespace quirkventory