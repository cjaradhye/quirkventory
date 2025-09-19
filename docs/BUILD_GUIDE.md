# Building and Running Quirkventory

This guide explains how to build and run the Quirkventory C++ inventory management system.

## Prerequisites

### Required Software
- **C++ Compiler**: GCC 7.0+, Clang 6.0+, or MSVC 2017+
- **CMake**: Version 3.10 or higher
- **Git**: For cloning the repository

### System Requirements
- **Operating System**: Linux, macOS, or Windows
- **RAM**: Minimum 1GB available
- **Disk Space**: 100MB for build artifacts

## Quick Start

### 1. Clone and Navigate to Project
```bash
cd /Users/zwarup.cj/Documents/projects/quirkventory
```

### 2. Build the Project
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)  # Linux/macOS
# OR for Windows:
# cmake --build . --config Release
```

### 3. Run the Application
```bash
# Run main application
./quirkventory

# Run tests
./quirkventory_test
```

## Build Options

### Debug Build
```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build
```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### With Custom Install Prefix
```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/quirkventory ..
make install
```

## CMake Targets

### Available Targets
- `quirkventory` - Main application executable
- `quirkventory_lib` - Static library with core functionality
- `quirkventory_test` - Test suite executable
- `run` - Convenience target to run the main application
- `test` - Convenience target to run tests
- `doc_doxygen` - Generate API documentation (if Doxygen is available)

### Running Targets
```bash
# Build and run application
make run

# Build and run tests
make test

# Generate documentation
make doc_doxygen
```

## Testing

### Running All Tests
```bash
cd build
./quirkventory_test
```

### Test Categories
The test suite includes:
- **Unit Tests**: Individual class functionality
- **Integration Tests**: Component interaction
- **Performance Tests**: Large dataset handling
- **Concurrency Tests**: Thread safety verification

### Expected Test Output
```
=== Quirkventory Test Suite ===

Testing Product classes...
✓ Product tests passed

Testing Inventory class...
✓ Inventory tests passed

Testing Order classes...
✓ Order processing tests passed

Testing User classes...
✓ User management tests passed

Testing Notification System...
✓ Notification system tests passed

Running performance tests...
Added 10,000 products in 45ms
Search completed in 12μs
✓ Performance tests passed

Testing concurrency...
Processed 50/50 orders successfully
✓ Concurrency tests passed

=== All Tests Passed! ===
```

## Using the Application

### CLI Interface
The main application provides an interactive command-line interface:

```bash
./quirkventory
```

#### Login Credentials
Default users for testing:
- **Staff Account**: username: `staff`, password: `password`
- **Manager Account**: username: `admin`, password: `admin123`

#### Available Commands
- **Inventory Management**: Add, view, search, update products
- **Order Processing**: Create and process customer orders
- **User Management**: (Manager only) Create staff accounts
- **Reports**: Generate sales and inventory reports
- **System Alerts**: View low stock and expiry notifications

### REST API (Simulated)
The application includes a simulated HTTP server demonstrating REST API design:

```bash
# The server shows available endpoints when started:
GET    /api/products
POST   /api/products
GET    /api/products/{id}
PUT    /api/products/{id}
DELETE /api/products/{id}
GET    /api/inventory/status
GET    /api/orders
POST   /api/orders
GET    /api/reports/sales
GET    /api/system/status
```

## Troubleshooting

### Common Build Issues

#### CMake Not Found
```bash
# Install CMake (Ubuntu/Debian)
sudo apt install cmake

# Install CMake (macOS with Homebrew)
brew install cmake

# Install CMake (Windows)
# Download from https://cmake.org/download/
```

#### Compiler Too Old
```bash
# Check compiler version
g++ --version
clang++ --version

# Update compiler (Ubuntu/Debian)
sudo apt update && sudo apt install g++-9

# Update Xcode Command Line Tools (macOS)
xcode-select --install
```

#### Threading Library Issues
```bash
# Link pthread manually if needed
cmake -DCMAKE_CXX_FLAGS="-pthread" ..
```

### Runtime Issues

#### Permission Denied
```bash
# Make executable
chmod +x quirkventory quirkventory_test
```

#### Segmentation Fault
```bash
# Run with debugger
gdb ./quirkventory
(gdb) run
(gdb) bt  # when it crashes
```

#### Memory Leaks (Debug Mode)
```bash
# Run with Valgrind (Linux)
valgrind --leak-check=full ./quirkventory

# Run with AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ./quirkventory
```

## Performance Optimization

### Compile Optimizations
```bash
# Maximum optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Link-time optimization
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=TRUE ..
```

### Runtime Performance
- The system is optimized for handling 10,000+ products
- Concurrent order processing supports multiple simultaneous operations
- Memory usage is optimized with smart pointers and move semantics

## Development

### Code Style
- **Standard**: C++17
- **Naming**: CamelCase for classes, snake_case for variables
- **Documentation**: Doxygen-style comments
- **Headers**: Include guards and forward declarations

### Adding New Features
1. Add header file to `include/`
2. Add source file to `src/`
3. Update `CMakeLists.txt` to include new files
4. Add tests to `tests/`
5. Update documentation

### Debugging
```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with GDB
gdb ./quirkventory
(gdb) break main
(gdb) run
```

## Documentation

### API Documentation
```bash
# Generate with Doxygen (if available)
make doc_doxygen

# View generated docs
open docs/html/index.html  # macOS
xdg-open docs/html/index.html  # Linux
```

### Code Structure
```
src/           - Source implementation files
include/       - Header files
tests/         - Test files
docs/          - Documentation and UML diagrams
build/         - Build artifacts (generated)
```

## Deployment

### Installing System-Wide
```bash
sudo make install
```

### Creating Package
```bash
make package
```

### Static Analysis
```bash
# Run with clang-tidy
clang-tidy src/*.cpp -- -Iinclude -std=c++17

# Run with cppcheck
cppcheck --enable=all src/ include/
```

For additional help or issues, refer to the project README.md or the UML documentation in the `docs/` directory.