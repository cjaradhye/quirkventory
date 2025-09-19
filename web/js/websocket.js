// WebSocket Client for Real-time Communication
class WebSocketClient {
    constructor() {
        this.url = CONFIG.WEBSOCKET.URL;
        this.socket = null;
        this.isConnected = false;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = CONFIG.WEBSOCKET.MAX_RECONNECT_ATTEMPTS;
        this.reconnectInterval = CONFIG.WEBSOCKET.RECONNECT_INTERVAL;
        this.heartbeatInterval = null;
        this.pingTimeout = null;
        
        // Event listeners
        this.eventListeners = new Map();
        this.subscriptions = new Set();
        
        // Connection state callbacks
        this.onConnect = null;
        this.onDisconnect = null;
        this.onReconnect = null;
        this.onError = null;
        
        // Message queue for offline messages
        this.messageQueue = [];
        this.maxQueueSize = 100;
        
        // Bind methods
        this.connect = this.connect.bind(this);
        this.disconnect = this.disconnect.bind(this);
        this.send = this.send.bind(this);
        this.subscribe = this.subscribe.bind(this);
        this.unsubscribe = this.unsubscribe.bind(this);
        
        // Auto-connect if real-time updates are enabled
        if (CONFIG.FEATURES.REAL_TIME_UPDATES) {
            this.connect();
        }
    }
    
