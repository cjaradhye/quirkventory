#include "../include/NotificationSystem.hpp"
#include "../include/Inventory.hpp"
#include "../include/Order.hpp"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

namespace quirkventory {

// Helper function implementations

std::string priorityToString(NotificationPriority priority) {
    switch (priority) {
        case NotificationPriority::LOW: return "LOW";
        case NotificationPriority::MEDIUM: return "MEDIUM";
        case NotificationPriority::HIGH: return "HIGH";
        case NotificationPriority::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Notification Implementation

Notification::Notification(const std::string& message,
                          NotificationPriority priority,
                          const std::string& sender_id)
    : message_(message), timestamp_(std::chrono::system_clock::now()),
      priority_(priority), sender_id_(sender_id) {
}

void Notification::setMessage(const std::string& message) {
    message_ = message;
}

void Notification::setPriority(NotificationPriority priority) {
    priority_ = priority;
}

void Notification::addRecipient(const std::string& recipient_id) {
    if (std::find(recipient_ids_.begin(), recipient_ids_.end(), recipient_id) == recipient_ids_.end()) {
        recipient_ids_.push_back(recipient_id);
    }
}

void Notification::clearRecipients() {
    recipient_ids_.clear();
}

std::string Notification::format() const {
    std::ostringstream oss;
    
    auto time_t = std::chrono::system_clock::to_time_t(timestamp_);
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    oss << "[" << priorityToString(priority_) << "] ";
    oss << message_;
    
    if (!recipient_ids_.empty()) {
        oss << " (Recipients: ";
        for (size_t i = 0; i < recipient_ids_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << recipient_ids_[i];
        }
        oss << ")";
    }
    
    return oss.str();
}

std::string Notification::getSummary() const {
    std::string summary = message_;
    if (summary.length() > 50) {
        summary = summary.substr(0, 47) + "...";
    }
    return "[" + priorityToString(priority_) + "] " + summary;
}

bool Notification::isHighPriority() const {
    return priority_ == NotificationPriority::HIGH || priority_ == NotificationPriority::CRITICAL;
}

long long Notification::getAgeInMinutes() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now - timestamp_;
    return std::chrono::duration_cast<std::chrono::minutes>(duration).count();
}

// EmailNotification Implementation

EmailNotification::EmailNotification(const std::string& message,
                                   const std::string& subject,
                                   NotificationPriority priority,
                                   const std::string& sender_id)
    : Notification(message, priority, sender_id), subject_(subject) {
    
    email_body_ = message;
}

void EmailNotification::setSubject(const std::string& subject) {
    subject_ = subject;
}

void EmailNotification::setEmailBody(const std::string& body) {
    email_body_ = body;
}

void EmailNotification::addAttachment(const std::string& attachment_path) {
    attachments_.push_back(attachment_path);
}

bool EmailNotification::send() {
    // Simulate email sending (in a real implementation, this would use an email service)
    std::cout << "=== EMAIL NOTIFICATION ===" << std::endl;
    std::cout << "To: ";
    for (size_t i = 0; i < recipient_ids_.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << recipient_ids_[i];
    }
    std::cout << std::endl;
    std::cout << "Subject: " << subject_ << std::endl;
    std::cout << "Priority: " << priorityToString(priority_) << std::endl;
    std::cout << "Body: " << email_body_ << std::endl;
    
    if (!attachments_.empty()) {
        std::cout << "Attachments: ";
        for (size_t i = 0; i < attachments_.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << attachments_[i];
        }
        std::cout << std::endl;
    }
    
    std::cout << "=========================" << std::endl;
    
    return true; // Assume successful delivery
}

std::string EmailNotification::format() const {
    std::ostringstream oss;
    oss << Notification::format() << std::endl;
    oss << "Subject: " << subject_ << std::endl;
    oss << "Body: " << email_body_;
    
    if (!attachments_.empty()) {
        oss << std::endl << "Attachments: ";
        for (size_t i = 0; i < attachments_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << attachments_[i];
        }
    }
    
    return oss.str();
}

// SystemNotification Implementation

SystemNotification::SystemNotification(const std::string& message,
                                     const std::string& category,
                                     NotificationPriority priority,
                                     const std::string& sender_id,
                                     bool persistent)
    : Notification(message, priority, sender_id), category_(category), is_persistent_(persistent) {
}

void SystemNotification::setCategory(const std::string& category) {
    category_ = category;
}

void SystemNotification::setPersistent(bool persistent) {
    is_persistent_ = persistent;
}

bool SystemNotification::send() {
    // Log system notification (simulate logging to console/file)
    std::cout << "=== SYSTEM NOTIFICATION ===" << std::endl;
    std::cout << format() << std::endl;
    std::cout << "Category: " << category_ << std::endl;
    std::cout << "Persistent: " << (is_persistent_ ? "Yes" : "No") << std::endl;
    std::cout << "==========================" << std::endl;
    
    return true; // Assume successful logging
}

std::string SystemNotification::format() const {
    std::ostringstream oss;
    oss << "[" << category_ << "] " << Notification::format();
    if (is_persistent_) {
        oss << " [PERSISTENT]";
    }
    return oss.str();
}

// Report Implementation

Report::Report(const std::string& title, const std::string& generated_by)
    : title_(title), generated_date_(std::chrono::system_clock::now()), generated_by_(generated_by) {
}

bool Report::exportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // This would typically call generate(), but we need the derived class implementation
    // For now, just export the header and footer
    file << getHeader() << std::endl;
    file << "Report content would be generated here..." << std::endl;
    file << getFooter() << std::endl;
    
    file.close();
    return true;
}

std::string Report::getHeader() const {
    std::ostringstream oss;
    oss << "========================================" << std::endl;
    oss << title_ << std::endl;
    oss << "========================================" << std::endl;
    
    auto time_t = std::chrono::system_clock::to_time_t(generated_date_);
    oss << "Generated: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    oss << "Generated by: " << generated_by_ << std::endl;
    oss << "========================================" << std::endl;
    
    return oss.str();
}

std::string Report::getFooter() const {
    std::ostringstream oss;
    oss << "========================================" << std::endl;
    oss << "End of Report" << std::endl;
    oss << "========================================" << std::endl;
    
    return oss.str();
}

void Report::addSection(const std::string& section_content) {
    sections_.push_back(section_content);
}

void Report::clearSections() {
    sections_.clear();
}

// SalesReport Implementation

SalesReport::SalesReport(const OrderManager* order_manager,
                        const std::chrono::system_clock::time_point& start_date,
                        const std::chrono::system_clock::time_point& end_date,
                        const std::string& generated_by)
    : Report("Sales Report", generated_by), order_manager_(order_manager),
      start_date_(start_date), end_date_(end_date) {
}

std::string SalesReport::generate() {
    clearSections();
    
    std::ostringstream oss;
    oss << getHeader() << std::endl;
    
    // Add date range
    auto start_time_t = std::chrono::system_clock::to_time_t(start_date_);
    auto end_time_t = std::chrono::system_clock::to_time_t(end_date_);
    oss << "Report Period: " << std::put_time(std::localtime(&start_time_t), "%Y-%m-%d") 
        << " to " << std::put_time(std::localtime(&end_time_t), "%Y-%m-%d") << std::endl << std::endl;
    
    // Generate sections
    oss << generateOrderSummary() << std::endl;
    oss << generateRevenueAnalysis() << std::endl;
    oss << generateCustomerAnalysis() << std::endl;
    
    oss << getFooter();
    
    return oss.str();
}

std::string SalesReport::generateOrderSummary() const {
    std::ostringstream oss;
    oss << "ORDER SUMMARY" << std::endl;
    oss << "-------------" << std::endl;
    
    if (!order_manager_) {
        oss << "No order data available." << std::endl;
        return oss.str();
    }
    
    auto all_orders = order_manager_->getAllOrders();
    
    // Filter orders by date range
    std::vector<Order*> period_orders;
    for (auto* order : all_orders) {
        auto order_date = order->getOrderDate();
        if (order_date >= start_date_ && order_date <= end_date_) {
            period_orders.push_back(order);
        }
    }
    
    oss << "Total Orders in Period: " << period_orders.size() << std::endl;
    
    // Count by status
    std::unordered_map<OrderStatus, int> status_counts;
    double total_revenue = 0.0;
    
    for (auto* order : period_orders) {
        status_counts[order->getStatus()]++;
        if (order->getStatus() == OrderStatus::CONFIRMED || 
            order->getStatus() == OrderStatus::SHIPPED ||
            order->getStatus() == OrderStatus::DELIVERED) {
            total_revenue += order->getTotalAmount();
        }
    }
    
    oss << std::fixed << std::setprecision(2);
    oss << "Total Revenue: $" << total_revenue << std::endl << std::endl;
    
    oss << "Orders by Status:" << std::endl;
    for (const auto& pair : status_counts) {
        oss << "- " << orderStatusToString(pair.first) << ": " << pair.second << std::endl;
    }
    
    return oss.str();
}

std::string SalesReport::generateRevenueAnalysis() const {
    std::ostringstream oss;
    oss << std::endl << "REVENUE ANALYSIS" << std::endl;
    oss << "----------------" << std::endl;
    
    if (!order_manager_) {
        oss << "No order data available for revenue analysis." << std::endl;
        return oss.str();
    }
    
    // This would contain more detailed revenue analysis
    // For now, provide basic information
    oss << "Detailed revenue breakdown by product category, time period, etc." << std::endl;
    oss << "would be implemented here with real order data analysis." << std::endl;
    
    return oss.str();
}

std::string SalesReport::generateCustomerAnalysis() const {
    std::ostringstream oss;
    oss << std::endl << "CUSTOMER ANALYSIS" << std::endl;
    oss << "-----------------" << std::endl;
    
    if (!order_manager_) {
        oss << "No order data available for customer analysis." << std::endl;
        return oss.str();
    }
    
    auto all_orders = order_manager_->getAllOrders();
    std::unordered_map<std::string, int> customer_order_counts;
    std::unordered_map<std::string, double> customer_totals;
    
    for (auto* order : all_orders) {
        auto order_date = order->getOrderDate();
        if (order_date >= start_date_ && order_date <= end_date_) {
            const std::string& customer_id = order->getCustomerId();
            customer_order_counts[customer_id]++;
            if (order->getStatus() == OrderStatus::CONFIRMED || 
                order->getStatus() == OrderStatus::SHIPPED ||
                order->getStatus() == OrderStatus::DELIVERED) {
                customer_totals[customer_id] += order->getTotalAmount();
            }
        }
    }
    
    oss << "Unique Customers: " << customer_order_counts.size() << std::endl;
    oss << std::fixed << std::setprecision(2);
    
    if (!customer_totals.empty()) {
        // Find top customer by revenue
        auto top_customer = std::max_element(customer_totals.begin(), customer_totals.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        oss << "Top Customer by Revenue: " << top_customer->first 
            << " ($" << top_customer->second << ")" << std::endl;
    }
    
    return oss.str();
}

// InventoryReport Implementation

InventoryReport::InventoryReport(const Inventory* inventory,
                               const std::string& generated_by,
                               bool include_low_stock,
                               bool include_expired)
    : Report("Inventory Report", generated_by), inventory_(inventory),
      include_low_stock_(include_low_stock), include_expired_(include_expired) {
}

std::string InventoryReport::generate() {
    clearSections();
    
    std::ostringstream oss;
    oss << getHeader() << std::endl;
    
    // Generate sections
    oss << generateInventoryOverview() << std::endl;
    oss << generateCategoryBreakdown() << std::endl;
    
    if (include_low_stock_) {
        oss << generateLowStockSection() << std::endl;
    }
    
    if (include_expired_) {
        oss << generateExpiredSection() << std::endl;
    }
    
    oss << getFooter();
    
    return oss.str();
}

std::string InventoryReport::generateInventoryOverview() const {
    std::ostringstream oss;
    oss << "INVENTORY OVERVIEW" << std::endl;
    oss << "------------------" << std::endl;
    
    if (!inventory_) {
        oss << "No inventory data available." << std::endl;
        return oss.str();
    }
    
    oss << std::fixed << std::setprecision(2);
    oss << "Total Products: " << inventory_->getTotalProductCount() << std::endl;
    oss << "Total Quantity: " << inventory_->getTotalQuantity() << std::endl;
    oss << "Total Value: $" << inventory_->getTotalValue() << std::endl;
    
    auto low_stock = inventory_->getLowStockProducts();
    auto expired = inventory_->getExpiredProducts();
    auto expiring = inventory_->getExpiringSoonProducts();
    
    oss << "Low Stock Items: " << low_stock.size() << std::endl;
    oss << "Expired Items: " << expired.size() << std::endl;
    oss << "Items Expiring Soon: " << expiring.size() << std::endl;
    
    return oss.str();
}

std::string InventoryReport::generateCategoryBreakdown() const {
    std::ostringstream oss;
    oss << std::endl << "CATEGORY BREAKDOWN" << std::endl;
    oss << "------------------" << std::endl;
    
    if (!inventory_) {
        oss << "No inventory data available." << std::endl;
        return oss.str();
    }
    
    auto category_values = inventory_->getValueByCategory();
    
    oss << std::fixed << std::setprecision(2);
    oss << "Value by Category:" << std::endl;
    for (const auto& pair : category_values) {
        oss << "- " << pair.first << ": $" << pair.second << std::endl;
    }
    
    return oss.str();
}

std::string InventoryReport::generateLowStockSection() const {
    std::ostringstream oss;
    oss << std::endl << "LOW STOCK ANALYSIS" << std::endl;
    oss << "------------------" << std::endl;
    
    if (!inventory_) {
        oss << "No inventory data available." << std::endl;
        return oss.str();
    }
    
    auto low_stock_products = inventory_->getLowStockProducts();
    
    if (low_stock_products.empty()) {
        oss << "No products are currently low in stock." << std::endl;
    } else {
        oss << "Products requiring attention (" << low_stock_products.size() << "):" << std::endl;
        for (const auto* product : low_stock_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - Stock: " << product->getQuantity() << std::endl;
        }
    }
    
    return oss.str();
}

std::string InventoryReport::generateExpiredSection() const {
    std::ostringstream oss;
    oss << std::endl << "EXPIRY ANALYSIS" << std::endl;
    oss << "---------------" << std::endl;
    
    if (!inventory_) {
        oss << "No inventory data available." << std::endl;
        return oss.str();
    }
    
    auto expired_products = inventory_->getExpiredProducts();
    auto expiring_products = inventory_->getExpiringSoonProducts();
    
    if (!expired_products.empty()) {
        oss << "EXPIRED PRODUCTS (" << expired_products.size() << "):" << std::endl;
        for (const auto* product : expired_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << std::endl;
        }
        oss << std::endl;
    }
    
    if (!expiring_products.empty()) {
        oss << "EXPIRING SOON (" << expiring_products.size() << "):" << std::endl;
        for (const auto* product : expiring_products) {
            oss << "- " << product->getName() << " (ID: " << product->getId() 
                << ") - " << product->getExpiryInfo() << std::endl;
        }
    }
    
    if (expired_products.empty() && expiring_products.empty()) {
        oss << "No products are expired or expiring soon." << std::endl;
    }
    
    return oss.str();
}

// NotificationManager Implementation

NotificationManager::NotificationManager(size_t max_history)
    : max_history_size_(max_history) {
}

bool NotificationManager::sendEmailNotification(const std::string& message,
                                               const std::string& subject,
                                               const std::vector<std::string>& recipients,
                                               NotificationPriority priority) {
    auto notification = std::make_unique<EmailNotification>(message, subject, priority);
    
    for (const auto& recipient : recipients) {
        notification->addRecipient(recipient);
    }
    
    bool success = notification->send();
    
    if (success) {
        notifyCallbacks(*notification);
        addToHistory(std::move(notification));
    }
    
    return success;
}

bool NotificationManager::sendSystemNotification(const std::string& message,
                                                const std::string& category,
                                                const std::vector<std::string>& recipients,
                                                NotificationPriority priority) {
    auto notification = std::make_unique<SystemNotification>(message, category, priority);
    
    for (const auto& recipient : recipients) {
        notification->addRecipient(recipient);
    }
    
    bool success = notification->send();
    
    if (success) {
        notifyCallbacks(*notification);
        addToHistory(std::move(notification));
    }
    
    return success;
}

void NotificationManager::registerNotificationCallback(std::function<void(const Notification&)> callback) {
    notification_callbacks_.push_back(callback);
}

std::vector<const Notification*> NotificationManager::getNotificationHistory(size_t limit) const {
    std::vector<const Notification*> result;
    
    size_t count = (limit == 0) ? notification_history_.size() : std::min(limit, notification_history_.size());
    result.reserve(count);
    
    // Return most recent notifications first
    for (size_t i = 0; i < count; ++i) {
        size_t index = notification_history_.size() - 1 - i;
        result.push_back(notification_history_[index].get());
    }
    
    return result;
}

std::vector<const Notification*> NotificationManager::getHighPriorityNotifications() const {
    std::vector<const Notification*> result;
    
    for (const auto& notification : notification_history_) {
        if (notification->isHighPriority()) {
            result.push_back(notification.get());
        }
    }
    
    return result;
}

void NotificationManager::clearHistory() {
    notification_history_.clear();
}

std::unique_ptr<SalesReport> NotificationManager::generateSalesReport(const OrderManager& order_manager,
                                                                     const std::chrono::system_clock::time_point& start_date,
                                                                     const std::chrono::system_clock::time_point& end_date,
                                                                     const std::string& generated_by) const {
    return std::make_unique<SalesReport>(&order_manager, start_date, end_date, generated_by);
}

std::unique_ptr<InventoryReport> NotificationManager::generateInventoryReport(const Inventory& inventory,
                                                                             const std::string& generated_by,
                                                                             bool include_low_stock,
                                                                             bool include_expired) const {
    return std::make_unique<InventoryReport>(&inventory, generated_by, include_low_stock, include_expired);
}

void NotificationManager::sendInventoryAlerts(const Inventory& inventory) {
    // Send low stock alerts
    auto low_stock_products = inventory.getLowStockProducts();
    if (!low_stock_products.empty()) {
        std::ostringstream message;
        message << "Low stock alert: " << low_stock_products.size() << " products need restocking.";
        
        sendSystemNotification(message.str(), "low_stock", {"managers"}, NotificationPriority::HIGH);
    }
    
    // Send expiry alerts
    auto expired_products = inventory.getExpiredProducts();
    if (!expired_products.empty()) {
        std::ostringstream message;
        message << "Expired products alert: " << expired_products.size() << " products have expired.";
        
        sendSystemNotification(message.str(), "expired", {"managers", "staff"}, NotificationPriority::CRITICAL);
    }
    
    auto expiring_products = inventory.getExpiringSoonProducts();
    if (!expiring_products.empty()) {
        std::ostringstream message;
        message << "Products expiring soon: " << expiring_products.size() << " products expire within 7 days.";
        
        sendSystemNotification(message.str(), "expiring", {"managers", "staff"}, NotificationPriority::HIGH);
    }
}

std::string NotificationManager::getNotificationStatistics() const {
    std::ostringstream oss;
    
    oss << "=== NOTIFICATION STATISTICS ===" << std::endl;
    oss << "Total Notifications: " << notification_history_.size() << std::endl;
    
    // Count by priority
    std::unordered_map<NotificationPriority, int> priority_counts;
    for (const auto& notification : notification_history_) {
        priority_counts[notification->getPriority()]++;
    }
    
    oss << "Notifications by Priority:" << std::endl;
    for (const auto& pair : priority_counts) {
        oss << "- " << priorityToString(pair.first) << ": " << pair.second << std::endl;
    }
    
    auto high_priority = getHighPriorityNotifications();
    oss << "High Priority Notifications: " << high_priority.size() << std::endl;
    
    oss << "Registered Callbacks: " << notification_callbacks_.size() << std::endl;
    
    return oss.str();
}

void NotificationManager::addToHistory(std::unique_ptr<Notification> notification) {
    notification_history_.push_back(std::move(notification));
    
    // Maintain history size limit
    if (notification_history_.size() > max_history_size_) {
        notification_history_.erase(notification_history_.begin());
    }
}

void NotificationManager::notifyCallbacks(const Notification& notification) {
    for (const auto& callback : notification_callbacks_) {
        try {
            callback(notification);
        } catch (const std::exception&) {
            // Silently ignore callback errors to prevent system instability
        }
    }
}

} // namespace quirkventory