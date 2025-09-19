#include "../include/User.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <functional>

namespace quirkventory {

// Helper function implementations

std::string permissionToString(Permission permission) {
    switch (permission) {
        case Permission::VIEW_PRODUCTS: return "VIEW_PRODUCTS";
        case Permission::ADD_PRODUCTS: return "ADD_PRODUCTS";
        case Permission::MODIFY_PRODUCTS: return "MODIFY_PRODUCTS";
        case Permission::DELETE_PRODUCTS: return "DELETE_PRODUCTS";
        case Permission::VIEW_INVENTORY: return "VIEW_INVENTORY";
        case Permission::MODIFY_INVENTORY: return "MODIFY_INVENTORY";
        case Permission::VIEW_ORDERS: return "VIEW_ORDERS";
        case Permission::CREATE_ORDERS: return "CREATE_ORDERS";
        case Permission::MODIFY_ORDERS: return "MODIFY_ORDERS";
        case Permission::CANCEL_ORDERS: return "CANCEL_ORDERS";
        case Permission::VIEW_REPORTS: return "VIEW_REPORTS";
        case Permission::GENERATE_REPORTS: return "GENERATE_REPORTS";
        case Permission::MANAGE_USERS: return "MANAGE_USERS";
        case Permission::SYSTEM_ADMIN: return "SYSTEM_ADMIN";
        default: return "UNKNOWN";
    }
}

// User Implementation

User::User(const std::string& user_id,
           const std::string& username,
           const std::string& password_hash,
           const std::string& email,
           const std::string& full_name)
    : user_id_(user_id), username_(username), password_hash_(password_hash),
      email_(email), full_name_(full_name),
      created_date_(std::chrono::system_clock::now()),
      is_active_(true) {
    
    if (user_id.empty()) {
        throw std::invalid_argument("User ID cannot be empty");
    }
    if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty");
    }
    if (password_hash.empty()) {
        throw std::invalid_argument("Password hash cannot be empty");
    }
    if (email.empty()) {
        throw std::invalid_argument("Email cannot be empty");
    }
    if (full_name.empty()) {
        throw std::invalid_argument("Full name cannot be empty");
    }
}

void User::setUsername(const std::string& username) {
    if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty");
    }
    username_ = username;
}

void User::setEmail(const std::string& email) {
    if (email.empty()) {
        throw std::invalid_argument("Email cannot be empty");
    }
    email_ = email;
}

void User::setFullName(const std::string& full_name) {
    if (full_name.empty()) {
        throw std::invalid_argument("Full name cannot be empty");
    }
    full_name_ = full_name;
}

void User::setPasswordHash(const std::string& password_hash) {
    if (password_hash.empty()) {
        throw std::invalid_argument("Password hash cannot be empty");
    }
    password_hash_ = password_hash;
}

void User::setActive(bool active) {
    is_active_ = active;
}

std::unordered_set<Permission> User::getPermissions() const {
    return permissions_;
}

bool User::hasPermission(Permission permission) const {
    return permissions_.find(permission) != permissions_.end();
}

bool User::canModify(const std::string& resource_type) const {
    // Base implementation - derived classes should override
    return false;
}

bool User::authenticate(const std::string& password) const {
    if (!is_active_) {
        return false;
    }
    return verifyPassword(password, password_hash_);
}

void User::updateLastLogin() {
    last_login_ = std::chrono::system_clock::now();
}

std::string User::getUserInfo() const {
    std::ostringstream oss;
    
    oss << "User ID: " << user_id_ << "\n";
    oss << "Username: " << username_ << "\n";
    oss << "Full Name: " << full_name_ << "\n";
    oss << "Email: " << email_ << "\n";
    oss << "Role: " << getRole() << "\n";
    oss << "Status: " << (is_active_ ? "Active" : "Inactive") << "\n";
    
    auto time_t = std::chrono::system_clock::to_time_t(created_date_);
    oss << "Created: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
    
    if (last_login_ != std::chrono::system_clock::time_point{}) {
        auto login_time_t = std::chrono::system_clock::to_time_t(last_login_);
        oss << "Last Login: " << std::put_time(std::localtime(&login_time_t), "%Y-%m-%d %H:%M:%S") << "\n";
    } else {
        oss << "Last Login: Never\n";
    }
    
    return oss.str();
}