    // Connect to WebSocket server
    connect() {
        if (this.socket && this.socket.readyState === WebSocket.OPEN) {
            return Promise.resolve();
        }
        
        return new Promise((resolve, reject) => {
            try {
                if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
                    console.log('Connecting to WebSocket:', this.url);
                }
                
                this.socket = new WebSocket(this.url);
                
                this.socket.onopen = (event) => {
                    this.isConnected = true;
                    this.reconnectAttempts = 0;
                    
                    if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
                        console.log('WebSocket connected:', event);
                    }
                    
                    // Start heartbeat
                    this.startHeartbeat();
                    
                    // Send queued messages
                    this.processMessageQueue();
                    
                    // Resubscribe to channels
                    this.resubscribe();
                    
                    // Update UI connection status
                    this.updateConnectionStatus('online');
                    
                    // Trigger connect callback
                    if (this.onConnect) {
                        this.onConnect(event);
                    }
                    
                    resolve();
                };
                
                this.socket.onmessage = (event) => {
                    this.handleMessage(event);
                };
                
                this.socket.onclose = (event) => {
                    this.isConnected = false;
                    this.stopHeartbeat();
                    
                    if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
                        console.log('WebSocket disconnected:', event);
                    }
                    
                    // Update UI connection status
                    this.updateConnectionStatus('offline');
                    
                    // Trigger disconnect callback
                    if (this.onDisconnect) {
                        this.onDisconnect(event);
                    }
                    
                    // Attempt reconnection if not a normal closure
                    if (event.code !== 1000 && event.code !== 1001) {
                        this.attemptReconnect();
                    }
                };
                
                this.socket.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    
                    // Trigger error callback
                    if (this.onError) {
                        this.onError(error);
                    }
                    
                    reject(error);
                };
                
            } catch (error) {
                console.error('Failed to create WebSocket connection:', error);
                reject(error);
            }
        });
    }
    
    // Disconnect from WebSocket server
    disconnect() {
        if (this.socket) {
            this.socket.close(1000, 'Client disconnect');
            this.socket = null;
        }
        
        this.isConnected = false;
        this.stopHeartbeat();
        this.updateConnectionStatus('offline');
    }
    
    // Attempt to reconnect
    attemptReconnect() {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            console.error('Max reconnection attempts reached');
            this.updateConnectionStatus('offline');
            return;
        }
        
        this.reconnectAttempts++;
        this.updateConnectionStatus('connecting');
        
        if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
            console.log(`Attempting to reconnect (${this.reconnectAttempts}/${this.maxReconnectAttempts})...`);
        }
        
        setTimeout(() => {
            this.connect().then(() => {
                if (this.onReconnect) {
                    this.onReconnect();
                }
            }).catch(() => {
                this.attemptReconnect();
            });
        }, this.reconnectInterval);
    }
    
    // Handle incoming messages
    handleMessage(event) {
        try {
            const message = JSON.parse(event.data);
            
            if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
                console.log('WebSocket message received:', message);
            }
            
            // Handle heartbeat pong
            if (message.type === 'pong') {
                this.handlePong();
                return;
            }
            
            // Handle system messages
            if (message.type === 'system') {
                this.handleSystemMessage(message);
                return;
            }
            
            // Trigger event listeners
            this.triggerEvent(message.type, message.data);
            
        } catch (error) {
            console.error('Failed to parse WebSocket message:', error);
        }
    }
    
    // Handle system messages
    handleSystemMessage(message) {
        switch (message.data.action) {
            case 'server_restart':
                console.log('Server restart notification received');
                break;
                
            case 'maintenance_mode':
                console.log('Maintenance mode notification received');
                break;
                
            default:
                console.log('Unknown system message:', message);
        }
    }
    
    // Send message to server
    send(message) {
        if (!this.isConnected || !this.socket) {
            // Queue message for later sending
            if (this.messageQueue.length < this.maxQueueSize) {
                this.messageQueue.push(message);
            }
            return false;
        }
        
        try {
            const messageString = typeof message === 'string' 
                ? message 
                : JSON.stringify(message);
                
            this.socket.send(messageString);
            
            if (CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS) {
                console.log('WebSocket message sent:', message);
            }
            
            return true;
        } catch (error) {
            console.error('Failed to send WebSocket message:', error);
            return false;
        }
    }
    
    // Subscribe to event channel
    subscribe(channel) {
        this.subscriptions.add(channel);
        
        if (this.isConnected) {
            this.send({
                type: 'subscribe',
                channel: channel
            });
        }
    }
    
    // Unsubscribe from event channel
    unsubscribe(channel) {
        this.subscriptions.delete(channel);
        
        if (this.isConnected) {
            this.send({
                type: 'unsubscribe',
                channel: channel
            });
        }
    }
    
    // Resubscribe to all channels after reconnection
    resubscribe() {
        this.subscriptions.forEach(channel => {
            this.send({
                type: 'subscribe',
                channel: channel
            });
        });
    }
    
    // Process queued messages
    processMessageQueue() {
        while (this.messageQueue.length > 0) {
            const message = this.messageQueue.shift();
            this.send(message);
        }
    }
    
    // Add event listener
    addEventListener(eventType, callback) {
        if (!this.eventListeners.has(eventType)) {
            this.eventListeners.set(eventType, new Set());
        }
        
        this.eventListeners.get(eventType).add(callback);
    }
    
    // Remove event listener
    removeEventListener(eventType, callback) {
        if (this.eventListeners.has(eventType)) {
            this.eventListeners.get(eventType).delete(callback);
        }
    }
    
    // Trigger event listeners
    triggerEvent(eventType, data) {
        if (this.eventListeners.has(eventType)) {
            this.eventListeners.get(eventType).forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error('Error in WebSocket event listener:', error);
                }
            });
        }
    }
    
    // Start heartbeat mechanism
    startHeartbeat() {
        this.stopHeartbeat();
        
        this.heartbeatInterval = setInterval(() => {
            if (this.isConnected) {
                this.send({ type: 'ping' });
                
                // Set timeout for pong response
                this.pingTimeout = setTimeout(() => {
                    console.log('Heartbeat timeout - connection may be lost');
                    this.socket.close();
                }, CONFIG.WEBSOCKET.PING_TIMEOUT);
            }
        }, CONFIG.WEBSOCKET.HEARTBEAT_INTERVAL);
    }
    
    // Stop heartbeat mechanism
    stopHeartbeat() {
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
            this.heartbeatInterval = null;
        }
        
        if (this.pingTimeout) {
            clearTimeout(this.pingTimeout);
            this.pingTimeout = null;
        }
    }
    
    // Handle pong response
    handlePong() {
        if (this.pingTimeout) {
            clearTimeout(this.pingTimeout);
            this.pingTimeout = null;
        }
    }
    
    // Update connection status in UI
    updateConnectionStatus(status) {
        const statusIndicator = document.getElementById('statusIndicator');
        const statusText = document.getElementById('statusText');
        
        if (statusIndicator && statusText) {
            statusIndicator.className = `status-indicator ${status}`;
            
            switch (status) {
                case 'online':
                    statusText.textContent = 'Connected';
                    break;
                case 'offline':
                    statusText.textContent = 'Disconnected';
                    break;
                case 'connecting':
                    statusText.textContent = 'Connecting...';
                    break;
                default:
                    statusText.textContent = 'Unknown';
            }
        }
    }
    
    // Get connection status
    getConnectionStatus() {
        return {
            connected: this.isConnected,
            reconnectAttempts: this.reconnectAttempts,
            subscriptions: Array.from(this.subscriptions),
            queuedMessages: this.messageQueue.length
        };
    }
    
    // Subscribe to specific data events
    subscribeToInventoryUpdates() {
        this.subscribe('inventory_updates');
        this.addEventListener(CONFIG.DATA.REAL_TIME_EVENTS.INVENTORY_CHANGED, (data) => {
            if (window.InventoryManager) {
                window.InventoryManager.handleInventoryUpdate(data);
            }
        });
    }
    
    subscribeToOrderUpdates() {
        this.subscribe('order_updates');
        this.addEventListener(CONFIG.DATA.REAL_TIME_EVENTS.ORDER_CREATED, (data) => {
            if (window.OrderManager) {
                window.OrderManager.handleOrderCreated(data);
            }
        });
        
        this.addEventListener(CONFIG.DATA.REAL_TIME_EVENTS.ORDER_UPDATED, (data) => {
            if (window.OrderManager) {
                window.OrderManager.handleOrderUpdated(data);
            }
        });
    }
    
    subscribeToAlerts() {
        this.subscribe('system_alerts');
        this.addEventListener(CONFIG.DATA.REAL_TIME_EVENTS.ALERT_TRIGGERED, (data) => {
            if (window.AlertManager) {
                window.AlertManager.handleNewAlert(data);
            }
        });
    }
    
    subscribeToProductUpdates() {
        this.subscribe('product_updates');
        this.addEventListener(CONFIG.DATA.REAL_TIME_EVENTS.PRODUCT_UPDATED, (data) => {
            if (window.DashboardManager) {
                window.DashboardManager.handleProductUpdate(data);
            }
        });
    }
}

// Create global WebSocket instance
const wsClient = new WebSocketClient();

// Set up connection event handlers
wsClient.onConnect = () => {
    console.log('Real-time connection established');
    
    // Subscribe to all relevant channels
    wsClient.subscribeToInventoryUpdates();
    wsClient.subscribeToOrderUpdates();
    wsClient.subscribeToAlerts();
    wsClient.subscribeToProductUpdates();
};

wsClient.onDisconnect = () => {
    console.log('Real-time connection lost');
};

wsClient.onReconnect = () => {
    console.log('Real-time connection restored');
};

wsClient.onError = (error) => {
    console.error('Real-time connection error:', error);
};

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { WebSocketClient, wsClient };
}

// Global access
window.wsClient = wsClient;