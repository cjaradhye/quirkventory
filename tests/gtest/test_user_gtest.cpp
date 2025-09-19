#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <future>
#include <memory>
#include "../../include/User.hpp"

using namespace quirkventory;

// Test Fixture for User Tests
class UserTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
    }
    
    void TearDown() override {
        user_manager.reset();
    }
    
    std::unique_ptr<UserManager> user_manager;
};

// Basic User Creation Tests
TEST_F(UserTest, CreateStaffUser) {
    bool result = user_manager->createUser("john_staff", "password123", UserRole::STAFF);
    EXPECT_TRUE(result);
    
    User* user = user_manager->authenticate("john_staff", "password123");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->getUsername(), "john_staff");
    EXPECT_EQ(user->getRole(), UserRole::STAFF);
}

TEST_F(UserTest, CreateManagerUser) {
    bool result = user_manager->createUser("jane_manager", "admin456", UserRole::MANAGER);
    EXPECT_TRUE(result);
    
    User* user = user_manager->authenticate("jane_manager", "admin456");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->getUsername(), "jane_manager");
    EXPECT_EQ(user->getRole(), UserRole::MANAGER);
}

TEST_F(UserTest, CreateDuplicateUserFails) {
    user_manager->createUser("duplicate_user", "password", UserRole::STAFF);
    
    bool result = user_manager->createUser("duplicate_user", "different_password", UserRole::MANAGER);
    EXPECT_FALSE(result);
}

TEST_F(UserTest, CreateUserWithEmptyUsername) {
    bool result = user_manager->createUser("", "password", UserRole::STAFF);
    EXPECT_FALSE(result);
}

TEST_F(UserTest, CreateUserWithEmptyPassword) {
    bool result = user_manager->createUser("user", "", UserRole::STAFF);
    EXPECT_FALSE(result);
}

// Authentication Tests
TEST_F(UserTest, AuthenticateValidUser) {
    user_manager->createUser("test_user", "test_password", UserRole::STAFF);
    
    User* user = user_manager->authenticate("test_user", "test_password");
    EXPECT_NE(user, nullptr);
    EXPECT_EQ(user->getUsername(), "test_user");
}

TEST_F(UserTest, AuthenticateInvalidUsername) {
    user_manager->createUser("test_user", "test_password", UserRole::STAFF);
    
    User* user = user_manager->authenticate("wrong_user", "test_password");
    EXPECT_EQ(user, nullptr);
}

TEST_F(UserTest, AuthenticateInvalidPassword) {
    user_manager->createUser("test_user", "test_password", UserRole::STAFF);
    
    User* user = user_manager->authenticate("test_user", "wrong_password");
    EXPECT_EQ(user, nullptr);
}

TEST_F(UserTest, AuthenticateEmptyCredentials) {
    user_manager->createUser("test_user", "test_password", UserRole::STAFF);
    
    User* user1 = user_manager->authenticate("", "test_password");
    EXPECT_EQ(user1, nullptr);
    
    User* user2 = user_manager->authenticate("test_user", "");
    EXPECT_EQ(user2, nullptr);
}

// Staff Permissions Tests
class StaffPermissionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
        user_manager->createUser("staff_user", "password", UserRole::STAFF);
        staff_user = user_manager->authenticate("staff_user", "password");
        ASSERT_NE(staff_user, nullptr);
    }
    
    std::unique_ptr<UserManager> user_manager;
    User* staff_user;
};

TEST_F(StaffPermissionsTest, StaffHasBasicPermissions) {
    EXPECT_TRUE(staff_user->hasPermission(Permission::VIEW_INVENTORY));
    EXPECT_TRUE(staff_user->hasPermission(Permission::ADD_PRODUCTS));
    EXPECT_TRUE(staff_user->hasPermission(Permission::EDIT_PRODUCTS));
    EXPECT_TRUE(staff_user->hasPermission(Permission::PROCESS_ORDERS));
    EXPECT_TRUE(staff_user->hasPermission(Permission::VIEW_REPORTS));
}

