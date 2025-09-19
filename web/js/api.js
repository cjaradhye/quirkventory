// API Client for Quirkventory Dashboard
class APIClient {
    constructor() {
        this.baseURL = CONFIG.API.BASE_URL;
        this.timeout = CONFIG.API.TIMEOUT;
        this.retryAttempts = CONFIG.API.RETRY_ATTEMPTS;
        this.retryDelay = CONFIG.API.RETRY_DELAY;
        this.cache = new Map();
        this.pendingRequests = new Map();
        this.csrfToken = null;
        
        // Initialize CSRF token
        this.initCSRFToken();
        
        // Bind methods to preserve context
        this.request = this.request.bind(this);
        this.get = this.get.bind(this);
        this.post = this.post.bind(this);
        this.put = this.put.bind(this);
        this.delete = this.delete.bind(this);
    }
    
    // Initialize CSRF token
    async initCSRFToken() {
        try {
            const response = await fetch(`${this.baseURL}/api/csrf`, {
                method: 'GET',
                credentials: 'include'
            });
            
            if (response.ok) {
                const data = await response.json();
                this.csrfToken = data.token;
            }
        } catch (error) {
            console.warn('Failed to fetch CSRF token:', error);
        }
    }
    
    // Main request method
    async request(method, endpoint, options = {}) {
        const url = `${this.baseURL}${endpoint}`;
        const requestKey = `${method}:${url}:${JSON.stringify(options.params || {})}`;
        
        // Check for pending identical requests
        if (this.pendingRequests.has(requestKey)) {
            return this.pendingRequests.get(requestKey);
        }
        
        // Check cache for GET requests
        if (method === 'GET' && !options.skipCache) {
            const cachedResponse = this.getFromCache(requestKey);
            if (cachedResponse) {
                return cachedResponse;
            }
        }
        
        const requestPromise = this._executeRequest(method, url, options);
        this.pendingRequests.set(requestKey, requestPromise);
        
        try {
            const response = await requestPromise;
            
            // Cache successful GET responses
            if (method === 'GET' && response.success) {
                this.setCache(requestKey, response);
            }
            
            return response;
        } finally {
            this.pendingRequests.delete(requestKey);
        }
    }
    
