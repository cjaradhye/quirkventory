#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <unordered_set>

namespace quirkventory {

/**
 * @brief User permissions enumeration
 */
enum class Permission {
    VIEW_PRODUCTS,
    ADD_PRODUCTS,
    MODIFY_PRODUCTS,
    DELETE_PRODUCTS,
    VIEW_INVENTORY,
    MODIFY_INVENTORY,
    VIEW_ORDERS,
    CREATE_ORDERS,
    MODIFY_ORDERS,
    CANCEL_ORDERS,
    VIEW_REPORTS,
    GENERATE_REPORTS,
    MANAGE_USERS,
    SYSTEM_ADMIN
};

/**
 * @brief Convert Permission to string
 */
std::string permissionToString(Permission permission);

/**
 * @brief Abstract base class for all users in the system
 * 
 * Demonstrates encapsulation with protected members and virtual methods
 * for polymorphic behavior in role-based access control.
 */
class User {
protected:
    std::string user_id_;
    std::string username_;
    std::string password_hash_;
    std::string email_;
    std::string full_name_;
    std::chrono::system_clock::time_point created_date_;
    std::chrono::system_clock::time_point last_login_;
    bool is_active_;
    std::unordered_set<Permission> permissions_;

public:
    /**
     * @brief Constructor for User
     * @param user_id Unique user identifier
     * @param username Login username
     * @param password_hash Hashed password
     * @param email User email address
     * @param full_name User's full name
     */
    User(const std::string& user_id,
         const std::string& username,
         const std::string& password_hash,
         const std::string& email,
         const std::string& full_name);

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~User() = default;

    // Getters (const methods for encapsulation)
    const std::string& getUserId() const { return user_id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getFullName() const { return full_name_; }
    const std::chrono::system_clock::time_point& getCreatedDate() const { return created_date_; }
    const std::chrono::system_clock::time_point& getLastLogin() const { return last_login_; }
    bool isActive() const { return is_active_; }

    // Setters with validation
    void setUsername(const std::string& username);
    void setEmail(const std::string& email);
    void setFullName(const std::string& full_name);
    void setPasswordHash(const std::string& password_hash);
    void setActive(bool active);

    /**
     * @brief Virtual method to get user role
     * @return String representation of the user's role
     */
    virtual std::string getRole() const = 0;

    /**
     * @brief Virtual method to get user permissions
     * @return Set of permissions for this user
     * 
     * Derived classes can override to provide role-specific permissions
     */
    virtual std::unordered_set<Permission> getPermissions() const;

    /**
     * @brief Virtual method to check if user can perform an action
     * @param permission Permission to check
     * @return true if user has the permission
     */
    virtual bool hasPermission(Permission permission) const;

    /**
     * @brief Virtual method to check if user can modify a resource
     * @param resource_type Type of resource (product, order, etc.)
     * @return true if user can modify this type of resource
     */
    virtual bool canModify(const std::string& resource_type) const;

    /**
     * @brief Authenticate user with password
     * @param password Plain text password to verify
     * @return true if authentication successful
     */
    virtual bool authenticate(const std::string& password) const;

    /**
     * @brief Update last login timestamp
     */
    void updateLastLogin();

    /**
     * @brief Get user information
     * @return Formatted string with user details
     */
    virtual std::string getUserInfo() const;

    /**
     * @brief Add a permission to the user
     * @param permission Permission to add
     */
    void addPermission(Permission permission);

    /**
     * @brief Remove a permission from the user
     * @param permission Permission to remove
     */
    void removePermission(Permission permission);

    /**
     * @brief Get all permissions as strings
     * @return Vector of permission names
     */
    std::vector<std::string> getPermissionStrings() const;

protected:
    /**
     * @brief Hash a password (simple implementation for demo)
     * @param password Plain text password
     * @return Hashed password
     */
    std::string hashPassword(const std::string& password) const;

    /**
     * @brief Verify a password against hash
     * @param password Plain text password
     * @param hash Stored password hash
     * @return true if password matches
     */
    bool verifyPassword(const std::string& password, const std::string& hash) const;
};

/**
 * @brief Staff user class with basic permissions
 * 
 * Demonstrates inheritance from User class and implements
 * role-specific permissions for regular staff members.
 */
class Staff : public User {
private:
    std::string department_;
    std::string shift_;
    std::string supervisor_id_;

public:
    /**
     * @brief Constructor for Staff
     */
    Staff(const std::string& user_id,
          const std::string& username,
          const std::string& password_hash,
          const std::string& email,
          const std::string& full_name,
          const std::string& department,
          const std::string& shift = "Day",
          const std::string& supervisor_id = "");