void User::addPermission(Permission permission) {
    permissions_.insert(permission);
}

void User::removePermission(Permission permission) {
    permissions_.erase(permission);
}

std::vector<std::string> User::getPermissionStrings() const {
    std::vector<std::string> result;
    result.reserve(permissions_.size());
    
    for (const auto& permission : permissions_) {
        result.push_back(permissionToString(permission));
    }
    
    std::sort(result.begin(), result.end());
    return result;
}

std::string User::hashPassword(const std::string& password) const {
    // Simple hash implementation for demo purposes
    // In production, use a proper cryptographic hash like bcrypt
    std::hash<std::string> hasher;
    return std::to_string(hasher(password + "salt_string"));
}

bool User::verifyPassword(const std::string& password, const std::string& hash) const {
    return hashPassword(password) == hash;
}

// Staff Implementation

Staff::Staff(const std::string& user_id,
             const std::string& username,
             const std::string& password_hash,
             const std::string& email,
             const std::string& full_name,
             const std::string& department,
             const std::string& shift,
             const std::string& supervisor_id)
    : User(user_id, username, password_hash, email, full_name),
      department_(department), shift_(shift), supervisor_id_(supervisor_id) {
    
    // Set default staff permissions
    permissions_ = {
        Permission::VIEW_PRODUCTS,
        Permission::VIEW_INVENTORY,
        Permission::VIEW_ORDERS,
        Permission::CREATE_ORDERS,
        Permission::VIEW_REPORTS
    };
}

void Staff::setDepartment(const std::string& department) {
    department_ = department;
}

void Staff::setShift(const std::string& shift) {
    shift_ = shift;
}

void Staff::setSupervisorId(const std::string& supervisor_id) {
    supervisor_id_ = supervisor_id;
}

std::string Staff::getRole() const {
    return "Staff";
}

std::unordered_set<Permission> Staff::getPermissions() const {
    return permissions_;
}

bool Staff::canModify(const std::string& resource_type) const {
    if (resource_type == "order") {
        return hasPermission(Permission::CREATE_ORDERS);
    }
    // Staff generally cannot modify products or inventory
    return false;
}

std::string Staff::getUserInfo() const {
    std::ostringstream oss;
    oss << User::getUserInfo();
    oss << "Department: " << department_ << "\n";
    oss << "Shift: " << shift_ << "\n";
    if (!supervisor_id_.empty()) {
        oss << "Supervisor ID: " << supervisor_id_ << "\n";
    }
    
    auto permissions = getPermissionStrings();
    oss << "Permissions: ";
    for (size_t i = 0; i < permissions.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << permissions[i];
    }
    oss << "\n";
    
    return oss.str();
}

// Manager Implementation

Manager::Manager(const std::string& user_id,
                 const std::string& username,
                 const std::string& password_hash,
                 const std::string& email,
                 const std::string& full_name,
                 const std::string& department,
                 double budget_limit)
    : User(user_id, username, password_hash, email, full_name),
      department_(department), budget_limit_(budget_limit) {
    
    // Set default manager permissions (superset of staff permissions)
    permissions_ = {
        Permission::VIEW_PRODUCTS,
        Permission::ADD_PRODUCTS,
        Permission::MODIFY_PRODUCTS,
        Permission::DELETE_PRODUCTS,
        Permission::VIEW_INVENTORY,
        Permission::MODIFY_INVENTORY,
        Permission::VIEW_ORDERS,
        Permission::CREATE_ORDERS,
        Permission::MODIFY_ORDERS,
        Permission::CANCEL_ORDERS,
        Permission::VIEW_REPORTS,
        Permission::GENERATE_REPORTS,
        Permission::MANAGE_USERS
    };
}

void Manager::setDepartment(const std::string& department) {
    department_ = department;
}

