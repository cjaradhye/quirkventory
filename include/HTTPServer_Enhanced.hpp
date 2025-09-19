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
#include <mutex>
#include <set>
#include <vector>
#include <sstream>
#include <regex>

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
 * @brief WebSocket connection for real-time updates
 */
class WebSocketConnection {
public:
    WebSocketConnection(const std::string& id);
    ~WebSocketConnection();
    
    const std::string& getId() const { return connection_id_; }
    bool isActive() const { return active_; }
    void close();
    
    // Send data to client (simulated)
    bool sendMessage(const std::string& message);
    bool sendJSON(const std::string& json_data);
    
    // Event subscriptions
    void subscribeToInventoryUpdates();
    void subscribeToOrderUpdates();
    void subscribeToLowStockAlerts();
    void unsubscribeFromAll();
    
    // Check subscriptions
    bool isSubscribedTo(const std::string& event_type) const;
    
private:
    std::string connection_id_;
    std::atomic<bool> active_;
    std::mutex subscription_mutex_;
    std::set<std::string> subscriptions_;
};

/**
 * @brief Real-time event manager for WebSocket communications
 */
class RealTimeEventManager {
public:
    RealTimeEventManager();
    ~RealTimeEventManager();
    
    // Connection management
    void addConnection(std::shared_ptr<WebSocketConnection> connection);
    void removeConnection(const std::string& connection_id);
    std::shared_ptr<WebSocketConnection> getConnection(const std::string& connection_id);
    size_t getActiveConnectionCount() const;
    
    // Event broadcasting
    void broadcastInventoryUpdate(const std::string& product_id, int new_quantity);
    void broadcastOrderUpdate(const std::string& order_id, const std::string& status);
    void broadcastLowStockAlert(const std::string& product_id, const std::string& product_name, int quantity);
    void broadcastSystemAlert(const std::string& message, const std::string& priority = "medium");
    
    // Custom event broadcasting
    void broadcastToSubscribers(const std::string& event_type, const std::string& data);
    
private:
    mutable std::mutex connections_mutex_;
    std::unordered_map<std::string, std::shared_ptr<WebSocketConnection>> connections_;
    std::atomic<size_t> connection_counter_;
    
    void cleanupInactiveConnections();
};

/**
 * @brief HTTP request handler function type
 */
using HTTPRequestHandler = std::function<HTTPResponse(const HTTPRequest&)>;

/**
 * @brief Enhanced HTTP server with REST API and real-time capabilities
 * 
 * This class provides a complete HTTP server implementation for the inventory
 * management system, including REST API endpoints and WebSocket support for
 * real-time updates to web dashboards.
 */
class HTTPServer {
public:
    HTTPServer(const std::string& host = "localhost", int port = 8080);
    ~HTTPServer();
    
    // Server lifecycle
    bool start();
    void stop();
    std::string getServerUrl() const;
    bool isRunning() const { return running_.load(); }
    
    // System integration
    void setSystemComponents(Inventory* inventory,
                           OrderManager* order_manager,
                           UserManager* user_manager,
                           NotificationManager* notification_manager);
    
    // Request handling
    HTTPResponse handleRequest(const std::string& request_data);
    
    // WebSocket support for real-time updates
    std::string createWebSocketConnection();
    bool closeWebSocketConnection(const std::string& connection_id);
    std::shared_ptr<WebSocketConnection> getWebSocketConnection(const std::string& connection_id);
    
    // Real-time event management
    RealTimeEventManager& getRealTimeEventManager() { return *event_manager_; }
    
    // CORS support for web dashboard
    void enableCORS(bool enable = true) { cors_enabled_ = enable; }
    void addCORSHeaders(HTTPResponse& response) const;
    
private:
    std::string host_;
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
    
    // System components
    Inventory* inventory_;
    OrderManager* order_manager_;
    UserManager* user_manager_;
    NotificationManager* notification_manager_;
    