    // Getters for staff-specific attributes
    const std::string& getDepartment() const { return department_; }
    const std::string& getShift() const { return shift_; }
    const std::string& getSupervisorId() const { return supervisor_id_; }

    // Setters for staff-specific attributes
    void setDepartment(const std::string& department);
    void setShift(const std::string& shift);
    void setSupervisorId(const std::string& supervisor_id);

    /**
     * @brief Override: Get user role
     * @return "Staff"
     */
    std::string getRole() const override;

    /**
     * @brief Override: Get staff permissions
     * @return Set of permissions available to staff
     */
    std::unordered_set<Permission> getPermissions() const override;

    /**
     * @brief Override: Check modification permissions
     * @param resource_type Type of resource
     * @return true if staff can modify this resource type
     */
    bool canModify(const std::string& resource_type) const override;

    /**
     * @brief Override: Get detailed user information
     * @return Formatted string with staff details
     */
    std::string getUserInfo() const override;
};

/**
 * @brief Manager user class with extended permissions
 * 
 * Demonstrates inheritance from User class and implements
 * elevated permissions for management roles.
 */
class Manager : public User {
private:
    std::string department_;
    double budget_limit_;
    std::vector<std::string> supervised_staff_;

public:
    /**
     * @brief Constructor for Manager
     */
    Manager(const std::string& user_id,
            const std::string& username,
            const std::string& password_hash,
            const std::string& email,
            const std::string& full_name,
            const std::string& department,
            double budget_limit = 0.0);

    // Getters for manager-specific attributes
    const std::string& getDepartment() const { return department_; }
    double getBudgetLimit() const { return budget_limit_; }
    const std::vector<std::string>& getSupervisedStaff() const { return supervised_staff_; }

    // Setters for manager-specific attributes
    void setDepartment(const std::string& department);
    void setBudgetLimit(double budget_limit);
    void addSupervisedStaff(const std::string& staff_id);
    void removeSupervisedStaff(const std::string& staff_id);

    /**
     * @brief Override: Get user role
     * @return "Manager"
     */
    std::string getRole() const override;

    /**
     * @brief Override: Get manager permissions
     * @return Set of permissions available to managers
     */
    std::unordered_set<Permission> getPermissions() const override;

    /**
     * @brief Override: Check modification permissions
     * @param resource_type Type of resource
     * @return true if manager can modify this resource type
     */
    bool canModify(const std::string& resource_type) const override;

    /**
     * @brief Check if manager can approve an expense
     * @param amount Amount to approve
     * @return true if within budget limit
     */
    bool canApproveExpense(double amount) const;

    /**
     * @brief Check if manager supervises a staff member
     * @param staff_id Staff member ID
     * @return true if staff is supervised by this manager
     */
    bool supervises(const std::string& staff_id) const;

    /**
     * @brief Override: Get detailed user information
     * @return Formatted string with manager details
     */
    std::string getUserInfo() const override;

    /**
     * @brief Generate report for supervised staff
     * @return Formatted string with staff information
     */
    std::string generateStaffReport() const;
};

/**
 * @brief User management system
 * 
 * Manages user authentication, authorization, and role-based access control
 */
class UserManager {
private:
    std::unordered_map<std::string, std::unique_ptr<User>> users_;
    std::unordered_map<std::string, std::string> username_to_id_;
    std::string current_user_id_;

public:
    /**
     * @brief Constructor
     */
    UserManager();

