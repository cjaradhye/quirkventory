// Main Application Controller
class QuirkventoryApp {
    constructor() {
        this.currentSection = 'dashboard';
        this.managers = {};
        this.isInitialized = false;
        
        // Initialize application
        this.init();
    }
    
    async init() {
        try {
            console.log('Initializing Quirkventory Dashboard...');
            
            // Initialize core managers
            await this.initializeManagers();
            
            // Set up global event listeners
            this.setupGlobalEventListeners();
            
            // Initialize navigation
            this.initializeNavigation();
            
            // Initialize keyboard shortcuts
            this.initializeKeyboardShortcuts();
            
            // Show initial section
            this.showSection(this.currentSection);
            
            // Mark as initialized
            this.isInitialized = true;
            
            console.log('Quirkventory Dashboard initialized successfully');
            
        } catch (error) {
            console.error('Failed to initialize application:', error);
            this.showCriticalError('Failed to initialize application');
        }
    }
    
    async initializeManagers() {
        // Initialize Navigation Manager
        this.managers.navigation = new NavigationManager();
        
        // Initialize Modal Manager
        this.managers.modal = new ModalManager();
        
        // Initialize Notification Manager
        this.managers.notification = new NotificationManager();
        
        // Initialize Inventory Manager
        this.managers.inventory = new InventoryManager();
        
        // Initialize Order Manager
        this.managers.order = new OrderManager();
        
        // Initialize Report Manager
        this.managers.report = new ReportManager();
        
        // Initialize Alert Manager
        this.managers.alert = new AlertManager();
        
        // Initialize Chart Manager
        this.managers.chart = new ChartManager();
        
        // Make managers globally accessible
        Object.entries(this.managers).forEach(([key, manager]) => {
            const globalName = key.charAt(0).toUpperCase() + key.slice(1) + 'Manager';
            window[globalName] = manager;
        });
        
        console.log('All managers initialized');
    }
    
    setupGlobalEventListeners() {
        // Handle window resize
        window.addEventListener('resize', this.debounce(() => {
            this.handleResize();
        }, CONFIG.PERFORMANCE.DEBOUNCE_INTERVALS.RESIZE));
        
        // Handle visibility change
        document.addEventListener('visibilitychange', () => {
            this.handleVisibilityChange();
        });
        
        // Handle online/offline status
        window.addEventListener('online', () => {
            this.handleOnlineStatus(true);
        });
        
        window.addEventListener('offline', () => {
            this.handleOnlineStatus(false);
        });
        
        // Handle unload
        window.addEventListener('beforeunload', (e) => {
            this.handleBeforeUnload(e);
        });
        
        // Global error handler
        window.addEventListener('error', (e) => {
            this.handleGlobalError(e);
        });
        
        // Unhandled promise rejection handler
        window.addEventListener('unhandledrejection', (e) => {
            this.handleUnhandledRejection(e);
        });
    }
    
