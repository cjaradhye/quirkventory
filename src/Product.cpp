#include "../include/Product.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace quirkventory {

// Product Implementation

Product::Product(const std::string& id,
                const std::string& name,
                const std::string& category,
                double price,
                int quantity)
    : id_(id), name_(name), category_(category), price_(price), quantity_(quantity),
      created_date_(std::chrono::system_clock::now()) {
    
    if (id.empty()) {
        throw std::invalid_argument("Product ID cannot be empty");
    }
    if (name.empty()) {
        throw std::invalid_argument("Product name cannot be empty");
    }
    if (price < 0) {
        throw std::invalid_argument("Product price cannot be negative");
    }
    if (quantity < 0) {
        throw std::invalid_argument("Product quantity cannot be negative");
    }
}

void Product::setName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Product name cannot be empty");
    }
    name_ = name;
}

void Product::setCategory(const std::string& category) {
    category_ = category;
}

void Product::setPrice(double price) {
    if (price < 0) {
        throw std::invalid_argument("Product price cannot be negative");
    }
    price_ = price;
}

void Product::setQuantity(int quantity) {
    if (quantity < 0) {
        throw std::invalid_argument("Product quantity cannot be negative");
    }
    quantity_ = quantity;
}

void Product::addQuantity(int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Amount to add cannot be negative");
    }
    quantity_ += amount;
}

void Product::removeQuantity(int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Amount to remove cannot be negative");
    }
    if (amount > quantity_) {
        throw std::invalid_argument("Cannot remove more quantity than available");
    }
    quantity_ -= amount;
}

std::string Product::getInfo() const {
    std::ostringstream oss;
    oss << "Product ID: " << id_ << "\n"
        << "Name: " << name_ << "\n"
        << "Category: " << category_ << "\n"
        << "Price: $" << std::fixed << std::setprecision(2) << price_ << "\n"
        << "Quantity: " << quantity_ << "\n"
        << "Total Value: $" << std::fixed << std::setprecision(2) << getTotalValue() << "\n"
        << "Expiry Status: " << getExpiryInfo();
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Product& product) {
    os << product.getInfo();
    return os;
}

// PerishableProduct Implementation

PerishableProduct::PerishableProduct(const std::string& id,
                                   const std::string& name,
                                   const std::string& category,
                                   double price,
                                   int quantity,
                                   const std::chrono::system_clock::time_point& expiry_date,
                                   const std::string& storage_requirements,
                                   double storage_temperature)
    : Product(id, name, category, price, quantity),
      expiry_date_(expiry_date),
      storage_requirements_(storage_requirements),
      storage_temperature_(storage_temperature) {
    
    // Validate that expiry date is in the future
    auto now = std::chrono::system_clock::now();
    if (expiry_date < now) {
        throw std::invalid_argument("Expiry date cannot be in the past");
    }
}

void PerishableProduct::setExpiryDate(const std::chrono::system_clock::time_point& expiry_date) {
    auto now = std::chrono::system_clock::now();
    if (expiry_date < now) {
        throw std::invalid_argument("Expiry date cannot be in the past");
    }
    expiry_date_ = expiry_date;
}

void PerishableProduct::setStorageRequirements(const std::string& requirements) {
    storage_requirements_ = requirements;
}

void PerishableProduct::setStorageTemperature(double temperature) {
    storage_temperature_ = temperature;
}

bool PerishableProduct::isExpired() const {
    auto now = std::chrono::system_clock::now();
    return now > expiry_date_;
}

std::string PerishableProduct::getInfo() const {
    std::ostringstream oss;
    oss << Product::getInfo() << "\n"
        << "Storage Temperature: " << storage_temperature_ << "°C\n"
        << "Storage Requirements: " << storage_requirements_ << "\n";
    
    // Format expiry date
    auto time_t = std::chrono::system_clock::to_time_t(expiry_date_);
    oss << "Expiry Date: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
    
    if (isExpired()) {
        oss << "STATUS: **EXPIRED**";
    } else if (expiresSoon()) {
        oss << "STATUS: **EXPIRES SOON** (" << getDaysUntilExpiry() << " days)";
    } else {
        oss << "STATUS: Fresh (" << getDaysUntilExpiry() << " days remaining)";
    }
    
    return oss.str();
}

std::string PerishableProduct::getExpiryInfo() const {
    if (isExpired()) {
        return "EXPIRED";
    } else {
        int days = getDaysUntilExpiry();
        return std::to_string(days) + " days remaining";
    }
}

int PerishableProduct::getDaysUntilExpiry() const {
    auto now = std::chrono::system_clock::now();
    auto duration = expiry_date_ - now;
    auto days = std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;
    return static_cast<int>(days);
}

bool PerishableProduct::expiresSoon(int days) const {
    if (isExpired()) {
        return true;
    }
    return getDaysUntilExpiry() <= days;
}

std::unique_ptr<Product> PerishableProduct::clone() const {
    return std::make_unique<PerishableProduct>(
        id_, name_, category_, price_, quantity_,
        expiry_date_, storage_requirements_, storage_temperature_
    );
}

} // namespace quirkventory