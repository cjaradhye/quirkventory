#pragma once

#include "Product.hpp"
#include "Inventory.hpp"
#include "Order.hpp"
#include "User.hpp"
#include "NotificationSystem.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cfloat>
#include <functional>

namespace quirkventory {

/**
 * @brief Command structure for CLI operations
 */
struct CLICommand {
    std::string name;
    std::string description;
    std::vector<std::string> required_permissions;
    std::function<void()> handler;
    
    CLICommand(const std::string& cmd_name, 
               const std::string& cmd_description,
               const std::vector<std::string>& permissions,
               std::function<void()> cmd_handler)
        : name(cmd_name), description(cmd_description), 
          required_permissions(permissions), handler(cmd_handler) {}
};

/**
 * @brief Command-line interface for the inventory management system
 * 
 * Provides an interactive CLI for staff operations including product management,
 * order processing, and report generation with role-based access control.
 */
class CLI {
private:
    // System components
    std::unique_ptr<Inventory> inventory_;
    std::unique_ptr<OrderManager> order_manager_;
    std::unique_ptr<UserManager> user_manager_;
    std::unique_ptr<NotificationManager> notification_manager_;
    
    // CLI state
    bool running_;
    User* current_user_;
    std::vector<CLICommand> commands_;
    
    // Input/output streams
    std::istream& input_stream_;
    std::ostream& output_stream_;

public:
    /**
     * @brief Constructor
     * @param input Input stream (default: std::cin)
     * @param output Output stream (default: std::cout)
     */
    CLI(std::istream& input = std::cin, std::ostream& output = std::cout);

    /**
     * @brief Destructor
     */
    ~CLI() = default;

    // Disable copy constructor and assignment operator
    CLI(const CLI&) = delete;
    CLI& operator=(const CLI&) = delete;

    /**
     * @brief Initialize the CLI system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Run the main CLI loop
     */
    void run();

    /**
     * @brief Stop the CLI
     */
    void stop();

    /**
     * @brief Display welcome message and login prompt
     */
    void displayWelcome();

    /**
     * @brief Handle user authentication
     * @return true if login successful
     */
    bool handleLogin();

    /**
     * @brief Display main menu
     */
    void displayMainMenu();

    /**
     * @brief Process user command input
     * @param command Command string
     * @return true if command processed successfully
     */
    bool processCommand(const std::string& command);

private:
    // System initialization
    void setupCommands();
    void createDefaultUsers();
    void loadSampleData();

    // Utility methods
    std::string readLine();
    std::string readPassword();
    void clearScreen();
    void pauseForInput();
    bool confirmAction(const std::string& message);
    void displayError(const std::string& error);
    void displaySuccess(const std::string& message);
    void displayInfo(const std::string& info);

    // Permission checking
    bool hasPermission(const std::vector<std::string>& required_permissions);
    bool checkCurrentUserPermission(Permission permission);

    // Command handlers - Product Management
    void handleAddProduct();
    void handleViewProducts();
    void handleSearchProducts();
    void handleUpdateProduct();
    void handleRemoveProduct();

    // Command handlers - Inventory Management
    void handleViewInventory();
    void handleUpdateStock();
    void handleLowStockReport();
    void handleExpiryReport();
    void handleInventoryReport();

    // Command handlers - Order Management
    void handleCreateOrder();
    void handleViewOrders();
    void handleProcessOrders();
    void handleOrderStatus();
    void handleCancelOrder();

    // Command handlers - User Management
    void handleViewProfile();
    void handleChangePassword();
    void handleViewUsers();
    void handleCreateUser();

    // Command handlers - Reports
    void handleSalesReport();
    void handleNotificationHistory();
    void handleSystemStatus();

    // Command handlers - System
    void handleHelp();
    void handleLogout();
    void handleExit();

    // Helper methods for data input
    std::string getStringInput(const std::string& prompt, bool required = true);
    int getIntInput(const std::string& prompt, int min_value = 0, int max_value = INT_MAX);
    double getDoubleInput(const std::string& prompt, double min_value = 0.0, double max_value = DBL_MAX);
    bool getBoolInput(const std::string& prompt);
    std::chrono::system_clock::time_point getDateInput(const std::string& prompt);

    // Data display helpers
    void displayProductList(const std::vector<const Product*>& products);
    void displayOrderList(const std::vector<Order*>& orders);
    void displayUserList(const std::vector<User*>& users);
    void displayNotificationList(const std::vector<const Notification*>& notifications);

    // Table formatting helpers
    void printTableHeader(const std::vector<std::string>& headers, const std::vector<int>& widths);
    void printTableRow(const std::vector<std::string>& values, const std::vector<int>& widths);
    void printTableSeparator(const std::vector<int>& widths);

    // Menu display helpers
    void displayMenu(const std::string& title, const std::vector<std::string>& options);
    int getMenuChoice(int max_options);
};

/**
 * @brief CLI utility functions
 */
namespace CLIUtils {
    /**
     * @brief Format currency value
     * @param amount Amount to format
     * @return Formatted currency string
     */
    std::string formatCurrency(double amount);

    /**
     * @brief Format date/time
     * @param time_point Time point to format
     * @return Formatted date/time string
     */
    std::string formatDateTime(const std::chrono::system_clock::time_point& time_point);

    /**
     * @brief Format duration
     * @param start Start time
     * @param end End time
     * @return Formatted duration string
     */
    std::string formatDuration(const std::chrono::system_clock::time_point& start,
                              const std::chrono::system_clock::time_point& end);

    /**
     * @brief Truncate string to specified length
     * @param str String to truncate
     * @param length Maximum length
     * @return Truncated string
     */
    std::string truncateString(const std::string& str, size_t length);

    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    std::string trim(const std::string& str);

    /**
     * @brief Convert string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    std::string toLowerCase(const std::string& str);

    /**
     * @brief Split string by delimiter
     * @param str String to split
     * @param delimiter Delimiter character
     * @return Vector of split strings
     */
    std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Generate unique ID
     * @param prefix ID prefix
     * @return Unique ID string
     */
    std::string generateId(const std::string& prefix = "");

    /**
     * @brief Parse date string
     * @param date_str Date string in YYYY-MM-DD format
     * @return Time point representing the date
     */
    std::chrono::system_clock::time_point parseDate(const std::string& date_str);

    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return true if valid email format
     */
    bool isValidEmail(const std::string& email);

    /**
     * @brief Get terminal width
     * @return Terminal width in characters
     */
    int getTerminalWidth();

    /**
     * @brief Center text in terminal
     * @param text Text to center
     * @param width Total width (0 = use terminal width)
     * @return Centered text
     */
    std::string centerText(const std::string& text, int width = 0);

    /**
     * @brief Create progress bar
     * @param current Current value
     * @param total Total value
     * @param width Bar width
     * @return Progress bar string
     */
    std::string createProgressBar(int current, int total, int width = 40);
}

} // namespace quirkventory