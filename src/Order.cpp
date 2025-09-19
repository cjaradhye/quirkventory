#include "../include/Order.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>

namespace quirkventory {

// Helper function implementations

std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING: return "PENDING";
        case OrderStatus::PROCESSING: return "PROCESSING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::SHIPPED: return "SHIPPED";
        case OrderStatus::DELIVERED: return "DELIVERED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        case OrderStatus::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

// Order Implementation

Order::Order(const std::string& order_id, const std::string& customer_id)
    : order_id_(order_id), customer_id_(customer_id), status_(OrderStatus::PENDING),
      order_date_(std::chrono::system_clock::now()), total_amount_(0.0),
      processing_flag_(false) {
    
    if (order_id.empty()) {
        throw std::invalid_argument("Order ID cannot be empty");
    }
    if (customer_id.empty()) {
        throw std::invalid_argument("Customer ID cannot be empty");
    }
}

OrderStatus Order::getStatus() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    return status_;
}

double Order::getTotalAmount() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    return total_amount_;
}

void Order::setNotes(const std::string& notes) {
    std::lock_guard<std::mutex> lock(order_mutex_);
    notes_ = notes;
}

void Order::setCustomerId(const std::string& customer_id) {
    if (customer_id.empty()) {
        throw std::invalid_argument("Customer ID cannot be empty");
    }
    
    std::lock_guard<std::mutex> lock(order_mutex_);
    if (!canModify()) {
        throw std::runtime_error("Cannot modify order in current status");
    }
    customer_id_ = customer_id;
}

bool Order::addItem(const std::string& product_id, int quantity, double unit_price) {
    if (product_id.empty() || quantity <= 0 || unit_price < 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (!canModify()) {
        return false;
    }

    // Check if item already exists
    auto it = std::find_if(items_.begin(), items_.end(),
        [&product_id](const OrderItem& item) {
            return item.product_id == product_id;
        });

    if (it != items_.end()) {
        // Update existing item quantity
        it->quantity += quantity;
    } else {
        // Add new item
        items_.emplace_back(product_id, quantity, unit_price);
    }

    updateTotalAmount();
    return true;
}

bool Order::removeItem(const std::string& product_id) {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (!canModify()) {
        return false;
    }

    auto it = std::find_if(items_.begin(), items_.end(),
        [&product_id](const OrderItem& item) {
            return item.product_id == product_id;
        });

    if (it != items_.end()) {
        items_.erase(it);
        updateTotalAmount();
        return true;
    }

    return false;
}

bool Order::updateItemQuantity(const std::string& product_id, int new_quantity) {
    if (new_quantity <= 0) {
        return removeItem(product_id);
    }

    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (!canModify()) {
        return false;
    }

    auto it = std::find_if(items_.begin(), items_.end(),
        [&product_id](const OrderItem& item) {
            return item.product_id == product_id;
        });

    if (it != items_.end()) {
        it->quantity = new_quantity;
        updateTotalAmount();
        return true;
    }

    return false;
}

std::vector<OrderItem> Order::getItems() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    return items_;
}

const OrderItem* Order::getItem(const std::string& product_id) const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    auto it = std::find_if(items_.begin(), items_.end(),
        [&product_id](const OrderItem& item) {
            return item.product_id == product_id;
        });

    return (it != items_.end()) ? &(*it) : nullptr;
}