    // Real-time support
    std::unique_ptr<RealTimeEventManager> event_manager_;
    
    // Configuration
    bool cors_enabled_;
    
    // Route handlers
    std::unordered_map<std::string, HTTPRequestHandler> get_handlers_;
    std::unordered_map<std::string, HTTPRequestHandler> post_handlers_;
    std::unordered_map<std::string, HTTPRequestHandler> put_handlers_;
    std::unordered_map<std::string, HTTPRequestHandler> delete_handlers_;
    
    // Server methods
    void setupRoutes();
    void serverLoop();
    HTTPRequest parseRequest(const std::string& request_data);
    HTTPResponse routeRequest(const HTTPRequest& request);
    HTTPResponse createErrorResponse(int status_code, const std::string& message);
    HTTPResponse createJSONResponse(const std::string& data);
    
    // Standard API endpoint handlers
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
    
    HTTPResponse handleGetSalesReport(const HTTPRequest& request);
    HTTPResponse handleGetInventoryReport(const HTTPRequest& request);
    
    HTTPResponse handleGetSystemStatus(const HTTPRequest& request);
    
    // Dashboard-specific endpoints
    HTTPResponse handleGetDashboard(const HTTPRequest& request);
    HTTPResponse handleGetInventoryChart(const HTTPRequest& request);
    HTTPResponse handleGetRealTimeInventory(const HTTPRequest& request);
    HTTPResponse handleGetRealTimeAlerts(const HTTPRequest& request);
    HTTPResponse handleGetRealTimeStats(const HTTPRequest& request);
    
    // WebSocket handlers
    HTTPResponse handleWebSocketUpgrade(const HTTPRequest& request);
    HTTPResponse handleWebSocketMessage(const HTTPRequest& request);
    
    // Utility methods
    std::string extractPathParameter(const std::string& path, const std::string& pattern);
    std::string productToJSON(const Product* product);
    std::string orderToJSON(const Order& order);
    std::string parseJSONString(const std::string& json, const std::string& key);
    double parseJSONDouble(const std::string& json, const std::string& key);
    int parseJSONInt(const std::string& json, const std::string& key);
    
    // Dashboard data generation
    std::string generateDashboardData();
    std::string generateInventoryChartData();
    std::string generateSalesChartData();
    std::string generateLowStockData();
    std::string generateRecentOrdersData();
};

/**
 * @brief JSON utilities namespace
 * 
 * Provides helper functions for JSON creation, parsing, and formatting
 * specifically designed for the inventory management system's API responses.
 */
namespace JSONUtils {
    // Basic JSON operations
    std::string escapeJSON(const std::string& str);
    std::string createJSONObject(const std::vector<std::pair<std::string, std::string>>& pairs);
    std::string createJSONArray(const std::vector<std::string>& elements);
    std::string extractJSONValue(const std::string& json, const std::string& key);
    
    // API response formatters
    std::string formatErrorJSON(const std::string& error_message, int error_code = 400);
    std::string formatSuccessJSON(const std::string& message, const std::string& data = "");
    
    // Real-time event JSON formatters
    std::string formatInventoryUpdateJSON(const std::string& product_id, int new_quantity, const std::string& timestamp = "");
    std::string formatOrderUpdateJSON(const std::string& order_id, const std::string& status, const std::string& timestamp = "");
    std::string formatLowStockAlertJSON(const std::string& product_id, const std::string& product_name, int quantity);
    std::string formatSystemAlertJSON(const std::string& message, const std::string& priority, const std::string& timestamp = "");
    
    // Dashboard data formatters
    std::string formatDashboardStatsJSON(int total_products, int total_quantity, double total_value, int low_stock_count);
    std::string formatChartDataJSON(const std::vector<std::pair<std::string, int>>& data);
    std::string formatRecentOrderJSON(const std::string& order_id, const std::string& status, const std::string& timestamp, double total);
}

} // namespace quirkventory