TEST_F(StaffPermissionsTest, StaffLacksAdvancedPermissions) {
    EXPECT_FALSE(staff_user->hasPermission(Permission::DELETE_PRODUCTS));
    EXPECT_FALSE(staff_user->hasPermission(Permission::MANAGE_USERS));
    EXPECT_FALSE(staff_user->hasPermission(Permission::SYSTEM_ADMIN));
}

TEST_F(StaffPermissionsTest, StaffGetAllPermissions) {
    auto permissions = staff_user->getAllPermissions();
    
    // Staff should have exactly 5 permissions
    EXPECT_EQ(permissions.size(), 5);
    
    // Check that specific permissions are included
    EXPECT_THAT(permissions, ::testing::Contains(Permission::VIEW_INVENTORY));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::ADD_PRODUCTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::EDIT_PRODUCTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::PROCESS_ORDERS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::VIEW_REPORTS));
    
    // Check that restricted permissions are not included
    EXPECT_THAT(permissions, ::testing::Not(::testing::Contains(Permission::DELETE_PRODUCTS)));
    EXPECT_THAT(permissions, ::testing::Not(::testing::Contains(Permission::MANAGE_USERS)));
    EXPECT_THAT(permissions, ::testing::Not(::testing::Contains(Permission::SYSTEM_ADMIN)));
}

// Manager Permissions Tests
class ManagerPermissionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
        user_manager->createUser("manager_user", "password", UserRole::MANAGER);
        manager_user = user_manager->authenticate("manager_user", "password");
        ASSERT_NE(manager_user, nullptr);
    }
    
    std::unique_ptr<UserManager> user_manager;
    User* manager_user;
};

TEST_F(ManagerPermissionsTest, ManagerHasAllPermissions) {
    // Managers should have all permissions
    EXPECT_TRUE(manager_user->hasPermission(Permission::VIEW_INVENTORY));
    EXPECT_TRUE(manager_user->hasPermission(Permission::ADD_PRODUCTS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::EDIT_PRODUCTS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::DELETE_PRODUCTS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::PROCESS_ORDERS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::VIEW_REPORTS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::MANAGE_USERS));
    EXPECT_TRUE(manager_user->hasPermission(Permission::SYSTEM_ADMIN));
}

TEST_F(ManagerPermissionsTest, ManagerGetAllPermissions) {
    auto permissions = manager_user->getAllPermissions();
    
    // Manager should have all 8 permissions
    EXPECT_EQ(permissions.size(), 8);
    
    // Check that all permissions are included
    EXPECT_THAT(permissions, ::testing::Contains(Permission::VIEW_INVENTORY));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::ADD_PRODUCTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::EDIT_PRODUCTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::DELETE_PRODUCTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::PROCESS_ORDERS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::VIEW_REPORTS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::MANAGE_USERS));
    EXPECT_THAT(permissions, ::testing::Contains(Permission::SYSTEM_ADMIN));
}

// Session Management Tests
TEST_F(UserTest, CreateAndValidateSession) {
    user_manager->createUser("session_user", "password", UserRole::STAFF);
    User* user = user_manager->authenticate("session_user", "password");
    
    bool session_created = user_manager->createSession(user);
    EXPECT_TRUE(session_created);
    
    bool is_logged_in = user_manager->isLoggedIn("session_user");
    EXPECT_TRUE(is_logged_in);
}

TEST_F(UserTest, LogoutUser) {
    user_manager->createUser("logout_user", "password", UserRole::STAFF);
    User* user = user_manager->authenticate("logout_user", "password");
    user_manager->createSession(user);
    
    EXPECT_TRUE(user_manager->isLoggedIn("logout_user"));
    
    bool logout_result = user_manager->logout("logout_user");
    EXPECT_TRUE(logout_result);
    
    bool is_logged_in = user_manager->isLoggedIn("logout_user");
    EXPECT_FALSE(is_logged_in);
}