std::vector<std::string> Order::validateOrder(const Inventory& inventory) const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    std::vector<std::string> errors;

    if (items_.empty()) {
        errors.push_back("Order contains no items");
        return errors;
    }

    for (const auto& item : items_) {
        // Check if product exists in inventory
        const Product* product = inventory.getProduct(item.product_id);
        if (!product) {
            errors.push_back("Product not found: " + item.product_id);
            continue;
        }

        // Check if sufficient quantity is available
        if (product->getQuantity() < item.quantity) {
            errors.push_back("Insufficient quantity for product " + item.product_id + 
                           ": requested " + std::to_string(item.quantity) + 
                           ", available " + std::to_string(product->getQuantity()));
        }

        // Check if product is expired
        if (product->isExpired()) {
            errors.push_back("Product is expired: " + item.product_id);
        }

        // Check price consistency (within 5% tolerance)
        double price_diff = std::abs(product->getPrice() - item.unit_price);
        double price_tolerance = product->getPrice() * 0.05;
        if (price_diff > price_tolerance) {
            errors.push_back("Price mismatch for product " + item.product_id + 
                           ": order price $" + std::to_string(item.unit_price) + 
                           ", current price $" + std::to_string(product->getPrice()));
        }
    }

    return errors;
}

bool Order::processOrder(Inventory& inventory) {
    // Check if already processing
    bool expected = false;
    if (!processing_flag_.compare_exchange_strong(expected, true)) {
        setError("Order is already being processed");
        return false;
    }

    bool result = processOrderInternal(inventory);
    processing_flag_.store(false);
    return result;
}

std::future<bool> Order::processOrderAsync(Inventory& inventory) {
    return std::async(std::launch::async, [this, &inventory]() {
        return processOrder(inventory);
    });
}

bool Order::cancelOrder(const std::string& reason) {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (status_ == OrderStatus::DELIVERED || status_ == OrderStatus::SHIPPED) {
        return false; // Cannot cancel delivered or shipped orders
    }

    status_ = OrderStatus::CANCELLED;
    if (!reason.empty()) {
        notes_ = reason;
    }
    
    return true;
}

bool Order::updateStatus(OrderStatus new_status) {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    // Validate status transition
    switch (status_) {
        case OrderStatus::PENDING:
            if (new_status != OrderStatus::PROCESSING && 
                new_status != OrderStatus::CANCELLED &&
                new_status != OrderStatus::FAILED) {
                return false;
            }
            break;
        case OrderStatus::PROCESSING:
            if (new_status != OrderStatus::CONFIRMED && 
                new_status != OrderStatus::FAILED &&
                new_status != OrderStatus::CANCELLED) {
                return false;
            }
            break;
        case OrderStatus::CONFIRMED:
            if (new_status != OrderStatus::SHIPPED && 
                new_status != OrderStatus::CANCELLED) {
                return false;
            }
            break;
        case OrderStatus::SHIPPED:
            if (new_status != OrderStatus::DELIVERED) {
                return false;
            }
            break;
        case OrderStatus::DELIVERED:
        case OrderStatus::CANCELLED:
        case OrderStatus::FAILED:
            return false; // Terminal states
    }

    status_ = new_status;
    if (new_status == OrderStatus::CONFIRMED || new_status == OrderStatus::FAILED) {
        processed_date_ = std::chrono::system_clock::now();
    }
    
    return true;
}

double Order::calculateTotal() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    double total = 0.0;
    for (const auto& item : items_) {
        total += item.getTotalPrice();
    }
    
    return total;
}

std::string Order::getOrderSummary() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "Order ID: " << order_id_ << "\n";
    oss << "Customer: " << customer_id_ << "\n";
    oss << "Status: " << orderStatusToString(status_) << "\n";
    oss << "Items: " << items_.size() << "\n";
    oss << "Total: $" << total_amount_ << "\n";
    
    auto time_t = std::chrono::system_clock::to_time_t(order_date_);
    oss << "Order Date: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    if (!error_message_.empty()) {
        oss << "\nError: " << error_message_;
    }
    
    return oss.str();
}

