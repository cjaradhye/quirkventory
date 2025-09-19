#include "../include/HTTPServer.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

// Note: This is a simplified HTTP server implementation for demonstration purposes.
// In a production environment, you would use a proper HTTP library like:
// - cpp-httplib (https://github.com/yhirose/cpp-httplib)
// - Crow (https://github.com/CrowCpp/Crow)
// - Pistache (https://github.com/pistacheio/pistache)

namespace quirkventory {

// HTTPRequest Implementation

std::string HTTPRequest::getQueryParam(const std::string& key) const {
    std::regex param_regex(key + "=([^&]*)");
    std::smatch match;
    if (std::regex_search(query_string, match, param_regex)) {
        return match[1].str();
    }
    return "";
}

// HTTPResponse Implementation

HTTPResponse::HTTPResponse(int code, const std::string& message)
    : status_code(code), status_message(message) {
    headers["Content-Type"] = "text/plain";
    headers["Server"] = "Quirkventory/1.0";
    headers["Connection"] = "close";
}

void HTTPResponse::setContentType(const std::string& content_type) {
    headers["Content-Type"] = content_type;
}

void HTTPResponse::setBody(const std::string& content, const std::string& content_type) {
    body = content;
    headers["Content-Type"] = content_type;
    headers["Content-Length"] = std::to_string(content.length());
}

void HTTPResponse::setJSONBody(const std::string& json_content) {
    setBody(json_content, "application/json");
}

std::string HTTPResponse::toString() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n" << body;
    return oss.str();
}

// HTTPServer Implementation

HTTPServer::HTTPServer(const std::string& host, int port)
    : host_(host), port_(port), running_(false),
      inventory_(nullptr), order_manager_(nullptr),
      user_manager_(nullptr), notification_manager_(nullptr) {
}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::setSystemComponents(Inventory* inventory,
                                   OrderManager* order_manager,
                                   UserManager* user_manager,
                                   NotificationManager* notification_manager) {
    inventory_ = inventory;
    order_manager_ = order_manager;
    user_manager_ = user_manager;
    notification_manager_ = notification_manager;
}

bool HTTPServer::start() {
    if (running_.load()) {
        return false; // Already running
    }

    setupRoutes();
    running_.store(true);
    
    // Note: In a real implementation, this would create a socket and listen for connections
    // For demonstration, we'll simulate the server
    server_thread_ = std::thread(&HTTPServer::serverLoop, this);
    
    std::cout << "HTTP Server started on " << getServerUrl() << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET    /api/products" << std::endl;
    std::cout << "  POST   /api/products" << std::endl;
    std::cout << "  GET    /api/products/{id}" << std::endl;
    std::cout << "  PUT    /api/products/{id}" << std::endl;
    std::cout << "  DELETE /api/products/{id}" << std::endl;
    std::cout << "  GET    /api/inventory/status" << std::endl;
    std::cout << "  GET    /api/inventory/alerts/low-stock" << std::endl;
    std::cout << "  GET    /api/inventory/alerts/expiry" << std::endl;
    std::cout << "  GET    /api/orders" << std::endl;
    std::cout << "  POST   /api/orders" << std::endl;
    std::cout << "  GET    /api/orders/{id}" << std::endl;
    std::cout << "  GET    /api/reports/sales" << std::endl;
    std::cout << "  GET    /api/reports/inventory" << std::endl;
    std::cout << "  GET    /api/system/status" << std::endl;
    
    return true;
}

void HTTPServer::stop() {
    if (!running_.load()) {
        return; // Not running
    }

    running_.store(false);
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    std::cout << "HTTP Server stopped" << std::endl;
}

std::string HTTPServer::getServerUrl() const {
    return "http://" + host_ + ":" + std::to_string(port_);
}

