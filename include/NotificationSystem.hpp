#pragma once

#include "Product.hpp"
#include "Order.hpp"
#include "User.hpp"
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>

namespace quirkventory {

// Forward declarations
class Inventory;
class OrderManager;

/**
 * @brief Notification priority levels
 */
enum class NotificationPriority {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

/**
 * @brief Convert NotificationPriority to string
 */
std::string priorityToString(NotificationPriority priority);

/**
 * @brief Abstract base class for all notifications
 * 
 * Demonstrates polymorphism with virtual methods for different
 * notification types and delivery mechanisms.
 */
class Notification {
protected:
    std::string message_;
    std::chrono::system_clock::time_point timestamp_;
    NotificationPriority priority_;
    std::string sender_id_;
    std::vector<std::string> recipient_ids_;

public:
    /**
     * @brief Constructor for Notification
     * @param message Notification message
     * @param priority Notification priority
     * @param sender_id ID of the sender (system or user)
     */
    Notification(const std::string& message,
                NotificationPriority priority = NotificationPriority::MEDIUM,
                const std::string& sender_id = "system");

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~Notification() = default;

    // Getters
    const std::string& getMessage() const { return message_; }
    const std::chrono::system_clock::time_point& getTimestamp() const { return timestamp_; }
    NotificationPriority getPriority() const { return priority_; }
    const std::string& getSenderId() const { return sender_id_; }
    const std::vector<std::string>& getRecipientIds() const { return recipient_ids_; }

    // Setters
    void setMessage(const std::string& message);
    void setPriority(NotificationPriority priority);
    void addRecipient(const std::string& recipient_id);
    void clearRecipients();

    /**
     * @brief Pure virtual method to send the notification
     * @return true if notification sent successfully
     * 
     * Derived classes implement specific delivery mechanisms
     */
    virtual bool send() = 0;

    /**
     * @brief Virtual method to format the notification
     * @return Formatted notification string
     * 
     * Can be overridden by derived classes for custom formatting
     */
    virtual std::string format() const;

    /**
     * @brief Get notification summary
     * @return Brief summary of the notification
     */
    virtual std::string getSummary() const;

    /**
     * @brief Check if notification is high priority
     * @return true if priority is HIGH or CRITICAL
     */
    bool isHighPriority() const;

    /**
     * @brief Get age of notification in minutes
     * @return Number of minutes since notification was created
     */
    long long getAgeInMinutes() const;
};

/**
 * @brief Email notification implementation
 * 
 * Demonstrates inheritance and polymorphic behavior for email delivery
 */
class EmailNotification : public Notification {
private:
    std::string subject_;
    std::string email_body_;
    std::vector<std::string> attachments_;

public:
    /**
     * @brief Constructor for EmailNotification
     * @param message Notification message
     * @param subject Email subject
     * @param priority Notification priority
     * @param sender_id Sender ID
     */
    EmailNotification(const std::string& message,
                     const std::string& subject,
                     NotificationPriority priority = NotificationPriority::MEDIUM,
                     const std::string& sender_id = "system");

    // Getters for email-specific attributes
    const std::string& getSubject() const { return subject_; }
    const std::string& getEmailBody() const { return email_body_; }
    const std::vector<std::string>& getAttachments() const { return attachments_; }

    // Setters for email-specific attributes
    void setSubject(const std::string& subject);
    void setEmailBody(const std::string& body);
    void addAttachment(const std::string& attachment_path);

    /**
     * @brief Override: Send email notification
     * @return true if email sent successfully
     */
    bool send() override;

    /**
     * @brief Override: Format email notification
     * @return Formatted email content
     */
    std::string format() const override;
};

/**
 * @brief System notification implementation
 * 
 * For in-system alerts and logging
 */
class SystemNotification : public Notification {
private:
    std::string category_;
    bool is_persistent_;

public:
    /**
     * @brief Constructor for SystemNotification
     * @param message Notification message
     * @param category Notification category (alert, info, warning, error)
     * @param priority Notification priority
     * @param sender_id Sender ID
     * @param persistent Whether notification should persist
     */
    SystemNotification(const std::string& message,
                      const std::string& category = "info",
                      NotificationPriority priority = NotificationPriority::MEDIUM,
                      const std::string& sender_id = "system",
                      bool persistent = false);

