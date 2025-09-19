#include "../include/Inventory.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace quirkventory {

Inventory::Inventory(int default_threshold)
    : default_low_stock_threshold_(default_threshold) {
}

bool Inventory::addProduct(std::unique_ptr<Product> product) {
    if (!product) {
        return false;
    }

    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    const std::string& product_id = product->getId();
    
    // Check if product already exists
    if (products_.find(product_id) != products_.end()) {
        return false; // Product ID already exists
    }

    products_[product_id] = std::move(product);
    return true;
}

bool Inventory::removeProduct(const std::string& product_id) {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return false; // Product not found
    }

    products_.erase(it);
    return true;
}

bool Inventory::updateQuantity(const std::string& product_id, int new_quantity) {
    if (new_quantity < 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return false; // Product not found
    }

    try {
        it->second->setQuantity(new_quantity);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Inventory::addQuantity(const std::string& product_id, int amount) {
    if (amount < 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return false; // Product not found
    }

    try {
        it->second->addQuantity(amount);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Inventory::removeQuantity(const std::string& product_id, int amount) {
    if (amount < 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return false; // Product not found
    }

    try {
        if (it->second->getQuantity() < amount) {
            return false; // Insufficient quantity
        }
        it->second->removeQuantity(amount);
        
        // Check if this creates a low stock situation
        int threshold = getThreshold(product_id);
        if (it->second->getQuantity() < threshold) {
            std::string alert = "LOW STOCK ALERT: Product '" + 
                              it->second->getName() + "' (ID: " + product_id + 
                              ") is now at " + std::to_string(it->second->getQuantity()) + 
                              " units (threshold: " + std::to_string(threshold) + ")";
            sendAlert(alert);
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

const Product* Inventory::getProduct(const std::string& product_id) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

std::vector<const Product*> Inventory::getAllProducts() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    result.reserve(products_.size());
    
    for (const auto& pair : products_) {
        result.push_back(pair.second.get());
    }
    
    return result;
}

std::vector<const Product*> Inventory::searchByName(const std::string& name_pattern) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    std::string lower_pattern = toLowerCase(name_pattern);
    
    for (const auto& pair : products_) {
        std::string lower_name = toLowerCase(pair.second->getName());
        if (lower_name.find(lower_pattern) != std::string::npos) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<const Product*> Inventory::getProductsByCategory(const std::string& category) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    
    for (const auto& pair : products_) {
        if (pair.second->getCategory() == category) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<const Product*> Inventory::getLowStockProducts() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    
    for (const auto& pair : products_) {
        int threshold = getThreshold(pair.first);
        if (pair.second->getQuantity() < threshold) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<const Product*> Inventory::getExpiredProducts() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    
    for (const auto& pair : products_) {
        if (pair.second->isExpired()) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<const Product*> Inventory::getExpiringSoonProducts(int days) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<const Product*> result;
    
    for (const auto& pair : products_) {
        // Check if product is perishable by attempting to cast
        const PerishableProduct* perishable = 
            dynamic_cast<const PerishableProduct*>(pair.second.get());
        
        if (perishable && perishable->expiresSoon(days)) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

size_t Inventory::getTotalProductCount() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    return products_.size();
}

int Inventory::getTotalQuantity() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    int total = 0;
    for (const auto& pair : products_) {
        total += pair.second->getQuantity();
    }
    
    return total;
}

double Inventory::getTotalValue() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    double total = 0.0;
    for (const auto& pair : products_) {
        total += pair.second->getTotalValue();
    }
    
    return total;
}

std::unordered_map<std::string, double> Inventory::getValueByCategory() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::unordered_map<std::string, double> category_values;
    
    for (const auto& pair : products_) {
        const std::string& category = pair.second->getCategory();
        category_values[category] += pair.second->getTotalValue();
    }
    
    return category_values;
}

void Inventory::setCategoryThreshold(const std::string& category, int threshold) {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    category_thresholds_[category] = threshold;
}

int Inventory::getThreshold(const std::string& product_id) const {
    // Note: This method assumes inventory_mutex_ is already locked by the caller
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return default_low_stock_threshold_;
    }
    
    const std::string& category = it->second->getCategory();
    auto threshold_it = category_thresholds_.find(category);
    
    if (threshold_it != category_thresholds_.end()) {
        return threshold_it->second;
    }
    
    return default_low_stock_threshold_;
}

void Inventory::registerAlertCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    alert_callbacks_.push_back(callback);
}

void Inventory::checkAndSendLowStockAlerts() {
    auto low_stock_products = getLowStockProducts();
    
    if (!low_stock_products.empty()) {
        std::ostringstream oss;
        oss << "LOW STOCK ALERT: " << low_stock_products.size() << " products are low in stock:\n";
        
        for (const auto* product : low_stock_products) {
            int threshold = getThreshold(product->getId());
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - Current: " << product->getQuantity() 
                << ", Threshold: " << threshold << "\n";
        }
        
        sendAlert(oss.str());
    }
}

void Inventory::checkAndSendExpiryAlerts() {
    auto expired_products = getExpiredProducts();
    auto expiring_products = getExpiringSoonProducts();
    
    if (!expired_products.empty()) {
        std::ostringstream oss;
        oss << "EXPIRED PRODUCTS ALERT: " << expired_products.size() << " products have expired:\n";
        
        for (const auto* product : expired_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << "\n";
        }
        
        sendAlert(oss.str());
    }
    
    if (!expiring_products.empty()) {
        std::ostringstream oss;
        oss << "EXPIRING SOON ALERT: " << expiring_products.size() << " products expire soon:\n";
        
        for (const auto* product : expiring_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << "\n";
        }
        
        sendAlert(oss.str());
    }
}

std::string Inventory::generateInventoryReport() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "=== INVENTORY REPORT ===\n\n";
    oss << "Total Products: " << getTotalProductCount() << "\n";
    oss << "Total Quantity: " << getTotalQuantity() << "\n";
    oss << "Total Value: $" << getTotalValue() << "\n\n";
    
    auto category_values = getValueByCategory();
    if (!category_values.empty()) {
        oss << "Value by Category:\n";
        for (const auto& pair : category_values) {
            oss << "- " << pair.first << ": $" << pair.second << "\n";
        }
        oss << "\n";
    }
    
    auto low_stock = getLowStockProducts();
    oss << "Low Stock Items: " << low_stock.size() << "\n";
    
    auto expired = getExpiredProducts();
    oss << "Expired Items: " << expired.size() << "\n";
    
    auto expiring = getExpiringSoonProducts();
    oss << "Expiring Soon: " << expiring.size() << "\n";
    
    return oss.str();
}

std::string Inventory::generateLowStockReport() const {
    std::ostringstream oss;
    auto low_stock_products = getLowStockProducts();
    
    oss << "=== LOW STOCK REPORT ===\n\n";
    
    if (low_stock_products.empty()) {
        oss << "No products are currently low in stock.\n";
    } else {
        oss << "Found " << low_stock_products.size() << " products with low stock:\n\n";
        
        for (const auto* product : low_stock_products) {
            int threshold = getThreshold(product->getId());
            oss << "Product: " << product->getName() << "\n";
            oss << "ID: " << product->getId() << "\n";
            oss << "Category: " << product->getCategory() << "\n";
            oss << "Current Stock: " << product->getQuantity() << "\n";
            oss << "Threshold: " << threshold << "\n";
            oss << "Price: $" << std::fixed << std::setprecision(2) << product->getPrice() << "\n";
            oss << "---\n";
        }
    }
    
    return oss.str();
}

std::string Inventory::generateExpiryReport() const {
    std::ostringstream oss;
    auto expired_products = getExpiredProducts();
    auto expiring_products = getExpiringSoonProducts();
    
    oss << "=== EXPIRY REPORT ===\n\n";
    
    if (!expired_products.empty()) {
        oss << "EXPIRED PRODUCTS (" << expired_products.size() << "):\n";
        for (const auto* product : expired_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << "\n";
        }
        oss << "\n";
    }
    
    if (!expiring_products.empty()) {
        oss << "EXPIRING SOON (" << expiring_products.size() << "):\n";
        for (const auto* product : expiring_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << "\n";
        }
        oss << "\n";
    }
    
    if (expired_products.empty() && expiring_products.empty()) {
        oss << "No products are expired or expiring soon.\n";
    }
    
    return oss.str();
}

bool Inventory::hasProduct(const std::string& product_id) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    return products_.find(product_id) != products_.end();
}

int Inventory::getAvailableQuantity(const std::string& product_id) const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    auto it = products_.find(product_id);
    if (it == products_.end()) {
        return -1; // Product not found
    }
    
    return it->second->getQuantity();
}

std::vector<std::string> Inventory::validateInventory() const {
    std::lock_guard<std::mutex> lock(inventory_mutex_);
    
    std::vector<std::string> errors;
    
    for (const auto& pair : products_) {
        const Product* product = pair.second.get();
        
        // Check for negative quantities
        if (product->getQuantity() < 0) {
            errors.push_back("Product " + product->getId() + " has negative quantity: " + 
                           std::to_string(product->getQuantity()));
        }
        
        // Check for negative prices
        if (product->getPrice() < 0) {
            errors.push_back("Product " + product->getId() + " has negative price: $" + 
                           std::to_string(product->getPrice()));
        }
        
        // Check for expired products
        if (product->isExpired()) {
            errors.push_back("Product " + product->getId() + " (" + product->getName() + 
                           ") has expired: " + product->getExpiryInfo());
        }
    }
    
    return errors;
}

void Inventory::sendAlert(const std::string& message) {
    // Note: This method assumes inventory_mutex_ is already locked by the caller
    for (const auto& callback : alert_callbacks_) {
        try {
            callback(message);
        } catch (const std::exception&) {
            // Silently ignore callback errors to prevent system instability
        }
    }
}

std::string Inventory::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

} // namespace quirkventory