std::string Order::getDetailedInfo() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "=== ORDER DETAILS ===\n";
    oss << "Order ID: " << order_id_ << "\n";
    oss << "Customer ID: " << customer_id_ << "\n";
    oss << "Status: " << orderStatusToString(status_) << "\n";
    
    auto time_t = std::chrono::system_clock::to_time_t(order_date_);
    oss << "Order Date: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
    
    if (processed_date_ != std::chrono::system_clock::time_point{}) {
        auto proc_time_t = std::chrono::system_clock::to_time_t(processed_date_);
        oss << "Processed Date: " << std::put_time(std::localtime(&proc_time_t), "%Y-%m-%d %H:%M:%S") << "\n";
    }
    
    if (!notes_.empty()) {
        oss << "Notes: " << notes_ << "\n";
    }
    
    oss << "\nITEMS:\n";
    for (const auto& item : items_) {
        oss << "- Product: " << item.product_id 
            << ", Qty: " << item.quantity 
            << ", Unit Price: $" << item.unit_price 
            << ", Total: $" << item.getTotalPrice() << "\n";
    }
    
    oss << "\nOrder Total: $" << total_amount_ << "\n";
    
    if (!error_message_.empty()) {
        oss << "\nERROR: " << error_message_ << "\n";
    }
    
    return oss.str();
}

bool Order::canModify() const {
    // Note: This method assumes order_mutex_ is already locked by the caller
    return status_ == OrderStatus::PENDING;
}