    // Getters for system-specific attributes
    const std::string& getCategory() const { return category_; }
    bool isPersistent() const { return is_persistent_; }

    // Setters for system-specific attributes
    void setCategory(const std::string& category);
    void setPersistent(bool persistent);

    /**
     * @brief Override: Send system notification
     * @return true if notification logged successfully
     */
    bool send() override;

    /**
     * @brief Override: Format system notification
     * @return Formatted system message
     */
    std::string format() const override;
};

/**
 * @brief Abstract base class for report generation
 * 
 * Demonstrates polymorphism with virtual methods for different
 * types of reports and output formats.
 */
class Report {
protected:
    std::string title_;
    std::chrono::system_clock::time_point generated_date_;
    std::string generated_by_;
    std::vector<std::string> sections_;

public:
    /**
     * @brief Constructor for Report
     * @param title Report title
     * @param generated_by ID of user who generated the report
     */
    Report(const std::string& title, const std::string& generated_by);

    /**
     * @brief Virtual destructor
     */
    virtual ~Report() = default;

    // Getters
    const std::string& getTitle() const { return title_; }
    const std::chrono::system_clock::time_point& getGeneratedDate() const { return generated_date_; }
    const std::string& getGeneratedBy() const { return generated_by_; }

    /**
     * @brief Pure virtual method to generate report content
     * @return Generated report as string
     */
    virtual std::string generate() = 0;

    /**
     * @brief Virtual method to export report to file
     * @param filename Output filename
     * @return true if export successful
     */
    virtual bool exportToFile(const std::string& filename) const;

    /**
     * @brief Get report header
     * @return Formatted header string
     */
    virtual std::string getHeader() const;

    /**
     * @brief Get report footer
     * @return Formatted footer string
     */
    virtual std::string getFooter() const;

protected:
    /**
     * @brief Add a section to the report
     * @param section_content Content to add
     */
    void addSection(const std::string& section_content);

    /**
     * @brief Clear all sections
     */
    void clearSections();
};

/**
 * @brief Sales report implementation
 */
class SalesReport : public Report {
private:
    const OrderManager* order_manager_;
    std::chrono::system_clock::time_point start_date_;
    std::chrono::system_clock::time_point end_date_;

public:
    /**
     * @brief Constructor for SalesReport
     * @param order_manager Pointer to order manager
     * @param start_date Report start date
     * @param end_date Report end date
     * @param generated_by User who generated the report
     */
    SalesReport(const OrderManager* order_manager,
               const std::chrono::system_clock::time_point& start_date,
               const std::chrono::system_clock::time_point& end_date,
               const std::string& generated_by);

    /**
     * @brief Override: Generate sales report
     * @return Formatted sales report
     */
    std::string generate() override;

private:
    /**
     * @brief Generate order summary section
     * @return Order summary content
     */
    std::string generateOrderSummary() const;

    /**
     * @brief Generate revenue analysis section
     * @return Revenue analysis content
     */
    std::string generateRevenueAnalysis() const;

    /**
     * @brief Generate customer analysis section
     * @return Customer analysis content
     */
    std::string generateCustomerAnalysis() const;
};

/**
 * @brief Inventory report implementation
 */
class InventoryReport : public Report {
private:
    const Inventory* inventory_;
    bool include_low_stock_;
    bool include_expired_;

public:
    /**
     * @brief Constructor for InventoryReport
     * @param inventory Pointer to inventory system
     * @param generated_by User who generated the report
     * @param include_low_stock Include low stock analysis
     * @param include_expired Include expired products analysis
     */
    InventoryReport(const Inventory* inventory,
                   const std::string& generated_by,
                   bool include_low_stock = true,
                   bool include_expired = true);

    /**
     * @brief Override: Generate inventory report
     * @return Formatted inventory report
     */
    std::string generate() override;

private:
    /**
     * @brief Generate inventory overview section
     * @return Inventory overview content
     */
    std::string generateInventoryOverview() const;

