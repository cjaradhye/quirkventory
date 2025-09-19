#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <ostream>

namespace quirkventory {

/**
 * @brief Abstract base class for all products in the inventory system
 * 
 * Demonstrates encapsulation with protected members and public interface.
 * Uses virtual methods for polymorphic behavior.
 */
class Product {
protected:
    std::string id_;
    std::string name_;
    std::string category_;
    double price_;
    int quantity_;
    std::chrono::system_clock::time_point created_date_;

public:
    /**
     * @brief Constructor for Product
     * @param id Unique product identifier
     * @param name Product name
     * @param category Product category
     * @param price Product price
     * @param quantity Initial quantity
     */
    Product(const std::string& id, 
            const std::string& name,
            const std::string& category,
            double price, 
            int quantity);

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~Product() = default;

    // Getters (const methods for encapsulation)
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getCategory() const { return category_; }
    double getPrice() const { return price_; }
    int getQuantity() const { return quantity_; }
    const std::chrono::system_clock::time_point& getCreatedDate() const { return created_date_; }

    // Setters with validation
    void setName(const std::string& name);
    void setCategory(const std::string& category);
    void setPrice(double price);
    void setQuantity(int quantity);
    void addQuantity(int amount);
    void removeQuantity(int amount);

    /**
     * @brief Virtual method for getting product information
     * @return Formatted string with product details
     * 
     * This method demonstrates polymorphism - derived classes can override
     * to provide specialized information formatting
     */
    virtual std::string getInfo() const;

    /**
     * @brief Virtual method to check if product has expired
     * @return true if expired, false otherwise
     * 
     * Base implementation always returns false.
     * PerishableProduct overrides this with actual expiry logic.
     */
    virtual bool isExpired() const { return false; }

    /**
     * @brief Virtual method to get expiry information
     * @return String describing expiry status
     */
    virtual std::string getExpiryInfo() const { return "Non-perishable"; }

    /**
     * @brief Calculate total value of this product in inventory
     * @return Total value (price * quantity)
     */
    double getTotalValue() const { return price_ * quantity_; }

    /**
     * @brief Check if product is low in stock
     * @param threshold Minimum stock threshold
     * @return true if quantity is below threshold
     */
    bool isLowStock(int threshold) const { return quantity_ < threshold; }

    /**
     * @brief Stream output operator for easy printing
     */
    friend std::ostream& operator<<(std::ostream& os, const Product& product);

    /**
     * @brief Pure virtual method for cloning products
     * @return Unique pointer to cloned product
     */
    virtual std::unique_ptr<Product> clone() const = 0;
};

/**
 * @brief Derived class for perishable products
 * 
 * Demonstrates inheritance from Product class and overrides virtual methods
 * to provide specialized behavior for products with expiry dates.
 */
class PerishableProduct : public Product {
private:
    std::chrono::system_clock::time_point expiry_date_;
    std::string storage_requirements_;
    double storage_temperature_;

public:
    /**
     * @brief Constructor for PerishableProduct
     * @param id Unique product identifier
     * @param name Product name
     * @param category Product category
     * @param price Product price
     * @param quantity Initial quantity
     * @param expiry_date Expiration date
     * @param storage_requirements Storage instructions
     * @param storage_temperature Required storage temperature (Celsius)
     */
    PerishableProduct(const std::string& id,
                     const std::string& name,
                     const std::string& category,
                     double price,
                     int quantity,
                     const std::chrono::system_clock::time_point& expiry_date,
                     const std::string& storage_requirements = "",
                     double storage_temperature = 20.0);

    // Getters for perishable-specific attributes
    const std::chrono::system_clock::time_point& getExpiryDate() const { return expiry_date_; }
    const std::string& getStorageRequirements() const { return storage_requirements_; }
    double getStorageTemperature() const { return storage_temperature_; }

    // Setters for perishable-specific attributes
    void setExpiryDate(const std::chrono::system_clock::time_point& expiry_date);
    void setStorageRequirements(const std::string& requirements);
    void setStorageTemperature(double temperature);

    /**
     * @brief Override: Check if product has expired
     * @return true if current date is past expiry date
     */
    bool isExpired() const override;

    /**
     * @brief Override: Get detailed product information including expiry
     * @return Formatted string with all product details
     */
    std::string getInfo() const override;

    /**
     * @brief Override: Get expiry information
     * @return String with days until expiry or "EXPIRED"
     */
    std::string getExpiryInfo() const override;

    /**
     * @brief Get days until expiry
     * @return Number of days (negative if expired)
     */
    int getDaysUntilExpiry() const;

    /**
     * @brief Check if product expires soon
     * @param days Number of days to check ahead
     * @return true if expires within specified days
     */
    bool expiresSoon(int days = 7) const;

    /**
     * @brief Override: Clone this perishable product
     * @return Unique pointer to cloned product
     */
    std::unique_ptr<Product> clone() const override;
};

} // namespace quirkventory