    // Execute the actual HTTP request
    async _executeRequest(method, url, options = {}) {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), this.timeout);
        
        try {
            const requestOptions = {
                method,
                headers: this._buildHeaders(options.headers),
                credentials: 'include',
                signal: controller.signal,
                ...this._buildRequestBody(options.body)
            };
            
            // Add query parameters for GET requests
            const requestUrl = method === 'GET' && options.params 
                ? `${url}?${this._buildQueryString(options.params)}`
                : url;
            
            let attempt = 0;
            let lastError;
            
            while (attempt <= this.retryAttempts) {
                try {
                    if (CONFIG.DEBUG.LOG_API_CALLS) {
                        console.log(`API Request [${method}]:`, requestUrl, requestOptions);
                    }
                    
                    const response = await fetch(requestUrl, requestOptions);
                    const result = await this._processResponse(response);
                    
                    if (CONFIG.DEBUG.LOG_API_CALLS) {
                        console.log(`API Response [${method}]:`, requestUrl, result);
                    }
                    
                    return result;
                } catch (error) {
                    lastError = error;
                    attempt++;
                    
                    if (attempt <= this.retryAttempts && this._shouldRetry(error)) {
                        await this._delay(this.retryDelay * attempt);
                        continue;
                    }
                    
                    break;
                }
            }
            
            throw lastError;
        } finally {
            clearTimeout(timeoutId);
        }
    }
    
    // Build request headers
    _buildHeaders(customHeaders = {}) {
        const headers = {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            ...customHeaders
        };
        
        if (this.csrfToken) {
            headers[CONFIG.SECURITY.CSRF_TOKEN_HEADER] = this.csrfToken;
        }
        
        return headers;
    }
    
    // Build request body
    _buildRequestBody(body) {
        if (!body) return {};
        
        if (body instanceof FormData) {
            return { body };
        }
        
        return { body: JSON.stringify(body) };
    }
    
    // Build query string from parameters
    _buildQueryString(params) {
        const searchParams = new URLSearchParams();
        Object.entries(params).forEach(([key, value]) => {
            if (value !== null && value !== undefined) {
                searchParams.append(key, value.toString());
            }
        });
        return searchParams.toString();
    }
    
    // Process response
    async _processResponse(response) {
        const contentType = response.headers.get('content-type');
        let data;
        
        try {
            if (contentType && contentType.includes('application/json')) {
                data = await response.json();
            } else {
                data = await response.text();
            }
        } catch (error) {
            data = null;
        }
        
        if (!response.ok) {
            const error = new APIError(
                data?.message || this._getStatusMessage(response.status),
                response.status,
                data
            );
            throw error;
        }
        
        return {
            success: true,
            data,
            status: response.status,
            headers: Object.fromEntries(response.headers.entries())
        };
    }
    
    // Get status message
    _getStatusMessage(status) {
        const messages = {
            400: CONFIG.MESSAGES.VALIDATION_ERROR,
            401: CONFIG.MESSAGES.UNAUTHORIZED,
            403: CONFIG.MESSAGES.UNAUTHORIZED,
            404: CONFIG.MESSAGES.NOT_FOUND,
            500: CONFIG.MESSAGES.SERVER_ERROR,
            502: CONFIG.MESSAGES.API_ERROR,
            503: CONFIG.MESSAGES.API_ERROR,
            504: CONFIG.MESSAGES.API_ERROR
        };
        
        return messages[status] || CONFIG.MESSAGES.API_ERROR;
    }
    
    // Check if request should be retried
    _shouldRetry(error) {
        if (error.name === 'AbortError') return false;
        if (error instanceof APIError) {
            return error.status >= 500 || error.status === 408 || error.status === 429;
        }
        return true;
    }
    
    // Delay utility
    _delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    
    // Cache management
    getFromCache(key) {
        const cached = this.cache.get(key);
        if (cached && Date.now() - cached.timestamp < CONFIG.DATA.CACHE_DURATION) {
            return cached.data;
        }
        this.cache.delete(key);
        return null;
    }
    
    setCache(key, data) {
        if (this.cache.size >= CONFIG.DATA.MAX_CACHE_SIZE) {
            const firstKey = this.cache.keys().next().value;
            this.cache.delete(firstKey);
        }
        
        this.cache.set(key, {
            data,
            timestamp: Date.now()
        });
    }
    
    clearCache() {
        this.cache.clear();
    }
    
    // HTTP method shortcuts
    get(endpoint, params = {}, options = {}) {
        return this.request('GET', endpoint, { params, ...options });
    }
    
    post(endpoint, body, options = {}) {
        return this.request('POST', endpoint, { body, ...options });
    }
    
    put(endpoint, body, options = {}) {
        return this.request('PUT', endpoint, { body, ...options });
    }
    
    delete(endpoint, options = {}) {
        return this.request('DELETE', endpoint, options);
    }
    
    // Specific API methods
    async getProducts(filters = {}) {
        return this.get(CONFIG.API.ENDPOINTS.PRODUCTS, filters);
    }
    
    async getProduct(id) {
        return this.get(`${CONFIG.API.ENDPOINTS.PRODUCTS}/${id}`);
    }
    
    async createProduct(productData) {
        return this.post(CONFIG.API.ENDPOINTS.PRODUCTS, productData);
    }
    
    async updateProduct(id, productData) {
        return this.put(`${CONFIG.API.ENDPOINTS.PRODUCTS}/${id}`, productData);
    }
    
    async deleteProduct(id) {
        return this.delete(`${CONFIG.API.ENDPOINTS.PRODUCTS}/${id}`);
    }
    
    async getInventory(filters = {}) {
        return this.get(CONFIG.API.ENDPOINTS.INVENTORY, filters);
    }
    
    async updateInventory(id, inventoryData) {
        return this.put(`${CONFIG.API.ENDPOINTS.INVENTORY}/${id}`, inventoryData);
    }
    
    async getOrders(filters = {}) {
        return this.get(CONFIG.API.ENDPOINTS.ORDERS, filters);
    }
    
    async getOrder(id) {
        return this.get(`${CONFIG.API.ENDPOINTS.ORDERS}/${id}`);
    }
    
    async createOrder(orderData) {
        return this.post(CONFIG.API.ENDPOINTS.ORDERS, orderData);
    }
    
    async updateOrder(id, orderData) {
        return this.put(`${CONFIG.API.ENDPOINTS.ORDERS}/${id}`, orderData);
    }
    
    async cancelOrder(id) {
        return this.post(`${CONFIG.API.ENDPOINTS.ORDERS}/${id}/cancel`);
    }
    
    async getDashboardStats() {
        return this.get(CONFIG.API.ENDPOINTS.DASHBOARD);
    }
    
    async getStats(timeRange = '7d') {
        return this.get(CONFIG.API.ENDPOINTS.STATS, { range: timeRange });
    }
    
    async getReports(type, filters = {}) {
        return this.get(`${CONFIG.API.ENDPOINTS.REPORTS}/${type}`, filters);
    }
    
    async generateReport(type, params = {}) {
        return this.post(`${CONFIG.API.ENDPOINTS.REPORTS}/${type}/generate`, params);
    }
    
    async getAlerts(filters = {}) {
        return this.get(CONFIG.API.ENDPOINTS.ALERTS, filters);
    }
    
    async markAlertRead(id) {
        return this.post(`${CONFIG.API.ENDPOINTS.ALERTS}/${id}/read`);
    }
    
    async getNotifications(filters = {}) {
        return this.get(CONFIG.API.ENDPOINTS.NOTIFICATIONS, filters);
    }
    
    async searchProducts(query, filters = {}) {
        return this.get(`${CONFIG.API.ENDPOINTS.PRODUCTS}/search`, { 
            q: query, 
            ...filters 
        });
    }
    
    async exportData(type, format = 'csv', filters = {}) {
        const response = await this.get(`/api/export/${type}`, {
            format,
            ...filters
        }, {
            headers: {
                'Accept': format === 'csv' ? 'text/csv' : 'application/octet-stream'
            }
        });
        
        return response;
    }
    
    // Health check
    async healthCheck() {
        try {
            const response = await this.get('/api/health');
            return response.success;
        } catch (error) {
            return false;
        }
    }
}

// Custom error class for API errors
class APIError extends Error {
    constructor(message, status, data = null) {
        super(message);
        this.name = 'APIError';
        this.status = status;
        this.data = data;
    }
}

// Create global API instance
const api = new APIClient();

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { APIClient, APIError, api };
}

// Global access
window.api = api;
window.APIError = APIError;