    /**
     * @brief Generate category breakdown section
     * @return Category breakdown content
     */
    std::string generateCategoryBreakdown() const;

    /**
     * @brief Generate low stock section
     * @return Low stock analysis content
     */
    std::string generateLowStockSection() const;

    /**
     * @brief Generate expired products section
     * @return Expired products analysis content
     */
    std::string generateExpiredSection() const;
};

/**
 * @brief Notification and reporting management system
 * 
 * Centralized system for managing notifications and generating reports
 */
class NotificationManager {
private:
    std::vector<std::unique_ptr<Notification>> notification_history_;
    std::vector<std::function<void(const Notification&)>> notification_callbacks_;
    size_t max_history_size_;

public:
    /**
     * @brief Constructor
     * @param max_history Maximum number of notifications to keep in history
     */
    explicit NotificationManager(size_t max_history = 1000);

    /**
     * @brief Destructor
     */
    ~NotificationManager() = default;

    // Disable copy constructor and assignment operator
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;

    /**
     * @brief Send an email notification
     * @param message Notification message
     * @param subject Email subject
     * @param recipients List of recipient IDs
     * @param priority Notification priority
     * @return true if notification sent successfully
     */
    bool sendEmailNotification(const std::string& message,
                              const std::string& subject,
                              const std::vector<std::string>& recipients,
                              NotificationPriority priority = NotificationPriority::MEDIUM);

    /**
     * @brief Send a system notification
     * @param message Notification message
     * @param category Notification category
     * @param recipients List of recipient IDs
     * @param priority Notification priority
     * @return true if notification sent successfully
     */
    bool sendSystemNotification(const std::string& message,
                               const std::string& category,
                               const std::vector<std::string>& recipients,
                               NotificationPriority priority = NotificationPriority::MEDIUM);

    /**
     * @brief Register a callback for notifications
     * @param callback Function to call when notifications are sent
     */
    void registerNotificationCallback(std::function<void(const Notification&)> callback);

    /**
     * @brief Get notification history
     * @param limit Maximum number of notifications to return (0 = all)
     * @return Vector of notifications
     */
    std::vector<const Notification*> getNotificationHistory(size_t limit = 0) const;

    /**
     * @brief Get high priority notifications
     * @return Vector of high priority notifications
     */
    std::vector<const Notification*> getHighPriorityNotifications() const;

    /**
     * @brief Clear notification history
     */
    void clearHistory();

    /**
     * @brief Generate sales report
     * @param order_manager Order manager instance
     * @param start_date Report start date
     * @param end_date Report end date
     * @param generated_by User generating the report
     * @return Generated sales report
     */
    std::unique_ptr<SalesReport> generateSalesReport(const OrderManager& order_manager,
                                                    const std::chrono::system_clock::time_point& start_date,
                                                    const std::chrono::system_clock::time_point& end_date,
                                                    const std::string& generated_by) const;

    /**
     * @brief Generate inventory report
     * @param inventory Inventory instance
     * @param generated_by User generating the report
     * @param include_low_stock Include low stock analysis
     * @param include_expired Include expired products analysis
     * @return Generated inventory report
     */
    std::unique_ptr<InventoryReport> generateInventoryReport(const Inventory& inventory,
                                                           const std::string& generated_by,
                                                           bool include_low_stock = true,
                                                           bool include_expired = true) const;

    /**
     * @brief Send automated inventory alerts
     * @param inventory Inventory to check
     */
    void sendInventoryAlerts(const Inventory& inventory);

    /**
     * @brief Get notification statistics
     * @return Formatted statistics string
     */
    std::string getNotificationStatistics() const;

private:
    /**
     * @brief Add notification to history
     * @param notification Notification to add
     */
    void addToHistory(std::unique_ptr<Notification> notification);

    /**
     * @brief Notify all registered callbacks
     * @param notification Notification to broadcast
     */
    void notifyCallbacks(const Notification& notification);
};

} // namespace quirkventory