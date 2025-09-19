// Configuration settings for the Quirkventory Dashboard
const CONFIG = {
    // API Configuration
    API: {
        BASE_URL: 'http://localhost:8080',
        ENDPOINTS: {
            PRODUCTS: '/api/products',
            INVENTORY: '/api/inventory',
            ORDERS: '/api/orders',
            USERS: '/api/users',
            REPORTS: '/api/reports',
            NOTIFICATIONS: '/api/notifications',
            ALERTS: '/api/alerts',
            DASHBOARD: '/api/dashboard',
            STATS: '/api/stats'
        },
        TIMEOUT: 10000, // 10 seconds
        RETRY_ATTEMPTS: 3,
        RETRY_DELAY: 1000 // 1 second
    },
    
    // WebSocket Configuration
    WEBSOCKET: {
        URL: 'ws://localhost:8080/ws',
        RECONNECT_INTERVAL: 5000, // 5 seconds
        MAX_RECONNECT_ATTEMPTS: 10,
        HEARTBEAT_INTERVAL: 30000, // 30 seconds
        PING_TIMEOUT: 5000 // 5 seconds
    },
    
    // UI Configuration
    UI: {
        ANIMATION_DURATION: 300,
        NOTIFICATION_DURATION: 5000, // 5 seconds
        REFRESH_INTERVAL: 30000, // 30 seconds
        AUTO_SAVE_DELAY: 2000, // 2 seconds
        SEARCH_DEBOUNCE: 500, // 500ms
        PAGINATION: {
            DEFAULT_PAGE_SIZE: 20,
            PAGE_SIZE_OPTIONS: [10, 20, 50, 100]
        }
    },
    
    // Chart Configuration
    CHARTS: {
        DEFAULT_THEME: 'default',
        ANIMATION_DURATION: 750,
        RESPONSIVE_BREAKPOINT: 768,
        COLORS: {
            PRIMARY: '#2563eb',
            SECONDARY: '#64748b',
            SUCCESS: '#10b981',
            WARNING: '#f59e0b',
            ERROR: '#ef4444',
            INFO: '#06b6d4',
            ACCENT: '#8b5cf6',
            NEUTRAL: '#6b7280'
        },
        GRADIENTS: {
            PRIMARY: ['#3b82f6', '#1d4ed8'],
            SUCCESS: ['#10b981', '#059669'],
            WARNING: ['#f59e0b', '#d97706'],
            ERROR: ['#ef4444', '#dc2626']
        }
    },
    
    // Data Configuration
    DATA: {
        CACHE_DURATION: 300000, // 5 minutes
        MAX_CACHE_SIZE: 50, // Maximum number of cached responses
        REAL_TIME_EVENTS: {
            PRODUCT_UPDATED: 'product_updated',
            INVENTORY_CHANGED: 'inventory_changed',
            ORDER_CREATED: 'order_created',
            ORDER_UPDATED: 'order_updated',
            ALERT_TRIGGERED: 'alert_triggered',
            USER_ACTION: 'user_action',
            SYSTEM_STATUS: 'system_status'
        }
    },
    
    // Feature Flags
    FEATURES: {
        REAL_TIME_UPDATES: true,
        NOTIFICATIONS: true,
        ADVANCED_CHARTS: true,
        EXPORT_FUNCTIONALITY: true,
        OFFLINE_MODE: false,
        DARK_MODE: false,
        AUTO_REFRESH: true,
        KEYBOARD_SHORTCUTS: true
    },
    
    // Validation Rules
    VALIDATION: {
        PRODUCT: {
            ID: {
                MIN_LENGTH: 3,
                MAX_LENGTH: 20,
                PATTERN: /^[A-Z0-9_-]+$/
            },
            NAME: {
                MIN_LENGTH: 2,
                MAX_LENGTH: 100
            },
            PRICE: {
                MIN: 0.01,
                MAX: 999999.99
            },
            QUANTITY: {
                MIN: 0,
                MAX: 999999
            }
        },
        ORDER: {
            ID: {
                MIN_LENGTH: 5,
                MAX_LENGTH: 15,
                PATTERN: /^ORD[0-9]{6,12}$/
            },
            ITEMS: {
                MIN_COUNT: 1,
                MAX_COUNT: 100
            }
        }
    },
    
    // Error Messages
    MESSAGES: {
        API_ERROR: 'Unable to connect to server. Please try again.',
        NETWORK_ERROR: 'Network connection error. Please check your internet connection.',
        VALIDATION_ERROR: 'Please check your input and try again.',
        UNAUTHORIZED: 'You are not authorized to perform this action.',
        NOT_FOUND: 'The requested resource was not found.',
        SERVER_ERROR: 'Server error occurred. Please try again later.',
        WEBSOCKET_ERROR: 'Real-time connection lost. Attempting to reconnect...',
        SUCCESS: 'Operation completed successfully.',
        WARNING: 'Warning: Please review your action.',
        INFO: 'Information updated.'
    },
    
    // Keyboard Shortcuts
    SHORTCUTS: {
        SEARCH: 'Ctrl+K',
        ADD_PRODUCT: 'Ctrl+N',
        REFRESH: 'F5',
        EXPORT: 'Ctrl+E',
        HELP: 'F1',
        NAVIGATE_UP: 'ArrowUp',
        NAVIGATE_DOWN: 'ArrowDown',
        SELECT: 'Enter',
        CANCEL: 'Escape'
    },
    
    // Storage Keys
    STORAGE_KEYS: {
        USER_PREFERENCES: 'quirkventory_user_prefs',
        CACHE_PREFIX: 'quirkventory_cache_',
        SESSION_DATA: 'quirkventory_session',
        FILTERS: 'quirkventory_filters',
        SORT_PREFERENCES: 'quirkventory_sort'
    },
    
    // Debug Configuration
    DEBUG: {
        ENABLED: false, // Set to true for development
        LOG_LEVEL: 'info', // 'debug', 'info', 'warn', 'error'
        LOG_API_CALLS: false,
        LOG_WEBSOCKET_EVENTS: false,
        PERFORMANCE_MONITORING: false
    },
    
    // Performance Configuration
    PERFORMANCE: {
        VIRTUALIZATION_THRESHOLD: 100, // Enable virtualization for tables with more than 100 rows
        DEBOUNCE_INTERVALS: {
            SEARCH: 300,
            RESIZE: 100,
            SCROLL: 16
        },
        LAZY_LOADING: {
            ENABLED: true,
            THRESHOLD: 200 // pixels
        }
    },
    
    // Accessibility Configuration
    ACCESSIBILITY: {
        HIGH_CONTRAST: false,
        REDUCED_MOTION: false,
        SCREEN_READER_ANNOUNCEMENTS: true,
        KEYBOARD_NAVIGATION: true,
        FOCUS_INDICATORS: true
    },
    
    // Export Configuration
    EXPORT: {
        FORMATS: ['csv', 'xlsx', 'pdf', 'json'],
        MAX_RECORDS: 10000,
        FILENAME_PREFIX: 'quirkventory_',
        DATE_FORMAT: 'YYYY-MM-DD'
    },
    
    // Security Configuration
    SECURITY: {
        CSRF_TOKEN_HEADER: 'X-CSRF-Token',
        SESSION_TIMEOUT: 3600000, // 1 hour
        MAX_LOGIN_ATTEMPTS: 5,
        LOCKOUT_DURATION: 300000 // 5 minutes
    }
};