    /**
     * @brief Destructor
     */
    ~UserManager() = default;

    // Disable copy constructor and assignment operator
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;

    /**
     * @brief Create a new staff user
     * @param user_id Unique user identifier
     * @param username Login username
     * @param password Plain text password
     * @param email User email
     * @param full_name User's full name
     * @param department Department name
     * @param shift Work shift
     * @param supervisor_id Supervisor's user ID
     * @return Pointer to created user, nullptr if user_id already exists
     */
    Staff* createStaff(const std::string& user_id,
                      const std::string& username,
                      const std::string& password,
                      const std::string& email,
                      const std::string& full_name,
                      const std::string& department,
                      const std::string& shift = "Day",
                      const std::string& supervisor_id = "");

    /**
     * @brief Create a new manager user
     * @param user_id Unique user identifier
     * @param username Login username
     * @param password Plain text password
     * @param email User email
     * @param full_name User's full name
     * @param department Department name
     * @param budget_limit Budget approval limit
     * @return Pointer to created user, nullptr if user_id already exists
     */
    Manager* createManager(const std::string& user_id,
                          const std::string& username,
                          const std::string& password,
                          const std::string& email,
                          const std::string& full_name,
                          const std::string& department,
                          double budget_limit = 0.0);

    /**
     * @brief Authenticate a user
     * @param username Username or user ID
     * @param password Plain text password
     * @return Pointer to user if authentication successful, nullptr otherwise
     */
    User* authenticateUser(const std::string& username, const std::string& password);

    /**
     * @brief Get a user by ID
     * @param user_id User identifier
     * @return Pointer to user, nullptr if not found
     */
    User* getUser(const std::string& user_id);

    /**
     * @brief Get a user by username
     * @param username Username
     * @return Pointer to user, nullptr if not found
     */
    User* getUserByUsername(const std::string& username);

    /**
     * @brief Get current logged-in user
     * @return Pointer to current user, nullptr if no user logged in
     */
    User* getCurrentUser();

    /**
     * @brief Set current user (login)
     * @param user_id User ID to set as current
     * @return true if user exists and is active
     */
    bool setCurrentUser(const std::string& user_id);

    /**
     * @brief Logout current user
     */
    void logout();

    /**
     * @brief Get all users
     * @return Vector of pointers to all users
     */
    std::vector<User*> getAllUsers() const;

    /**
     * @brief Get users by role
     * @param role Role to filter by ("Staff", "Manager")
     * @return Vector of users with the specified role
     */
    std::vector<User*> getUsersByRole(const std::string& role) const;

    /**
     * @brief Remove a user
     * @param user_id User ID to remove
     * @return true if removed successfully
     */
    bool removeUser(const std::string& user_id);

    /**
     * @brief Check if current user has permission
     * @param permission Permission to check
     * @return true if current user has the permission
     */
    bool currentUserHasPermission(Permission permission) const;

    /**
     * @brief Check if current user can modify a resource
     * @param resource_type Type of resource
     * @return true if current user can modify this resource type
     */
    bool currentUserCanModify(const std::string& resource_type) const;

    /**
     * @brief Get user statistics
     * @return Formatted string with user statistics
     */
    std::string getUserStatistics() const;

    /**
     * @brief Validate username format
     * @param username Username to validate
     * @return true if username is valid
     */
    bool isValidUsername(const std::string& username) const;

    /**
     * @brief Validate email format
     * @param email Email to validate
     * @return true if email is valid
     */
    bool isValidEmail(const std::string& email) const;

private:
    /**
     * @brief Check if username already exists
     * @param username Username to check
     * @return true if username exists
     */
    bool usernameExists(const std::string& username) const;
};

} // namespace quirkventory