void HTTPServer::setupRoutes() {
    // Product endpoints
    get_handlers_["/api/products"] = [this](const HTTPRequest& req) { return handleGetProducts(req); };
    get_handlers_["/api/products/{id}"] = [this](const HTTPRequest& req) { return handleGetProduct(req); };
    post_handlers_["/api/products"] = [this](const HTTPRequest& req) { return handlePostProduct(req); };
    put_handlers_["/api/products/{id}"] = [this](const HTTPRequest& req) { return handlePutProduct(req); };
    delete_handlers_["/api/products/{id}"] = [this](const HTTPRequest& req) { return handleDeleteProduct(req); };
    
    // Inventory endpoints
    get_handlers_["/api/inventory/status"] = [this](const HTTPRequest& req) { return handleGetInventoryStatus(req); };
    get_handlers_["/api/inventory/alerts/low-stock"] = [this](const HTTPRequest& req) { return handleGetLowStockAlerts(req); };
    get_handlers_["/api/inventory/alerts/expiry"] = [this](const HTTPRequest& req) { return handleGetExpiryAlerts(req); };
    
    // Order endpoints
    get_handlers_["/api/orders"] = [this](const HTTPRequest& req) { return handleGetOrders(req); };
    get_handlers_["/api/orders/{id}"] = [this](const HTTPRequest& req) { return handleGetOrder(req); };
    post_handlers_["/api/orders"] = [this](const HTTPRequest& req) { return handlePostOrder(req); };
    
    // Report endpoints
    get_handlers_["/api/reports/sales"] = [this](const HTTPRequest& req) { return handleGetSalesReport(req); };
    get_handlers_["/api/reports/inventory"] = [this](const HTTPRequest& req) { return handleGetInventoryReport(req); };
    
    // System endpoints
    get_handlers_["/api/system/status"] = [this](const HTTPRequest& req) { return handleGetSystemStatus(req); };
}

void HTTPServer::serverLoop() {
    // Simulated server loop - in a real implementation, this would handle actual HTTP connections
    std::cout << "Server loop started (simulated - actual HTTP handling would be implemented here)" << std::endl;
    
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // In a real implementation, this would:
        // 1. Accept incoming connections
        // 2. Read HTTP requests
        // 3. Parse requests
        // 4. Route to handlers
        // 5. Send responses
    }
}

HTTPResponse HTTPServer::handleRequest(const std::string& request_data) {
    try {
        HTTPRequest request = parseRequest(request_data);
        return routeRequest(request);
    } catch (const std::exception& e) {
        return createErrorResponse(400, "Bad Request: " + std::string(e.what()));
    }
}

HTTPRequest HTTPServer::parseRequest(const std::string& request_data) {
    HTTPRequest request;
    std::istringstream iss(request_data);
    std::string line;
    
    // Parse request line
    if (std::getline(iss, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path;
        
        // Extract query string
        auto query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query_string = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
    }
    
    // Parse headers
    while (std::getline(iss, line) && line != "\r") {
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            request.headers[key] = value;
        }
    }
    
    // Parse body
    std::string body_line;
    while (std::getline(iss, body_line)) {
        request.body += body_line + "\n";
    }
    
    return request;
}

HTTPResponse HTTPServer::routeRequest(const HTTPRequest& request) {
    auto& handlers = (request.method == "GET") ? get_handlers_ :
                    (request.method == "POST") ? post_handlers_ :
                    (request.method == "PUT") ? put_handlers_ :
                    (request.method == "DELETE") ? delete_handlers_ :
                    get_handlers_; // fallback
    
    // Try exact match first
    auto it = handlers.find(request.path);
    if (it != handlers.end()) {
        return it->second(request);
    }
    
    // Try pattern matching for parameterized routes
    for (const auto& pair : handlers) {
        if (pair.first.find("{id}") != std::string::npos) {
            std::string pattern = pair.first;
            std::replace(pattern.begin(), pattern.end(), '{', '(');
            std::replace(pattern.begin(), pattern.end(), '}', ')');
            pattern = std::regex_replace(pattern, std::regex("\\(id\\)"), "([^/]+)");
            
            std::regex route_regex(pattern);
            if (std::regex_match(request.path, route_regex)) {
                return pair.second(request);
            }
        }
    }
    
    return createErrorResponse(404, "Not Found");
}

HTTPResponse HTTPServer::createErrorResponse(int status_code, const std::string& message) {
    HTTPResponse response(status_code, message);
    response.setJSONBody(JSONUtils::formatErrorJSON(message, status_code));
    return response;
}