    initializeNavigation() {
        // Set up navigation event listeners
        const navLinks = document.querySelectorAll('.nav-link');
        navLinks.forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();
                const section = link.dataset.section;
                if (section) {
                    this.showSection(section);
                }
            });
        });
    }
    
    initializeKeyboardShortcuts() {
        document.addEventListener('keydown', (e) => {
            // Check if user is typing in an input field
            if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') {
                return;
            }
            
            const key = e.key;
            const ctrl = e.ctrlKey || e.metaKey;
            
            // Handle shortcuts
            if (ctrl && key === 'k') {
                e.preventDefault();
                this.focusSearch();
            } else if (ctrl && key === 'n') {
                e.preventDefault();
                this.managers.modal.openModal('addProductModal');
            } else if (key === 'F5') {
                e.preventDefault();
                this.refreshCurrentSection();
            } else if (ctrl && key === 'e') {
                e.preventDefault();
                this.showExportOptions();
            } else if (key === 'F1') {
                e.preventDefault();
                this.showHelp();
            } else if (key === 'Escape') {
                this.handleEscape();
            }
        });
    }
    
    showSection(sectionName) {
        // Hide all sections
        const sections = document.querySelectorAll('.content-section');
        sections.forEach(section => {
            section.classList.remove('active');
        });
        
        // Show target section
        const targetSection = document.getElementById(`${sectionName}-section`);
        if (targetSection) {
            targetSection.classList.add('active');
            this.currentSection = sectionName;
            
            // Update navigation
            this.updateNavigation(sectionName);
            
            // Initialize section if needed
            this.initializeSection(sectionName);
            
            // Update page title
            this.updatePageTitle(sectionName);
        }
    }
    
    updateNavigation(activeSection) {
        const navLinks = document.querySelectorAll('.nav-link');
        navLinks.forEach(link => {
            link.classList.remove('active');
            if (link.dataset.section === activeSection) {
                link.classList.add('active');
            }
        });
    }
    
    initializeSection(sectionName) {
        switch (sectionName) {
            case 'dashboard':
                if (!this.managers.dashboard?.isInitialized) {
                    // Dashboard is already initialized in dashboard.js
                }
                break;
                
            case 'inventory':
                this.managers.inventory.loadInventoryData();
                break;
                
            case 'orders':
                this.managers.order.loadOrderData();
                break;
                
            case 'reports':
                this.managers.report.loadReports();
                break;
                
            case 'alerts':
                this.managers.alert.loadAlerts();
                break;
        }
    }
    
    updatePageTitle(sectionName) {
        const titles = {
            dashboard: 'Dashboard',
            inventory: 'Inventory Management',
            orders: 'Order Management',
            reports: 'Reports & Analytics',
            alerts: 'System Alerts'
        };
        
        const title = titles[sectionName] || 'Quirkventory';
        document.title = `${title} - Quirkventory`;
    }
    
    // Event handlers
    handleResize() {
        // Notify all managers about resize
        Object.values(this.managers).forEach(manager => {
            if (manager.handleResize) {
                manager.handleResize();
            }
        });
        
        // Update responsive charts
        if (this.managers.chart) {
            this.managers.chart.handleResize();
        }
    }
    
    handleVisibilityChange() {
        if (document.hidden) {
            // Page is hidden - pause real-time updates
            if (wsClient) {
                console.log('Page hidden - reducing update frequency');
            }
        } else {
            // Page is visible - resume normal updates
            if (wsClient) {
                console.log('Page visible - resuming normal updates');
                this.refreshCurrentSection();
            }
        }
    }
    
    handleOnlineStatus(isOnline) {
        if (isOnline) {
            this.managers.notification.show('Connection restored', 'success');
            
            // Reconnect WebSocket if needed
            if (wsClient && !wsClient.isConnected) {
                wsClient.connect();
            }
            
            // Refresh current section
            this.refreshCurrentSection();
        } else {
            this.managers.notification.show('Connection lost - working offline', 'warning');
        }
    }
    
    handleBeforeUnload(e) {
        // Check for unsaved changes
        const hasUnsavedChanges = this.checkUnsavedChanges();
        
        if (hasUnsavedChanges) {
            e.preventDefault();
            e.returnValue = '';
            return '';
        }
    }
    
    handleGlobalError(e) {
        console.error('Global error:', e.error);
        
        // Log error for debugging
        if (CONFIG.DEBUG.ENABLED) {
            console.error('Error details:', {
                message: e.message,
                filename: e.filename,
                lineno: e.lineno,
                colno: e.colno,
                error: e.error
            });
        }
        
        // Show user-friendly error message
        this.managers.notification.show(
            'An unexpected error occurred. Please refresh the page.',
            'error'
        );
    }
    
    handleUnhandledRejection(e) {
        console.error('Unhandled promise rejection:', e.reason);
        
        // Prevent the default browser behavior
        e.preventDefault();
        
        // Show user-friendly error message
        this.managers.notification.show(
            'A network error occurred. Please check your connection.',
            'error'
        );
    }
    
    // Utility methods
    focusSearch() {
        const searchInput = document.getElementById('searchProducts') || 
                           document.querySelector('.search-input');
        if (searchInput) {
            searchInput.focus();
            searchInput.select();
        }
    }
    
    refreshCurrentSection() {
        switch (this.currentSection) {
            case 'dashboard':
                if (window.DashboardManager) {
                    window.DashboardManager.refresh();
                }
                break;
                
            case 'inventory':
                this.managers.inventory.refresh();
                break;
                
            case 'orders':
                this.managers.order.refresh();
                break;
                
            case 'reports':
                this.managers.report.refresh();
                break;
                
            case 'alerts':
                this.managers.alert.refresh();
                break;
        }
    }
    
    showExportOptions() {
        // Show export modal or options based on current section
        console.log('Export options for:', this.currentSection);
        
        // This would open an export dialog
        this.managers.notification.show('Export feature coming soon', 'info');
    }
    
    showHelp() {
        // Show help modal or navigate to help section
        console.log('Showing help');
        
        this.managers.notification.show('Help documentation coming soon', 'info');
    }
    
    handleEscape() {
        // Close any open modals
        this.managers.modal.closeAllModals();
        
        // Clear any active selections
        this.clearSelections();
    }
    
    clearSelections() {
        // Clear any selected items in tables
        const selectedItems = document.querySelectorAll('.selected');
        selectedItems.forEach(item => {
            item.classList.remove('selected');
        });
    }
    
    checkUnsavedChanges() {
        // Check all managers for unsaved changes
        return Object.values(this.managers).some(manager => {
            return manager.hasUnsavedChanges && manager.hasUnsavedChanges();
        });
    }
    
    showCriticalError(message) {
        // Show critical error overlay
        const overlay = document.createElement('div');
        overlay.className = 'critical-error-overlay';
        overlay.innerHTML = `
            <div class="critical-error-content">
                <div class="error-icon">⚠️</div>
                <h2>Application Error</h2>
                <p>${message}</p>
                <button onclick="location.reload()" class="btn primary">
                    Reload Application
                </button>
            </div>
        `;
        
        document.body.appendChild(overlay);
    }
    
    // Utility function for debouncing
    debounce(func, wait) {
        let timeout;
        return function executedFunction(...args) {
            const later = () => {
                clearTimeout(timeout);
                func(...args);
            };
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
        };
    }
    
    // Public API
    getManager(name) {
        return this.managers[name];
    }
    
    getCurrentSection() {
        return this.currentSection;
    }
    
    isReady() {
        return this.isInitialized;
    }
}

// Initialize application when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.app = new QuirkventoryApp();
});

// Export for testing and external use
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { QuirkventoryApp };
}