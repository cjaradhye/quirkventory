#include "../include/CLI.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <regex>
#include <random>
#include <chrono>
#include <climits>
#include <cfloat>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace quirkventory {

// CLI Implementation

CLI::CLI(std::istream& input, std::ostream& output)
    : running_(false), current_user_(nullptr), 
      input_stream_(input), output_stream_(output) {
}

bool CLI::initialize() {
    try {
        // Initialize system components
        inventory_ = std::make_unique<Inventory>(5); // Low stock threshold of 5
        order_manager_ = std::make_unique<OrderManager>();
        user_manager_ = std::make_unique<UserManager>();
        notification_manager_ = std::make_unique<NotificationManager>();

        // Setup CLI commands
        setupCommands();

        // Create default users
        createDefaultUsers();

        // Load sample data
        loadSampleData();

        output_stream_ << "System initialized successfully!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        output_stream_ << "Failed to initialize system: " << e.what() << std::endl;
        return false;
    }
}

void CLI::run() {
    if (!initialize()) {
        return;
    }

    running_ = true;
    displayWelcome();

    while (running_) {
        if (!current_user_) {
            if (!handleLogin()) {
                break;
            }
        }

        displayMainMenu();
        std::string command = readLine();
        
        if (!processCommand(command)) {
            displayError("Unknown command. Type 'help' for available commands.");
        }
    }

    output_stream_ << "Thank you for using the Inventory Management System!" << std::endl;
}

void CLI::stop() {
    running_ = false;
}

void CLI::displayWelcome() {
    clearScreen();
    output_stream_ << CLIUtils::centerText("==============================================") << std::endl;
    output_stream_ << CLIUtils::centerText("    QUIRKVENTORY MANAGEMENT SYSTEM") << std::endl;
    output_stream_ << CLIUtils::centerText("        Retail Inventory Solution") << std::endl;
    output_stream_ << CLIUtils::centerText("==============================================") << std::endl;
    output_stream_ << std::endl;
}

bool CLI::handleLogin() {
    output_stream_ << "=== LOGIN ===" << std::endl;
    
    std::string username = getStringInput("Username: ");
    std::string password = readPassword();
    
    User* user = user_manager_->authenticateUser(username, password);
    if (user) {
        current_user_ = user;
        output_stream_ << std::endl << "Welcome, " << user->getFullName() 
                      << " (" << user->getRole() << ")!" << std::endl;
        pauseForInput();
        return true;
    } else {
        displayError("Invalid username or password.");
        pauseForInput();
        return false;
    }
}

void CLI::displayMainMenu() {
    clearScreen();
    if (current_user_) {
        output_stream_ << "Logged in as: " << current_user_->getFullName() 
                      << " (" << current_user_->getRole() << ")" << std::endl;
        output_stream_ << "===========================================" << std::endl;
    }
    
    output_stream_ << "Available Commands:" << std::endl;
    output_stream_ << "------------------" << std::endl;
    
    for (const auto& cmd : commands_) {
        if (hasPermission(cmd.required_permissions)) {
            output_stream_ << "  " << std::left << std::setw(15) << cmd.name 
                          << " - " << cmd.description << std::endl;
        }
    }
    
    output_stream_ << std::endl << "Enter command: ";
}

bool CLI::processCommand(const std::string& command) {
    std::string cmd = CLIUtils::toLowerCase(CLIUtils::trim(command));
    
    for (const auto& cli_cmd : commands_) {
        if (CLIUtils::toLowerCase(cli_cmd.name) == cmd) {
            if (hasPermission(cli_cmd.required_permissions)) {
                try {
                    cli_cmd.handler();
                    return true;
                } catch (const std::exception& e) {
                    displayError(std::string("Error executing command: ") + e.what());
                    pauseForInput();
                    return true;
                }
            } else {
                displayError("You don't have permission to execute this command.");
                pauseForInput();
                return true;
            }
        }
    }
    
    return false;
}