HTTPResponse HTTPServer::createJSONResponse(const std::string& data) {
    HTTPResponse response(200, "OK");
    response.setJSONBody(data);
    return response;
}

// API endpoint handlers

HTTPResponse HTTPServer::handleGetProducts(const HTTPRequest& request) {
    if (!inventory_) {
        return createErrorResponse(500, "Inventory system not available");
    }
    
    auto products = inventory_->getAllProducts();
    std::vector<std::string> product_json_list;
    
    for (const auto* product : products) {
        product_json_list.push_back(productToJSON(product));
    }
    
    std::string json_response = JSONUtils::createJSONObject({
        {"status", "\"success\""},
        {"count", std::to_string(products.size())},
        {"products", JSONUtils::createJSONArray(product_json_list)}
    });
    
    return createJSONResponse(json_response);
}

HTTPResponse HTTPServer::handleGetProduct(const HTTPRequest& request) {
    if (!inventory_) {
        return createErrorResponse(500, "Inventory system not available");
    }
    
    std::string product_id = extractPathParameter(request.path, "/api/products/([^/]+)");
    if (product_id.empty()) {
        return createErrorResponse(400, "Invalid product ID");
    }
    
    const Product* product = inventory_->getProduct(product_id);
    if (!product) {
        return createErrorResponse(404, "Product not found");
    }
    
    std::string json_response = JSONUtils::createJSONObject({
        {"status", "\"success\""},
        {"product", productToJSON(product)}
    });
    
    return createJSONResponse(json_response);
}

HTTPResponse HTTPServer::handlePostProduct(const HTTPRequest& request) {
    if (!inventory_) {
        return createErrorResponse(500, "Inventory system not available");
    }
    
    try {
        // Parse JSON body to create product
        std::string id = parseJSONString(request.body, "id");
        std::string name = parseJSONString(request.body, "name");
        std::string category = parseJSONString(request.body, "category");
        double price = parseJSONDouble(request.body, "price");
        int quantity = parseJSONInt(request.body, "quantity");
        
        if (id.empty() || name.empty()) {
            return createErrorResponse(400, "Product ID and name are required");
        }
        
        auto product = std::make_unique<Product>(id, name, category, price, quantity);
        
        if (inventory_->addProduct(std::move(product))) {
            return createJSONResponse(JSONUtils::formatSuccessJSON("Product created successfully"));
        } else {
            return createErrorResponse(409, "Product ID already exists");
        }
    } catch (const std::exception& e) {
        return createErrorResponse(400, "Invalid product data: " + std::string(e.what()));
    }
}

HTTPResponse HTTPServer::handleGetInventoryStatus(const HTTPRequest& request) {
    if (!inventory_) {
        return createErrorResponse(500, "Inventory system not available");
    }
    
    std::string json_response = JSONUtils::createJSONObject({
        {"status", "\"success\""},
        {"total_products", std::to_string(inventory_->getTotalProductCount())},
        {"total_quantity", std::to_string(inventory_->getTotalQuantity())},
        {"total_value", std::to_string(inventory_->getTotalValue())},
        {"low_stock_count", std::to_string(inventory_->getLowStockProducts().size())},
        {"expired_count", std::to_string(inventory_->getExpiredProducts().size())}
    });
    
    return createJSONResponse(json_response);
}

HTTPResponse HTTPServer::handleGetLowStockAlerts(const HTTPRequest& request) {
    if (!inventory_) {
        return createErrorResponse(500, "Inventory system not available");
    }
    
    auto low_stock_products = inventory_->getLowStockProducts();
    std::vector<std::string> alerts;
    
    for (const auto* product : low_stock_products) {
        alerts.push_back(JSONUtils::createJSONObject({
            {"product_id", "\"" + product->getId() + "\""},
            {"product_name", "\"" + product->getName() + "\""},
            {"current_stock", std::to_string(product->getQuantity())},
            {"category", "\"" + product->getCategory() + "\""}
        }));
    }
    
    std::string json_response = JSONUtils::createJSONObject({
        {"status", "\"success\""},
        {"alert_count", std::to_string(alerts.size())},
        {"alerts", JSONUtils::createJSONArray(alerts)}
    });
    
    return createJSONResponse(json_response);
}