long long Order::getProcessingDuration() const {
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (processed_date_ == std::chrono::system_clock::time_point{}) {
        return -1; // Not processed yet
    }
    
    auto duration = processed_date_ - order_date_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

bool Order::processOrderInternal(Inventory& inventory) {
    // Update status to processing
    {
        std::lock_guard<std::mutex> lock(order_mutex_);
        if (!updateStatus(OrderStatus::PROCESSING)) {
            setError("Cannot process order in current status");
            return false;
        }
    }

    // Validate order
    auto validation_errors = validateOrder(inventory);
    if (!validation_errors.empty()) {
        std::ostringstream error_stream;
        error_stream << "Validation failed: ";
        for (size_t i = 0; i < validation_errors.size(); ++i) {
            if (i > 0) error_stream << "; ";
            error_stream << validation_errors[i];
        }
        setError(error_stream.str());
        updateStatus(OrderStatus::FAILED);
        return false;
    }

    // Process each item and update inventory
    std::vector<std::pair<std::string, int>> processed_items;
    
    try {
        for (const auto& item : items_) {
            if (!inventory.removeQuantity(item.product_id, item.quantity)) {
                // Rollback previously processed items
                for (const auto& processed : processed_items) {
                    inventory.addQuantity(processed.first, processed.second);
                }
                setError("Failed to reserve inventory for product: " + item.product_id);
                updateStatus(OrderStatus::FAILED);
                return false;
            }
            processed_items.emplace_back(item.product_id, item.quantity);
        }
    } catch (const std::exception& e) {
        // Rollback all processed items
        for (const auto& processed : processed_items) {
            inventory.addQuantity(processed.first, processed.second);
        }
        setError(std::string("Exception during processing: ") + e.what());
        updateStatus(OrderStatus::FAILED);
        return false;
    }

    // Order processed successfully
    updateStatus(OrderStatus::CONFIRMED);
    return true;
}

void Order::setError(const std::string& message) {
    // Note: This method assumes order_mutex_ is already locked by the caller
    error_message_ = message;
}

void Order::updateTotalAmount() {
    // Note: This method assumes order_mutex_ is already locked by the caller
    total_amount_ = calculateTotal();
}

// OrderManager Implementation

OrderManager::OrderManager()
    : total_orders_processed_(0), successful_orders_(0), failed_orders_(0) {
}

Order* OrderManager::createOrder(const std::string& order_id, const std::string& customer_id) {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    if (orders_.find(order_id) != orders_.end()) {
        return nullptr; // Order ID already exists
    }

    auto order = std::make_unique<Order>(order_id, customer_id);
    Order* order_ptr = order.get();
    orders_[order_id] = std::move(order);
    
    return order_ptr;
}

Order* OrderManager::getOrder(const std::string& order_id) {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    auto it = orders_.find(order_id);
    return (it != orders_.end()) ? it->second.get() : nullptr;
}

std::vector<Order*> OrderManager::getAllOrders() const {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    std::vector<Order*> result;
    result.reserve(orders_.size());
    
    for (const auto& pair : orders_) {
        result.push_back(pair.second.get());
    }
    
    return result;
}

std::vector<Order*> OrderManager::getOrdersByStatus(OrderStatus status) const {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    std::vector<Order*> result;
    
    for (const auto& pair : orders_) {
        if (pair.second->getStatus() == status) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<Order*> OrderManager::getOrdersByCustomer(const std::string& customer_id) const {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    std::vector<Order*> result;
    
    for (const auto& pair : orders_) {
        if (pair.second->getCustomerId() == customer_id) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

int OrderManager::processAllPendingOrders(Inventory& inventory, int max_concurrent) {
    auto pending_orders = getOrdersByStatus(OrderStatus::PENDING);
    
    if (pending_orders.empty()) {
        return 0;
    }

    std::vector<std::future<bool>> futures;
    int successful_count = 0;
    
    // Process orders in batches to limit concurrency
    for (size_t i = 0; i < pending_orders.size(); i += max_concurrent) {
        futures.clear();
        
        // Start batch of concurrent processing
        size_t batch_end = std::min(i + max_concurrent, pending_orders.size());
        for (size_t j = i; j < batch_end; ++j) {
            futures.push_back(pending_orders[j]->processOrderAsync(inventory));
        }
        
        // Wait for batch to complete and count successes
        for (auto& future : futures) {
            if (future.get()) {
                successful_count++;
                updateStatistics(true);
            } else {
                updateStatistics(false);
            }
        }
    }
    
    return successful_count;
}

bool OrderManager::removeOrder(const std::string& order_id) {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }
    
    orders_.erase(it);
    return true;
}

std::string OrderManager::getStatistics() const {
    std::ostringstream oss;
    
    oss << "=== ORDER STATISTICS ===\n";
    oss << "Total Orders: " << orders_.size() << "\n";
    oss << "Orders Processed: " << total_orders_processed_.load() << "\n";
    oss << "Successful Orders: " << successful_orders_.load() << "\n";
    oss << "Failed Orders: " << failed_orders_.load() << "\n";
    
    int total = total_orders_processed_.load();
    if (total > 0) {
        double success_rate = (double)successful_orders_.load() / total * 100.0;
        oss << "Success Rate: " << std::fixed << std::setprecision(1) << success_rate << "%\n";
    }
    
    // Count orders by status
    auto all_orders = getAllOrders();
    std::unordered_map<OrderStatus, int> status_counts;
    
    for (const auto* order : all_orders) {
        status_counts[order->getStatus()]++;
    }
    
    oss << "\nOrders by Status:\n";
    for (const auto& pair : status_counts) {
        oss << "- " << orderStatusToString(pair.first) << ": " << pair.second << "\n";
    }
    
    return oss.str();
}

size_t OrderManager::getTotalOrderCount() const {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    return orders_.size();
}

int OrderManager::clearCompletedOrders() {
    std::lock_guard<std::mutex> lock(orders_mutex_);
    
    int cleared_count = 0;
    auto it = orders_.begin();
    
    while (it != orders_.end()) {
        OrderStatus status = it->second->getStatus();
        if (status == OrderStatus::DELIVERED || status == OrderStatus::CANCELLED) {
            it = orders_.erase(it);
            cleared_count++;
        } else {
            ++it;
        }
    }
    
    return cleared_count;
}

void OrderManager::updateStatistics(bool success) {
    total_orders_processed_.fetch_add(1);
    if (success) {
        successful_orders_.fetch_add(1);
    } else {
        failed_orders_.fetch_add(1);
    }
}

} // namespace quirkventory