void CLI::setupCommands() {
    commands_.clear();
    
    // Product Management Commands
    commands_.emplace_back("add-product", "Add a new product", 
        std::vector<std::string>{"ADD_PRODUCTS"}, 
        [this]() { handleAddProduct(); });
    
    commands_.emplace_back("view-products", "View all products", 
        std::vector<std::string>{"VIEW_PRODUCTS"}, 
        [this]() { handleViewProducts(); });
    
    commands_.emplace_back("search-products", "Search products", 
        std::vector<std::string>{"VIEW_PRODUCTS"}, 
        [this]() { handleSearchProducts(); });
    
    commands_.emplace_back("update-product", "Update product information", 
        std::vector<std::string>{"MODIFY_PRODUCTS"}, 
        [this]() { handleUpdateProduct(); });
    
    commands_.emplace_back("remove-product", "Remove a product", 
        std::vector<std::string>{"DELETE_PRODUCTS"}, 
        [this]() { handleRemoveProduct(); });
    
    // Inventory Management Commands
    commands_.emplace_back("view-inventory", "View inventory status", 
        std::vector<std::string>{"VIEW_INVENTORY"}, 
        [this]() { handleViewInventory(); });
    
    commands_.emplace_back("update-stock", "Update product stock", 
        std::vector<std::string>{"MODIFY_INVENTORY"}, 
        [this]() { handleUpdateStock(); });
    
    commands_.emplace_back("low-stock", "View low stock report", 
        std::vector<std::string>{"VIEW_REPORTS"}, 
        [this]() { handleLowStockReport(); });
    
    commands_.emplace_back("expiry-report", "View product expiry report", 
        std::vector<std::string>{"VIEW_REPORTS"}, 
        [this]() { handleExpiryReport(); });
    
    commands_.emplace_back("inventory-report", "Generate inventory report", 
        std::vector<std::string>{"GENERATE_REPORTS"}, 
        [this]() { handleInventoryReport(); });
    
    // Order Management Commands
    commands_.emplace_back("create-order", "Create a new order", 
        std::vector<std::string>{"CREATE_ORDERS"}, 
        [this]() { handleCreateOrder(); });
    
    commands_.emplace_back("view-orders", "View orders", 
        std::vector<std::string>{"VIEW_ORDERS"}, 
        [this]() { handleViewOrders(); });
    
    commands_.emplace_back("process-orders", "Process pending orders", 
        std::vector<std::string>{"MODIFY_ORDERS"}, 
        [this]() { handleProcessOrders(); });
    
    commands_.emplace_back("order-status", "Check order status", 
        std::vector<std::string>{"VIEW_ORDERS"}, 
        [this]() { handleOrderStatus(); });
    
    commands_.emplace_back("cancel-order", "Cancel an order", 
        std::vector<std::string>{"CANCEL_ORDERS"}, 
        [this]() { handleCancelOrder(); });
    
    // User Management Commands
    commands_.emplace_back("profile", "View user profile", 
        std::vector<std::string>{}, 
        [this]() { handleViewProfile(); });
    
    commands_.emplace_back("change-password", "Change password", 
        std::vector<std::string>{}, 
        [this]() { handleChangePassword(); });
    
    commands_.emplace_back("view-users", "View all users", 
        std::vector<std::string>{"MANAGE_USERS"}, 
        [this]() { handleViewUsers(); });
    
    commands_.emplace_back("create-user", "Create new user", 
        std::vector<std::string>{"MANAGE_USERS"}, 
        [this]() { handleCreateUser(); });
    
    // Reports Commands
    commands_.emplace_back("sales-report", "Generate sales report", 
        std::vector<std::string>{"GENERATE_REPORTS"}, 
        [this]() { handleSalesReport(); });
    
    commands_.emplace_back("notifications", "View notification history", 
        std::vector<std::string>{"VIEW_REPORTS"}, 
        [this]() { handleNotificationHistory(); });
    
    commands_.emplace_back("system-status", "View system status", 
        std::vector<std::string>{"VIEW_REPORTS"}, 
        [this]() { handleSystemStatus(); });
    
    // System Commands
    commands_.emplace_back("help", "Show this help message", 
        std::vector<std::string>{}, 
        [this]() { handleHelp(); });
    
    commands_.emplace_back("logout", "Logout current user", 
        std::vector<std::string>{}, 
        [this]() { handleLogout(); });
    
    commands_.emplace_back("exit", "Exit the application", 
        std::vector<std::string>{}, 
        [this]() { handleExit(); });
}