// Environment-specific overrides
if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') {
    CONFIG.DEBUG.ENABLED = true;
    CONFIG.DEBUG.LOG_API_CALLS = true;
    CONFIG.DEBUG.LOG_WEBSOCKET_EVENTS = true;
}

// Runtime configuration updates
CONFIG.updateAPIBaseUrl = function(newUrl) {
    this.API.BASE_URL = newUrl;
    this.WEBSOCKET.URL = newUrl.replace('http', 'ws') + '/ws';
};

CONFIG.enableDebugMode = function() {
    this.DEBUG.ENABLED = true;
    this.DEBUG.LOG_API_CALLS = true;
    this.DEBUG.LOG_WEBSOCKET_EVENTS = true;
    this.DEBUG.PERFORMANCE_MONITORING = true;
    console.log('Debug mode enabled');
};

CONFIG.disableDebugMode = function() {
    this.DEBUG.ENABLED = false;
    this.DEBUG.LOG_API_CALLS = false;
    this.DEBUG.LOG_WEBSOCKET_EVENTS = false;
    this.DEBUG.PERFORMANCE_MONITORING = false;
    console.log('Debug mode disabled');
};

// Feature detection and configuration
CONFIG.detectFeatures = function() {
    // Check for WebSocket support
    this.FEATURES.REAL_TIME_UPDATES = 'WebSocket' in window;
    
    // Check for modern features
    this.FEATURES.OFFLINE_MODE = 'serviceWorker' in navigator;
    
    // Check for reduced motion preference
    if (window.matchMedia) {
        this.ACCESSIBILITY.REDUCED_MOTION = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    }
    
    // Check for high contrast preference
    if (window.matchMedia) {
        this.ACCESSIBILITY.HIGH_CONTRAST = window.matchMedia('(prefers-contrast: high)').matches;
    }
    
    // Detect touch device
    this.UI.IS_TOUCH_DEVICE = 'ontouchstart' in window || navigator.maxTouchPoints > 0;
    
    // Detect screen size
    this.UI.IS_MOBILE = window.innerWidth < 768;
    this.UI.IS_TABLET = window.innerWidth >= 768 && window.innerWidth < 1024;
    this.UI.IS_DESKTOP = window.innerWidth >= 1024;
};

// Initialize feature detection
CONFIG.detectFeatures();

// Update configuration on window resize
window.addEventListener('resize', () => {
    CONFIG.UI.IS_MOBILE = window.innerWidth < 768;
    CONFIG.UI.IS_TABLET = window.innerWidth >= 768 && window.innerWidth < 1024;
    CONFIG.UI.IS_DESKTOP = window.innerWidth >= 1024;
});

// Export configuration
if (typeof module !== 'undefined' && module.exports) {
    module.exports = CONFIG;
}

// Global access
window.CONFIG = CONFIG;