TEST_F(UserTest, LogoutNonExistentUser) {
    bool logout_result = user_manager->logout("nonexistent_user");
    EXPECT_FALSE(logout_result);
}

TEST_F(UserTest, CheckLoginStatusNonExistentUser) {
    bool is_logged_in = user_manager->isLoggedIn("nonexistent_user");
    EXPECT_FALSE(is_logged_in);
}

TEST_F(UserTest, CreateSessionForNullUser) {
    bool session_created = user_manager->createSession(nullptr);
    EXPECT_FALSE(session_created);
}

// Password Verification Tests
class PasswordTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
        user_manager->createUser("password_user", "test_password_123", UserRole::STAFF);
        user = user_manager->authenticate("password_user", "test_password_123");
        ASSERT_NE(user, nullptr);
    }
    
    std::unique_ptr<UserManager> user_manager;
    User* user;
};

TEST_F(PasswordTest, VerifyCorrectPassword) {
    EXPECT_TRUE(user->verifyPassword("test_password_123"));
}

TEST_F(PasswordTest, VerifyIncorrectPassword) {
    EXPECT_FALSE(user->verifyPassword("wrong_password"));
}

TEST_F(PasswordTest, VerifyEmptyPassword) {
    EXPECT_FALSE(user->verifyPassword(""));
}

TEST_F(PasswordTest, VerifyPasswordWithSpecialCharacters) {
    user_manager->createUser("special_user", "p@ssw0rd!#$", UserRole::STAFF);
    User* special_user = user_manager->authenticate("special_user", "p@ssw0rd!#$");
    
    ASSERT_NE(special_user, nullptr);
    EXPECT_TRUE(special_user->verifyPassword("p@ssw0rd!#$"));
    EXPECT_FALSE(special_user->verifyPassword("p@ssw0rd!#"));
}

// Concurrency Tests
class UserConcurrencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
    }
    
    std::unique_ptr<UserManager> user_manager;
};

TEST_F(UserConcurrencyTest, ConcurrentUserCreation) {
    std::vector<std::thread> threads;
    std::atomic<int> successful_creations{0};
    
    // Try to create users concurrently
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([&, i]() {
            std::string username = "concurrent_user_" + std::to_string(i);
            std::string password = "password_" + std::to_string(i);
            UserRole role = (i % 2 == 0) ? UserRole::STAFF : UserRole::MANAGER;
            
            if (user_manager->createUser(username, password, role)) {
                successful_creations++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_creations.load(), 100);
}

TEST_F(UserConcurrencyTest, ConcurrentAuthentication) {
    // Create test users first
    for (int i = 0; i < 50; ++i) {
        user_manager->createUser("auth_user_" + std::to_string(i), "password", UserRole::STAFF);
    }
    
    std::vector<std::future<bool>> futures;
    
    // Authenticate concurrently
    for (int i = 0; i < 50; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            User* user = user_manager->authenticate("auth_user_" + std::to_string(i), "password");
            return user != nullptr;
        }));
    }
    
    // All authentications should succeed
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

TEST_F(UserConcurrencyTest, ConcurrentSessionManagement) {
    // Create test users
    for (int i = 0; i < 20; ++i) {
        user_manager->createUser("session_user_" + std::to_string(i), "password", UserRole::STAFF);
    }
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_sessions{0};
    
    // Create sessions concurrently
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back([&, i]() {
            User* user = user_manager->authenticate("session_user_" + std::to_string(i), "password");
            if (user && user_manager->createSession(user)) {
                successful_sessions++;
                
                // Also test concurrent logout
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                user_manager->logout("session_user_" + std::to_string(i));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_sessions.load(), 20);
    
    // All users should be logged out
    for (int i = 0; i < 20; ++i) {
        EXPECT_FALSE(user_manager->isLoggedIn("session_user_" + std::to_string(i)));
    }
}

// Performance Tests
class UserPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>();
    }
    
    std::unique_ptr<UserManager> user_manager;
};