void CLI::createDefaultUsers() {
    // Create default manager
    user_manager_->createManager("manager1", "admin", "admin123", 
                                "admin@quirkventory.com", "System Administrator", 
                                "Management", 50000.0);
    
    // Create default staff
    user_manager_->createStaff("staff1", "staff", "staff123", 
                              "staff@quirkventory.com", "Jane Smith", 
                              "Sales", "Day", "manager1");
}

void CLI::loadSampleData() {
    // Add sample products
    auto product1 = std::make_unique<Product>("P001", "Laptop Computer", "Electronics", 999.99, 10);
    auto product2 = std::make_unique<Product>("P002", "Office Chair", "Furniture", 199.99, 25);
    
    // Add perishable product
    auto future_date = std::chrono::system_clock::now() + std::chrono::hours(24 * 30); // 30 days from now
    auto perishable = std::make_unique<PerishableProduct>("P003", "Fresh Milk", "Dairy", 3.99, 50, 
                                                         future_date, "Keep refrigerated", 4.0);
    
    inventory_->addProduct(std::move(product1));
    inventory_->addProduct(std::move(product2));
    inventory_->addProduct(std::move(perishable));
}

// Utility methods implementation

std::string CLI::readLine() {
    std::string line;
    std::getline(input_stream_, line);
    return CLIUtils::trim(line);
}

std::string CLI::readPassword() {
    output_stream_ << "Password: ";
    std::string password;
    
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                output_stream_ << "\b \b";
            }
        } else {
            password += ch;
            output_stream_ << '*';
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    std::getline(input_stream_, password);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    
    output_stream_ << std::endl;
    return password;
}