HTTPResponse HTTPServer::handleGetSystemStatus(const HTTPRequest& request) {
    std::string json_response = JSONUtils::createJSONObject({
        {"status", "\"success\""},
        {"server", "\"Quirkventory HTTP Server\""},
        {"version", "\"1.0.0\""},
        {"uptime", "\"running\""},
        {"inventory_available", inventory_ ? "true" : "false"},
        {"order_manager_available", order_manager_ ? "true" : "false"},
        {"user_manager_available", user_manager_ ? "true" : "false"},
        {"notification_manager_available", notification_manager_ ? "true" : "false"}
    });
    
    return createJSONResponse(json_response);
}

// Utility methods

std::string HTTPServer::extractPathParameter(const std::string& path, const std::string& pattern) {
    std::regex param_regex(pattern);
    std::smatch match;
    if (std::regex_match(path, match, param_regex) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string HTTPServer::productToJSON(const Product* product) {
    if (!product) return "{}";
    
    return JSONUtils::createJSONObject({
        {"id", "\"" + product->getId() + "\""},
        {"name", "\"" + JSONUtils::escapeJSON(product->getName()) + "\""},
        {"category", "\"" + JSONUtils::escapeJSON(product->getCategory()) + "\""},
        {"price", std::to_string(product->getPrice())},
        {"quantity", std::to_string(product->getQuantity())},
        {"is_expired", product->isExpired() ? "true" : "false"},
        {"expiry_info", "\"" + JSONUtils::escapeJSON(product->getExpiryInfo()) + "\""}
    });
}

std::string HTTPServer::parseJSONString(const std::string& json, const std::string& key) {
    std::string value = JSONUtils::extractJSONValue(json, key);
    // Remove quotes
    if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.length() - 2);
    }
    return value;
}

double HTTPServer::parseJSONDouble(const std::string& json, const std::string& key) {
    std::string value = JSONUtils::extractJSONValue(json, key);
    return value.empty() ? 0.0 : std::stod(value);
}

int HTTPServer::parseJSONInt(const std::string& json, const std::string& key) {
    std::string value = JSONUtils::extractJSONValue(json, key);
    return value.empty() ? 0 : std::stoi(value);
}

// Additional handlers would be implemented here...
// For brevity, showing the pattern and key endpoints

// JSONUtils Implementation

namespace JSONUtils {

std::string escapeJSON(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    
    return result;
}

std::string createJSONObject(const std::vector<std::pair<std::string, std::string>>& pairs) {
    std::ostringstream oss;
    oss << "{";
    
    for (size_t i = 0; i < pairs.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << pairs[i].first << "\":" << pairs[i].second;
    }
    
    oss << "}";
    return oss.str();
}

std::string createJSONArray(const std::vector<std::string>& elements) {
    std::ostringstream oss;
    oss << "[";
    
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) oss << ",";
        oss << elements[i];
    }
    
    oss << "]";
    return oss.str();
}

std::string extractJSONValue(const std::string& json, const std::string& key) {
    std::regex value_regex("\"" + key + "\"\\s*:\\s*([^,}]+)");
    std::smatch match;
    if (std::regex_search(json, match, value_regex)) {
        std::string value = match[1].str();
        // Trim whitespace
        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
        return value;
    }
    return "";
}

std::string formatErrorJSON(const std::string& error_message, int error_code) {
    return createJSONObject({
        {"status", "\"error\""},
        {"error_code", std::to_string(error_code)},
        {"message", "\"" + escapeJSON(error_message) + "\""}
    });
}

std::string formatSuccessJSON(const std::string& message, const std::string& data) {
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"status", "\"success\""},
        {"message", "\"" + escapeJSON(message) + "\""}
    };
    
    if (!data.empty()) {
        pairs.emplace_back("data", data);
    }
    
    return createJSONObject(pairs);
}

} // namespace JSONUtils

} // namespace quirkventory