void Manager::setBudgetLimit(double budget_limit) {
    if (budget_limit < 0) {
        throw std::invalid_argument("Budget limit cannot be negative");
    }
    budget_limit_ = budget_limit;
}

void Manager::addSupervisedStaff(const std::string& staff_id) {
    if (std::find(supervised_staff_.begin(), supervised_staff_.end(), staff_id) == supervised_staff_.end()) {
        supervised_staff_.push_back(staff_id);
    }
}

void Manager::removeSupervisedStaff(const std::string& staff_id) {
    auto it = std::find(supervised_staff_.begin(), supervised_staff_.end(), staff_id);
    if (it != supervised_staff_.end()) {
        supervised_staff_.erase(it);
    }
}

std::string Manager::getRole() const {
    return "Manager";
}

std::unordered_set<Permission> Manager::getPermissions() const {
    return permissions_;
}

bool Manager::canModify(const std::string& resource_type) const {
    if (resource_type == "product") {
        return hasPermission(Permission::MODIFY_PRODUCTS);
    } else if (resource_type == "inventory") {
        return hasPermission(Permission::MODIFY_INVENTORY);
    } else if (resource_type == "order") {
        return hasPermission(Permission::MODIFY_ORDERS);
    } else if (resource_type == "user") {
        return hasPermission(Permission::MANAGE_USERS);
    }
    return false;
}

bool Manager::canApproveExpense(double amount) const {
    return amount <= budget_limit_;
}

bool Manager::supervises(const std::string& staff_id) const {
    return std::find(supervised_staff_.begin(), supervised_staff_.end(), staff_id) != supervised_staff_.end();
}

std::string Manager::getUserInfo() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << User::getUserInfo();
    oss << "Department: " << department_ << "\n";
    oss << "Budget Limit: $" << budget_limit_ << "\n";
    oss << "Supervised Staff: " << supervised_staff_.size() << " members\n";
    
    auto permissions = getPermissionStrings();
    oss << "Permissions: ";
    for (size_t i = 0; i < permissions.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << permissions[i];
    }
    oss << "\n";
    
    return oss.str();
}

std::string Manager::generateStaffReport() const {
    std::ostringstream oss;
    
    oss << "=== STAFF REPORT ===\n";
    oss << "Manager: " << full_name_ << " (" << user_id_ << ")\n";
    oss << "Department: " << department_ << "\n";
    oss << "Total Supervised Staff: " << supervised_staff_.size() << "\n\n";
    
    if (!supervised_staff_.empty()) {
        oss << "Supervised Staff IDs:\n";
        for (const auto& staff_id : supervised_staff_) {
            oss << "- " << staff_id << "\n";
        }
    } else {
        oss << "No staff members currently supervised.\n";
    }
    
    return oss.str();
}

// UserManager Implementation

UserManager::UserManager() {
}

Staff* UserManager::createStaff(const std::string& user_id,
                                const std::string& username,
                                const std::string& password,
                                const std::string& email,
                                const std::string& full_name,
                                const std::string& department,
                                const std::string& shift,
                                const std::string& supervisor_id) {
    
    if (users_.find(user_id) != users_.end() || usernameExists(username)) {
        return nullptr; // User ID or username already exists
    }

    // Hash the password
    std::hash<std::string> hasher;
    std::string password_hash = std::to_string(hasher(password + "salt_string"));

    auto staff = std::make_unique<Staff>(user_id, username, password_hash, email, 
                                        full_name, department, shift, supervisor_id);
    Staff* staff_ptr = staff.get();
    
    users_[user_id] = std::move(staff);
    username_to_id_[username] = user_id;
    
    return staff_ptr;
}

Manager* UserManager::createManager(const std::string& user_id,
                                   const std::string& username,
                                   const std::string& password,
                                   const std::string& email,
                                   const std::string& full_name,
                                   const std::string& department,
                                   double budget_limit) {
    
    if (users_.find(user_id) != users_.end() || usernameExists(username)) {
        return nullptr; // User ID or username already exists
    }

    // Hash the password
    std::hash<std::string> hasher;
    std::string password_hash = std::to_string(hasher(password + "salt_string"));

    auto manager = std::make_unique<Manager>(user_id, username, password_hash, email, 
                                           full_name, department, budget_limit);
    Manager* manager_ptr = manager.get();
    
    users_[user_id] = std::move(manager);
    username_to_id_[username] = user_id;
    
    return manager_ptr;
}