TEST_F(UserPerformanceTest, LargeScaleUserCreation) {
    const int num_users = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_users; ++i) {
        std::string username = "perf_user_" + std::to_string(i);
        std::string password = "password_" + std::to_string(i);
        UserRole role = (i % 2 == 0) ? UserRole::STAFF : UserRole::MANAGER;
        
        user_manager->createUser(username, password, role);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 10000); // Should complete within 10 seconds
}

TEST_F(UserPerformanceTest, LargeScaleAuthentication) {
    // Create users first
    const int num_users = 1000;
    for (int i = 0; i < num_users; ++i) {
        user_manager->createUser("auth_perf_user_" + std::to_string(i), "password", UserRole::STAFF);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Authenticate all users
    int successful_auths = 0;
    for (int i = 0; i < num_users; ++i) {
        User* user = user_manager->authenticate("auth_perf_user_" + std::to_string(i), "password");
        if (user) {
            successful_auths++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(successful_auths, num_users);
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
}

// Edge Cases and Error Conditions
TEST_F(UserTest, UserWithVeryLongUsername) {
    std::string long_username(1000, 'a'); // 1000 character username
    bool result = user_manager->createUser(long_username, "password", UserRole::STAFF);
    
    // System should handle long usernames gracefully
    EXPECT_TRUE(result);
    
    User* user = user_manager->authenticate(long_username, "password");
    EXPECT_NE(user, nullptr);
}

TEST_F(UserTest, UserWithVeryLongPassword) {
    std::string long_password(1000, 'p'); // 1000 character password
    bool result = user_manager->createUser("long_pass_user", long_password, UserRole::STAFF);
    
    EXPECT_TRUE(result);
    
    User* user = user_manager->authenticate("long_pass_user", long_password);
    EXPECT_NE(user, nullptr);
}

TEST_F(UserTest, MultipleSessionsForSameUser) {
    user_manager->createUser("multi_session_user", "password", UserRole::STAFF);
    User* user = user_manager->authenticate("multi_session_user", "password");
    
    // Create multiple sessions for the same user
    EXPECT_TRUE(user_manager->createSession(user));
    EXPECT_TRUE(user_manager->createSession(user)); // Should still return true
    
    EXPECT_TRUE(user_manager->isLoggedIn("multi_session_user"));
}

TEST_F(UserTest, PolymorphicBehavior) {
    // Test that Staff and Manager classes behave polymorphically
    user_manager->createUser("poly_staff", "password", UserRole::STAFF);
    user_manager->createUser("poly_manager", "password", UserRole::MANAGER);
    
    User* staff = user_manager->authenticate("poly_staff", "password");
    User* manager = user_manager->authenticate("poly_manager", "password");
    
    ASSERT_NE(staff, nullptr);
    ASSERT_NE(manager, nullptr);
    
    // Test polymorphic permission checking
    std::vector<User*> users = {staff, manager};
    for (User* user : users) {
        // All users should have basic permissions
        EXPECT_TRUE(user->hasPermission(Permission::VIEW_INVENTORY));
        
        // Only managers should have delete permission
        bool has_delete = user->hasPermission(Permission::DELETE_PRODUCTS);
        if (user->getRole() == UserRole::MANAGER) {
            EXPECT_TRUE(has_delete);
        } else {
            EXPECT_FALSE(has_delete);
        }
        
        // Test getAllPermissions polymorphism
        auto permissions = user->getAllPermissions();
        EXPECT_FALSE(permissions.empty());
        
        if (user->getRole() == UserRole::MANAGER) {
            EXPECT_EQ(permissions.size(), 8);
        } else {
            EXPECT_EQ(permissions.size(), 5);
        }
    }
}