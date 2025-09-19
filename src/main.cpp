#include "include/CLI.hpp"
#include <iostream>
#include <exception>

/**
 * @brief Main entry point for the Quirkventory Inventory Management System
 * 
 * This application demonstrates a comprehensive C++ inventory management system
 * with object-oriented programming principles, multithreading, and modern C++ features.
 * 
 * Features demonstrated:
 * - Inheritance: Product -> PerishableProduct, User -> Staff/Manager
 * - Polymorphism: Virtual methods for reporting and notifications
 * - Encapsulation: Private/protected members with controlled access
 * - STL containers: maps, vectors, sets for data storage
 * - Multithreading: Concurrent order processing with thread safety
 * - CLI interface: Interactive command-line interface for staff operations
 * 
 * @return 0 on successful execution, 1 on error
 */
int main() {
    try {
        std::cout << "Starting Quirkventory Inventory Management System..." << std::endl;
        
        // Create and run the CLI interface
        quirkventory::CLI cli;
        cli.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}