User* UserManager::authenticateUser(const std::string& username, const std::string& password) {
    User* user = getUserByUsername(username);
    if (!user) {
        // Try as user ID
        user = getUser(username);
    }
    
    if (user && user->authenticate(password)) {
        user->updateLastLogin();
        current_user_id_ = user->getUserId();
        return user;
    }
    
    return nullptr;
}

User* UserManager::getUser(const std::string& user_id) {
    auto it = users_.find(user_id);
    return (it != users_.end()) ? it->second.get() : nullptr;
}

User* UserManager::getUserByUsername(const std::string& username) {
    auto it = username_to_id_.find(username);
    if (it != username_to_id_.end()) {
        return getUser(it->second);
    }
    return nullptr;
}

User* UserManager::getCurrentUser() {
    return current_user_id_.empty() ? nullptr : getUser(current_user_id_);
}

bool UserManager::setCurrentUser(const std::string& user_id) {
    User* user = getUser(user_id);
    if (user && user->isActive()) {
        current_user_id_ = user_id;
        return true;
    }
    return false;
}

void UserManager::logout() {
    current_user_id_.clear();
}

std::vector<User*> UserManager::getAllUsers() const {
    std::vector<User*> result;
    result.reserve(users_.size());
    
    for (const auto& pair : users_) {
        result.push_back(pair.second.get());
    }
    
    return result;
}

std::vector<User*> UserManager::getUsersByRole(const std::string& role) const {
    std::vector<User*> result;
    
    for (const auto& pair : users_) {
        if (pair.second->getRole() == role) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

bool UserManager::removeUser(const std::string& user_id) {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
        return false;
    }
    
    // Remove from username mapping
    const std::string& username = it->second->getUsername();
    username_to_id_.erase(username);
    
    // Clear current user if removing current user
    if (current_user_id_ == user_id) {
        current_user_id_.clear();
    }
    
    users_.erase(it);
    return true;
}

bool UserManager::currentUserHasPermission(Permission permission) const {
    User* user = const_cast<UserManager*>(this)->getCurrentUser();
    return user ? user->hasPermission(permission) : false;
}

bool UserManager::currentUserCanModify(const std::string& resource_type) const {
    User* user = const_cast<UserManager*>(this)->getCurrentUser();
    return user ? user->canModify(resource_type) : false;
}

std::string UserManager::getUserStatistics() const {
    std::ostringstream oss;
    
    oss << "=== USER STATISTICS ===\n";
    oss << "Total Users: " << users_.size() << "\n";
    
    auto staff = getUsersByRole("Staff");
    auto managers = getUsersByRole("Manager");
    
    oss << "Staff Members: " << staff.size() << "\n";
    oss << "Managers: " << managers.size() << "\n";
    
    int active_users = 0;
    for (const auto& pair : users_) {
        if (pair.second->isActive()) {
            active_users++;
        }
    }
    
    oss << "Active Users: " << active_users << "\n";
    oss << "Inactive Users: " << (users_.size() - active_users) << "\n";
    
    if (!current_user_id_.empty()) {
        User* current = const_cast<UserManager*>(this)->getCurrentUser();
        oss << "Currently Logged In: " << current->getUsername() << " (" << current->getRole() << ")\n";
    } else {
        oss << "Currently Logged In: None\n";
    }
    
    return oss.str();
}

bool UserManager::isValidUsername(const std::string& username) const {
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    // Check for valid characters (alphanumeric and underscore)
    std::regex username_regex("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, username_regex);
}

bool UserManager::isValidEmail(const std::string& email) const {
    // Basic email validation
    std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, email_regex);
}

bool UserManager::usernameExists(const std::string& username) const {
    return username_to_id_.find(username) != username_to_id_.end();
}

} // namespace quirkventory