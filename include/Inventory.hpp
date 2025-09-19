#pragma once

#include "Product.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <functional>

namespace quirkventory {

// Forward declarations
class Notification;

/**
 * @brief Thread-safe inventory management system
 * 
 * Manages product storage using STL containers with thread-safe operations.
 * Provides functionality for adding/removing products, stock monitoring,
 * and automated alert generation.
 */
class Inventory {
private:
    // STL containers for product storage
    std::unordered_map<std::string, std::unique_ptr<Product>> products_;
    
    // Thread safety
    mutable std::mutex inventory_mutex_;
    
    // Configuration
    int default_low_stock_threshold_;
    std::unordered_map<std::string, int> category_thresholds_;
    
    // Notification system
    std::vector<std::function<void(const std::string&)>> alert_callbacks_;

public:
    /**
     * @brief Constructor
     * @param default_threshold Default low stock threshold for all products
     */
    explicit Inventory(int default_threshold = 10);

    /**
     * @brief Destructor
     */
    ~Inventory() = default;

    // Disable copy constructor and assignment operator for thread safety
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    // Enable move constructor and assignment operator
    Inventory(Inventory&&) = default;
    Inventory& operator=(Inventory&&) = default;

    /**
     * @brief Add a product to the inventory
     * @param product Unique pointer to the product
     * @return true if added successfully, false if product ID already exists
     */
    bool addProduct(std::unique_ptr<Product> product);

    /**
     * @brief Remove a product from the inventory
     * @param product_id ID of the product to remove
     * @return true if removed successfully, false if product not found
     */
    bool removeProduct(const std::string& product_id);

    /**
     * @brief Update product quantity
     * @param product_id ID of the product
     * @param new_quantity New quantity value
     * @return true if updated successfully, false if product not found
     */
    bool updateQuantity(const std::string& product_id, int new_quantity);

    /**
     * @brief Add quantity to existing product
     * @param product_id ID of the product
     * @param amount Amount to add
     * @return true if updated successfully, false if product not found
     */
    bool addQuantity(const std::string& product_id, int amount);

    /**
     * @brief Remove quantity from existing product
     * @param product_id ID of the product
     * @param amount Amount to remove
     * @return true if updated successfully, false if product not found or insufficient quantity
     */
    bool removeQuantity(const std::string& product_id, int amount);

    /**
     * @brief Get a product by ID
     * @param product_id ID of the product
     * @return Const pointer to product, nullptr if not found
     */
    const Product* getProduct(const std::string& product_id) const;

    /**
     * @brief Get all products in inventory
     * @return Vector of const pointers to all products
     */
    std::vector<const Product*> getAllProducts() const;

    /**
     * @brief Search products by name (case-insensitive)
     * @param name_pattern Name pattern to search for
     * @return Vector of matching products
     */
    std::vector<const Product*> searchByName(const std::string& name_pattern) const;

    /**
     * @brief Get products by category
     * @param category Category to filter by
     * @return Vector of products in the specified category
     */
    std::vector<const Product*> getProductsByCategory(const std::string& category) const;

    /**
     * @brief Check for low stock products
     * @return Vector of products that are low in stock
     */
    std::vector<const Product*> getLowStockProducts() const;

    /**
     * @brief Check for expired products
     * @return Vector of expired products
     */
    std::vector<const Product*> getExpiredProducts() const;

    /**
     * @brief Check for products expiring soon
     * @param days Number of days to check ahead (default: 7)
     * @return Vector of products expiring within specified days
     */
    std::vector<const Product*> getExpiringSoonProducts(int days = 7) const;

    /**
     * @brief Get total number of products
     * @return Number of unique products in inventory
     */
    size_t getTotalProductCount() const;

    /**
     * @brief Get total quantity of all products
     * @return Sum of quantities of all products
     */
    int getTotalQuantity() const;

    /**
     * @brief Get total value of inventory
     * @return Total monetary value of all products
     */
    double getTotalValue() const;

    /**
     * @brief Get inventory statistics by category
     * @return Map of category name to total value
     */
    std::unordered_map<std::string, double> getValueByCategory() const;

    /**
     * @brief Set low stock threshold for a specific category
     * @param category Category name
     * @param threshold Threshold value
     */
    void setCategoryThreshold(const std::string& category, int threshold);

    /**
     * @brief Get low stock threshold for a product
     * @param product_id Product ID
     * @return Threshold value
     */
    int getThreshold(const std::string& product_id) const;

    /**
     * @brief Register an alert callback function
     * @param callback Function to call when alerts are generated
     */
    void registerAlertCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Generate and send low stock alerts
     */
    void checkAndSendLowStockAlerts();

    /**
     * @brief Generate and send expiry alerts
     */
    void checkAndSendExpiryAlerts();

    /**
     * @brief Generate inventory report
     * @return Formatted string with inventory summary
     */
    std::string generateInventoryReport() const;

    /**
     * @brief Generate low stock report
     * @return Formatted string with low stock items
     */
    std::string generateLowStockReport() const;

    /**
     * @brief Generate expiry report
     * @return Formatted string with expired and expiring items
     */
    std::string generateExpiryReport() const;

    /**
     * @brief Check if product exists in inventory
     * @param product_id Product ID to check
     * @return true if product exists
     */
    bool hasProduct(const std::string& product_id) const;

    /**
     * @brief Get available quantity for a product
     * @param product_id Product ID
     * @return Available quantity, -1 if product not found
     */
    int getAvailableQuantity(const std::string& product_id) const;

    /**
     * @brief Validate inventory consistency
     * @return Vector of error messages, empty if no issues
     */
    std::vector<std::string> validateInventory() const;

private:
    /**
     * @brief Send alert to all registered callbacks
     * @param message Alert message
     */
    void sendAlert(const std::string& message);

    /**
     * @brief Convert string to lowercase for case-insensitive search
     * @param str Input string
     * @return Lowercase string
     */
    std::string toLowerCase(const std::string& str) const;
};

} // namespace quirkventory