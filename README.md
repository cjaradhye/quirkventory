# Inventory Management System

A comprehensive C++ retail inventory management system demonstrating object-oriented programming principles, multithreading, and modern C++ practices.

## Features

- **Object-Oriented Design**: Classes for Product, Inventory, Order, and User with inheritance, encapsulation, and polymorphism
- **Product Management**: Support for regular and perishable products with expiry tracking
- **User Management**: Role-based access control with Staff and Manager hierarchies
- **Order Processing**: Multi-threaded order processing with inventory validation
- **Inventory Tracking**: Real-time stock monitoring with low-stock alerts
- **Reporting System**: Sales reports and analytics with polymorphic behavior
- **CLI Interface**: Command-line interface for staff operations
- **REST API**: HTTP endpoints for external system integration
- **Thread Safety**: Concurrent operations with proper synchronization

## Architecture

### Core Classes

1. **Product Hierarchy**
   - `Product` (abstract base class)
   - `PerishableProduct` (derived class with expiry tracking)

2. **User Management**
   - `User` (abstract base class)
   - `Staff` and `Manager` (derived classes with role-based permissions)

3. **System Components**
   - `Inventory` (product storage and management)
   - `Order` (order processing and validation)
   - `Notification` (alert system with polymorphic notifications)

### Design Patterns

- **Inheritance**: Product and User hierarchies
- **Polymorphism**: Virtual methods for reporting and notifications
- **Encapsulation**: Private/protected members with controlled access
- **Template Pattern**: Order processing workflow
- **Observer Pattern**: Stock alert notifications

## Building and Running

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make

# Run the application
./bin/inventory_system
```

## API Endpoints

- `GET /api/products` - List all products
- `POST /api/products` - Add new product
- `PUT /api/products/{id}` - Update product
- `DELETE /api/products/{id}` - Remove product
- `GET /api/inventory/alerts` - Get low stock alerts
- `POST /api/orders` - Create new order
- `GET /api/reports/sales` - Generate sales report

## Dependencies

- C++17 or later
- CMake 3.12+
- STL containers and threading support
- HTTP library for REST API (included)

## Project Structure

```
quirkventory/
├── src/           # Source files (.cpp)
├── include/       # Header files (.hpp)
├── docs/          # Documentation and UML diagrams
├── tests/         # Unit tests
├── CMakeLists.txt # Build configuration
└── README.md      # This file
```