void CLI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void CLI::pauseForInput() {
    output_stream_ << "Press Enter to continue...";
    input_stream_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool CLI::confirmAction(const std::string& message) {
    output_stream_ << message << " (y/N): ";
    std::string response = readLine();
    return CLIUtils::toLowerCase(response) == "y" || CLIUtils::toLowerCase(response) == "yes";
}

void CLI::displayError(const std::string& error) {
    output_stream_ << "ERROR: " << error << std::endl;
}

void CLI::displaySuccess(const std::string& message) {
    output_stream_ << "SUCCESS: " << message << std::endl;
}

void CLI::displayInfo(const std::string& info) {
    output_stream_ << "INFO: " << info << std::endl;
}

bool CLI::hasPermission(const std::vector<std::string>& required_permissions) {
    if (!current_user_) {
        return false;
    }
    
    if (required_permissions.empty()) {
        return true; // No specific permissions required
    }
    
    for (const auto& permission_str : required_permissions) {
        // Convert string to Permission enum and check
        if (permission_str == "VIEW_PRODUCTS" && !current_user_->hasPermission(Permission::VIEW_PRODUCTS)) return false;
        if (permission_str == "ADD_PRODUCTS" && !current_user_->hasPermission(Permission::ADD_PRODUCTS)) return false;
        if (permission_str == "MODIFY_PRODUCTS" && !current_user_->hasPermission(Permission::MODIFY_PRODUCTS)) return false;
        if (permission_str == "DELETE_PRODUCTS" && !current_user_->hasPermission(Permission::DELETE_PRODUCTS)) return false;
        if (permission_str == "VIEW_INVENTORY" && !current_user_->hasPermission(Permission::VIEW_INVENTORY)) return false;
        if (permission_str == "MODIFY_INVENTORY" && !current_user_->hasPermission(Permission::MODIFY_INVENTORY)) return false;
        if (permission_str == "VIEW_ORDERS" && !current_user_->hasPermission(Permission::VIEW_ORDERS)) return false;
        if (permission_str == "CREATE_ORDERS" && !current_user_->hasPermission(Permission::CREATE_ORDERS)) return false;
        if (permission_str == "MODIFY_ORDERS" && !current_user_->hasPermission(Permission::MODIFY_ORDERS)) return false;
        if (permission_str == "CANCEL_ORDERS" && !current_user_->hasPermission(Permission::CANCEL_ORDERS)) return false;
        if (permission_str == "VIEW_REPORTS" && !current_user_->hasPermission(Permission::VIEW_REPORTS)) return false;
        if (permission_str == "GENERATE_REPORTS" && !current_user_->hasPermission(Permission::GENERATE_REPORTS)) return false;
        if (permission_str == "MANAGE_USERS" && !current_user_->hasPermission(Permission::MANAGE_USERS)) return false;
    }
    
    return true;
}

bool CLI::checkCurrentUserPermission(Permission permission) {
    return current_user_ && current_user_->hasPermission(permission);
}

// Command handlers implementation

void CLI::handleAddProduct() {
    clearScreen();
    output_stream_ << "=== ADD NEW PRODUCT ===" << std::endl;
    
    std::string id = getStringInput("Product ID: ");
    if (inventory_->hasProduct(id)) {
        displayError("Product ID already exists.");
        pauseForInput();
        return;
    }
    
    std::string name = getStringInput("Product Name: ");
    std::string category = getStringInput("Category: ");
    double price = getDoubleInput("Price: $", 0.0);
    int quantity = getIntInput("Initial Quantity: ", 0);
    
    bool is_perishable = getBoolInput("Is this a perishable product? (y/n): ");
    
    std::unique_ptr<Product> product;
    
    if (is_perishable) {
        auto expiry_date = getDateInput("Expiry Date (YYYY-MM-DD): ");
        std::string storage_requirements = getStringInput("Storage Requirements: ", false);
        double storage_temp = getDoubleInput("Storage Temperature (°C): ", -50.0, 100.0);
        
        product = std::make_unique<PerishableProduct>(id, name, category, price, quantity,
                                                     expiry_date, storage_requirements, storage_temp);
    } else {
        product = std::make_unique<Product>(id, name, category, price, quantity);
    }
    
    if (inventory_->addProduct(std::move(product))) {
        displaySuccess("Product added successfully.");
    } else {
        displayError("Failed to add product.");
    }
    
    pauseForInput();
}

void CLI::handleViewProducts() {
    clearScreen();
    output_stream_ << "=== PRODUCT LIST ===" << std::endl;
    
    auto products = inventory_->getAllProducts();
    if (products.empty()) {
        displayInfo("No products found.");
    } else {
        displayProductList(products);
    }
    
    pauseForInput();
}

void CLI::handleViewInventory() {
    clearScreen();
    output_stream_ << "=== INVENTORY STATUS ===" << std::endl;
    
    output_stream_ << inventory_->generateInventoryReport() << std::endl;
    
    pauseForInput();
}

void CLI::handleCreateOrder() {
    clearScreen();
    output_stream_ << "=== CREATE NEW ORDER ===" << std::endl;
    
    std::string order_id = CLIUtils::generateId("ORD");
    std::string customer_id = getStringInput("Customer ID: ");
    
    Order* order = order_manager_->createOrder(order_id, customer_id);
    if (!order) {
        displayError("Failed to create order.");
        pauseForInput();
        return;
    }
    
    output_stream_ << "Order " << order_id << " created. Now add items:" << std::endl;
    
    while (true) {
        std::string product_id = getStringInput("Product ID (or 'done' to finish): ");
        if (CLIUtils::toLowerCase(product_id) == "done") {
            break;
        }
        
        const Product* product = inventory_->getProduct(product_id);
        if (!product) {
            displayError("Product not found.");
            continue;
        }
        
        output_stream_ << "Product: " << product->getName() 
                      << " (Available: " << product->getQuantity() 
                      << ", Price: $" << product->getPrice() << ")" << std::endl;
        
        int quantity = getIntInput("Quantity: ", 1, product->getQuantity());
        
        if (order->addItem(product_id, quantity, product->getPrice())) {
            displaySuccess("Item added to order.");
        } else {
            displayError("Failed to add item to order.");
        }
    }
    
    if (order->getItems().empty()) {
        displayError("Order has no items. Removing order.");
        order_manager_->removeOrder(order_id);
    } else {
        output_stream_ << std::endl << order->getDetailedInfo() << std::endl;
        displaySuccess("Order created successfully.");
    }
    
    pauseForInput();
}

void CLI::handleViewOrders() {
    clearScreen();
    output_stream_ << "=== ORDER LIST ===" << std::endl;
    
    auto orders = order_manager_->getAllOrders();
    if (orders.empty()) {
        displayInfo("No orders found.");
    } else {
        displayOrderList(orders);
    }
    
    pauseForInput();
}

void CLI::handleViewProfile() {
    clearScreen();
    output_stream_ << "=== USER PROFILE ===" << std::endl;
    
    if (current_user_) {
        output_stream_ << current_user_->getUserInfo() << std::endl;
    } else {
        displayError("No user logged in.");
    }
    
    pauseForInput();
}

void CLI::handleHelp() {
    clearScreen();
    output_stream_ << "=== HELP ===" << std::endl;
    output_stream_ << "Available commands and their descriptions:" << std::endl << std::endl;
    
    for (const auto& cmd : commands_) {
        if (hasPermission(cmd.required_permissions)) {
            output_stream_ << std::left << std::setw(20) << cmd.name 
                          << " - " << cmd.description << std::endl;
        }
    }
    
    output_stream_ << std::endl;
    output_stream_ << "Note: Available commands depend on your user role and permissions." << std::endl;
    
    pauseForInput();
}

void CLI::handleLogout() {
    if (current_user_) {
        output_stream_ << "Logging out " << current_user_->getFullName() << "..." << std::endl;
        user_manager_->logout();
        current_user_ = nullptr;
        displaySuccess("Logged out successfully.");
    } else {
        displayError("No user logged in.");
    }
    pauseForInput();
}

void CLI::handleExit() {
    if (confirmAction("Are you sure you want to exit?")) {
        stop();
    }
}

// Additional command handlers would be implemented here...
// For brevity, I'm showing the structure and key methods

// Helper methods for input
std::string CLI::getStringInput(const std::string& prompt, bool required) {
    std::string input;
    do {
        output_stream_ << prompt;
        input = readLine();
        if (!required && input.empty()) {
            break;
        }
        if (required && input.empty()) {
            displayError("This field is required.");
        }
    } while (required && input.empty());
    
    return input;
}

int CLI::getIntInput(const std::string& prompt, int min_value, int max_value) {
    int value;
    do {
        output_stream_ << prompt;
        std::string input = readLine();
        try {
            value = std::stoi(input);
            if (value < min_value || value > max_value) {
                displayError("Value must be between " + std::to_string(min_value) + 
                           " and " + std::to_string(max_value) + ".");
                continue;
            }
            break;
        } catch (const std::exception&) {
            displayError("Please enter a valid number.");
        }
    } while (true);
    
    return value;
}

double CLI::getDoubleInput(const std::string& prompt, double min_value, double max_value) {
    double value;
    do {
        output_stream_ << prompt;
        std::string input = readLine();
        try {
            value = std::stod(input);
            if (value < min_value || value > max_value) {
                displayError("Value must be between " + std::to_string(min_value) + 
                           " and " + std::to_string(max_value) + ".");
                continue;
            }
            break;
        } catch (const std::exception&) {
            displayError("Please enter a valid number.");
        }
    } while (true);
    
    return value;
}

bool CLI::getBoolInput(const std::string& prompt) {
    output_stream_ << prompt;
    std::string input = CLIUtils::toLowerCase(readLine());
    return input == "y" || input == "yes" || input == "true" || input == "1";
}

std::chrono::system_clock::time_point CLI::getDateInput(const std::string& prompt) {
    do {
        output_stream_ << prompt;
        std::string input = readLine();
        try {
            return CLIUtils::parseDate(input);
        } catch (const std::exception&) {
            displayError("Please enter a valid date in YYYY-MM-DD format.");
        }
    } while (true);
}

void CLI::displayProductList(const std::vector<const Product*>& products) {
    std::vector<std::string> headers = {"ID", "Name", "Category", "Price", "Quantity", "Status"};
    std::vector<int> widths = {10, 20, 15, 10, 10, 15};
    
    printTableHeader(headers, widths);
    
    for (const auto* product : products) {
        std::vector<std::string> values = {
            product->getId(),
            CLIUtils::truncateString(product->getName(), 18),
            CLIUtils::truncateString(product->getCategory(), 13),
            CLIUtils::formatCurrency(product->getPrice()),
            std::to_string(product->getQuantity()),
            product->isExpired() ? "EXPIRED" : (product->getQuantity() < 5 ? "LOW STOCK" : "OK")
        };
        printTableRow(values, widths);
    }
    
    printTableSeparator(widths);
    output_stream_ << "Total Products: " << products.size() << std::endl;
}

void CLI::printTableHeader(const std::vector<std::string>& headers, const std::vector<int>& widths) {
    printTableSeparator(widths);
    output_stream_ << "|";
    for (size_t i = 0; i < headers.size() && i < widths.size(); ++i) {
        output_stream_ << " " << std::left << std::setw(widths[i] - 1) << headers[i] << "|";
    }
    output_stream_ << std::endl;
    printTableSeparator(widths);
}

void CLI::printTableRow(const std::vector<std::string>& values, const std::vector<int>& widths) {
    output_stream_ << "|";
    for (size_t i = 0; i < values.size() && i < widths.size(); ++i) {
        output_stream_ << " " << std::left << std::setw(widths[i] - 1) << values[i] << "|";
    }
    output_stream_ << std::endl;
}

void CLI::printTableSeparator(const std::vector<int>& widths) {
    output_stream_ << "+";
    for (int width : widths) {
        output_stream_ << std::string(width, '-') << "+";
    }
    output_stream_ << std::endl;
}

// CLIUtils Implementation

namespace CLIUtils {

std::string formatCurrency(double amount) {
    std::ostringstream oss;
    oss << "$" << std::fixed << std::setprecision(2) << amount;
    return oss.str();
}

std::string formatDateTime(const std::chrono::system_clock::time_point& time_point) {
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string truncateString(const std::string& str, size_t length) {
    if (str.length() <= length) {
        return str;
    }
    return str.substr(0, length - 3) + "...";
}

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string generateId(const std::string& prefix) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    return prefix + std::to_string(timestamp % 100000) + std::to_string(dis(gen));
}

std::chrono::system_clock::time_point parseDate(const std::string& date_str) {
    std::regex date_regex(R"((\d{4})-(\d{2})-(\d{2}))");
    std::smatch match;
    
    if (!std::regex_match(date_str, match, date_regex)) {
        throw std::invalid_argument("Invalid date format");
    }
    
    int year = std::stoi(match[1]);
    int month = std::stoi(match[2]);
    int day = std::stoi(match[3]);
    
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

std::string centerText(const std::string& text, int width) {
    if (width == 0) {
        width = getTerminalWidth();
    }
    
    if (static_cast<int>(text.length()) >= width) {
        return text;
    }
    
    int padding = (width - static_cast<int>(text.length())) / 2;
    return std::string(padding, ' ') + text;
}

} // namespace CLIUtils

} // namespace quirkventory