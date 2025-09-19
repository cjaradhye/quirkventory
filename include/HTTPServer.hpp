#pragma once

#include "Product.hpp"
#include "Inventory.hpp"
#include "Order.hpp"
#include "User.hpp"
#include "NotificationSystem.hpp"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>

namespace quirkventory {

/**
 * @brief HTTP request structure
 */
struct HTTPRequest {
    std::string method;          // GET, POST, PUT, DELETE
    std::string path;           // URL path
    std::string query_string;   // Query parameters
    std::unordered_map<std::string, std::string> headers;
    std::string body;           // Request body
    
    // Helper method to get query parameter
    std::string getQueryParam(const std::string& key) const;
};

/**
 * @brief HTTP response structure
 */
struct HTTPResponse {
    int status_code;
    std::string status_message;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    HTTPResponse(int code = 200, const std::string& message = "OK");
    
    // Helper methods
    void setContentType(const std::string& content_type);
    void setBody(const std::string& content, const std::string& content_type = "text/plain");
    void setJSONBody(const std::string& json_content);
    std::string toString() const;
};

/**
 * @brief HTTP request handler function type
 */
using RequestHandler = std::function<HTTPResponse(const HTTPRequest&)>;

/**
 * @brief Simple HTTP server for REST API endpoints
 * 
 * Provides a lightweight HTTP server implementation for exposing
 * inventory management system functionality via REST API.
 * 
 * Note: This is a simplified implementation for demonstration purposes.
 * In production, use a robust HTTP library like Crow, Pistache, or cpp-httplib.
 */
class HTTPServer {
private:
    std::string host_;
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    // Route handlers
    std::unordered_map<std::string, RequestHandler> get_handlers_;
    std::unordered_map<std::string, RequestHandler> post_handlers_;
    std::unordered_map<std::string, RequestHandler> put_handlers_;
    std::unordered_map<std::string, RequestHandler> delete_handlers_;
    
    // System components
    Inventory* inventory_;
    OrderManager* order_manager_;
    UserManager* user_manager_;
    NotificationManager* notification_manager_;

public:
    /**
     * @brief Constructor
     * @param host Server host address
     * @param port Server port
     */
    HTTPServer(const std::string& host = "localhost", int port = 8080);

    /**
     * @brief Destructor
     */
    ~HTTPServer();

    /**
     * @brief Set system components
     * @param inventory Inventory system
     * @param order_manager Order management system
     * @param user_manager User management system
     * @param notification_manager Notification system
     */
    void setSystemComponents(Inventory* inventory,
                           OrderManager* order_manager,
                           UserManager* user_manager,
                           NotificationManager* notification_manager);

    /**
     * @brief Start the HTTP server
     * @return true if server started successfully
     */
    bool start();

    /**
     * @brief Stop the HTTP server
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief Get server URL
     * @return Server URL string
     */
    std::string getServerUrl() const;

private:
    /**
     * @brief Setup REST API routes
     */
    void setupRoutes();

    /**
     * @brief Main server loop
     */
    void serverLoop();

    /**
     * @brief Handle incoming HTTP request
     * @param request_data Raw request data
     * @return HTTP response
     */
    HTTPResponse handleRequest(const std::string& request_data);

    /**
     * @brief Parse HTTP request from raw data
     * @param request_data Raw request string
     * @return Parsed HTTPRequest
     */
    HTTPRequest parseRequest(const std::string& request_data);

    /**
     * @brief Route request to appropriate handler
     * @param request HTTP request
     * @return HTTP response
     */
    HTTPResponse routeRequest(const HTTPRequest& request);

    /**
     * @brief Create error response
     * @param status_code HTTP status code
     * @param message Error message
     * @return Error response
     */
    HTTPResponse createErrorResponse(int status_code, const std::string& message);

    /**
     * @brief Create JSON success response
     * @param data JSON data
     * @return Success response
     */
    HTTPResponse createJSONResponse(const std::string& data);

    // API endpoint handlers
    HTTPResponse handleGetProducts(const HTTPRequest& request);
    HTTPResponse handleGetProduct(const HTTPRequest& request);
    HTTPResponse handlePostProduct(const HTTPRequest& request);
    HTTPResponse handlePutProduct(const HTTPRequest& request);
    HTTPResponse handleDeleteProduct(const HTTPRequest& request);
    
    HTTPResponse handleGetInventoryStatus(const HTTPRequest& request);
    HTTPResponse handleGetLowStockAlerts(const HTTPRequest& request);
    HTTPResponse handleGetExpiryAlerts(const HTTPRequest& request);
    
    HTTPResponse handleGetOrders(const HTTPRequest& request);
    HTTPResponse handleGetOrder(const HTTPRequest& request);
    HTTPResponse handlePostOrder(const HTTPRequest& request);
    HTTPResponse handlePutOrder(const HTTPRequest& request);
    
    HTTPResponse handleGetSalesReport(const HTTPRequest& request);
    HTTPResponse handleGetInventoryReport(const HTTPRequest& request);
    
    HTTPResponse handleGetUsers(const HTTPRequest& request);
    HTTPResponse handlePostUser(const HTTPRequest& request);
    
    HTTPResponse handleGetSystemStatus(const HTTPRequest& request);

    // Utility methods
    std::string extractPathParameter(const std::string& path, const std::string& pattern);
    std::string productToJSON(const Product* product);
    std::string orderToJSON(const Order* order);
    std::string userToJSON(const User* user);
    std::string parseJSONString(const std::string& json, const std::string& key);
    double parseJSONDouble(const std::string& json, const std::string& key);
    int parseJSONInt(const std::string& json, const std::string& key);
};

/**
 * @brief JSON utility functions for simple JSON handling
 * 
 * Note: This is a simplified JSON implementation for demonstration.
 * In production, use a proper JSON library like nlohmann/json or rapidjson.
 */
namespace JSONUtils {
    /**
     * @brief Escape string for JSON
     * @param str String to escape
     * @return Escaped string
     */
    std::string escapeJSON(const std::string& str);

    /**
     * @brief Create JSON object string
     * @param pairs Key-value pairs
     * @return JSON object string
     */
    std::string createJSONObject(const std::vector<std::pair<std::string, std::string>>& pairs);

    /**
     * @brief Create JSON array string
     * @param elements Array elements
     * @return JSON array string
     */
    std::string createJSONArray(const std::vector<std::string>& elements);

    /**
     * @brief Extract value from simple JSON
     * @param json JSON string
     * @param key Key to extract
     * @return Extracted value or empty string if not found
     */
    std::string extractJSONValue(const std::string& json, const std::string& key);

    /**
     * @brief Format error response as JSON
     * @param error_message Error message
     * @param error_code Error code
     * @return JSON error response
     */
    std::string formatErrorJSON(const std::string& error_message, int error_code = 0);

    /**
     * @brief Format success response as JSON
     * @param message Success message
     * @param data Optional data object
     * @return JSON success response
     */
    std::string formatSuccessJSON(const std::string& message, const std::string& data = "");
}

} // namespace quirkventory