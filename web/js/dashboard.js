// Main Dashboard Manager
class DashboardManager {
    constructor() {
        this.metrics = {
            totalProducts: 0,
            totalInventory: 0,
            totalValue: 0,
            lowStockCount: 0
        };
        
        this.charts = {};
        this.refreshInterval = null;
        this.lastUpdated = null;
        
        // Activity feed management
        this.activityFeed = [];
        this.maxActivityItems = 50;
        this.activityPaused = false;
        
        // Initialize dashboard
        this.init();
    }
    
    async init() {
        try {
            // Load initial data
            await this.loadDashboardData();
            
            // Initialize charts
            this.initCharts();
            
            // Set up event listeners
            this.setupEventListeners();
            
            // Start auto-refresh if enabled
            if (CONFIG.FEATURES.AUTO_REFRESH) {
                this.startAutoRefresh();
            }
            
            console.log('Dashboard initialized successfully');
        } catch (error) {
            console.error('Failed to initialize dashboard:', error);
            this.showError('Failed to load dashboard data');
        }
    }
    
    async loadDashboardData() {
        try {
            // Show loading state
            this.showLoadingState();
            
            // Fetch dashboard statistics
            const response = await api.getDashboardStats();
            
            if (response.success) {
                this.updateMetrics(response.data);
                this.updateLastUpdated();
            }
            
            // Hide loading state
            this.hideLoadingState();
            
        } catch (error) {
            console.error('Error loading dashboard data:', error);
            this.hideLoadingState();
            throw error;
        }
    }
    
    updateMetrics(data) {
        // Update metric values
        this.metrics = {
            totalProducts: data.totalProducts || 0,
            totalInventory: data.totalInventory || 0,
            totalValue: data.totalValue || 0,
            lowStockCount: data.lowStockCount || 0
        };
        
        // Update UI
        this.updateMetricCards();
        
        // Update charts with new data
        if (data.chartData) {
            this.updateChartData(data.chartData);
        }
    }
    
    updateMetricCards() {
        // Total Products
        const totalProductsEl = document.getElementById('totalProducts');
        if (totalProductsEl) {
            this.animateValue(totalProductsEl, this.metrics.totalProducts);
        }
        
        // Total Inventory
        const totalInventoryEl = document.getElementById('totalInventory');
        if (totalInventoryEl) {
            this.animateValue(totalInventoryEl, this.metrics.totalInventory);
        }
        
        // Total Value
        const totalValueEl = document.getElementById('totalValue');
        if (totalValueEl) {
            const formattedValue = this.formatCurrency(this.metrics.totalValue);
            totalValueEl.textContent = formattedValue;
        }
        
        // Low Stock Count
        const lowStockEl = document.getElementById('lowStockCount');
        if (lowStockEl) {
            this.animateValue(lowStockEl, this.metrics.lowStockCount);
        }
        
        // Update metric changes (if previous data exists)
        this.updateMetricChanges();
    }
    
    updateMetricChanges() {
        // This would compare with previous values and show percentage changes
        // For now, we'll show placeholder values
        const changes = {
            products: '+5.2%',
            inventory: '-2.1%',
            value: '+8.7%',
            lowStock: '+12.3%'
        };
        
        this.updateMetricChange('productsChange', changes.products, changes.products.startsWith('+'));
        this.updateMetricChange('inventoryChange', changes.inventory, changes.inventory.startsWith('+'));
        this.updateMetricChange('valueChange', changes.value, changes.value.startsWith('+'));
        this.updateMetricChange('lowStockChange', changes.lowStock, changes.lowStock.startsWith('+'));
    }
    
    updateMetricChange(elementId, value, isPositive) {
        const element = document.getElementById(elementId);
        if (element) {
            element.textContent = value;
            element.className = `metric-change ${isPositive ? 'positive' : 'negative'}`;
        }
    }
    
    animateValue(element, targetValue, duration = 1000) {
        const startValue = parseInt(element.textContent) || 0;
        const startTime = Date.now();
        
        const animate = () => {
            const elapsed = Date.now() - startTime;
            const progress = Math.min(elapsed / duration, 1);
            
            const currentValue = Math.floor(startValue + (targetValue - startValue) * progress);
            element.textContent = currentValue.toLocaleString();
            
            if (progress < 1) {
                requestAnimationFrame(animate);
            }
        };
        
        animate();
    }
    
    initCharts() {
        // Initialize inventory distribution chart
        this.initInventoryChart();
        
        // Initialize other charts as needed
        console.log('Charts initialized');
    }
    
    initInventoryChart() {
        const canvas = document.getElementById('inventoryChart');
        if (!canvas) return;
        
        const ctx = canvas.getContext('2d');
        
        // Sample data - will be replaced with real data
        const data = {
            labels: ['Electronics', 'Clothing', 'Food', 'Books', 'Sports'],
            datasets: [{
                data: [30, 25, 20, 15, 10],
                backgroundColor: [
                    CONFIG.CHARTS.COLORS.PRIMARY,
                    CONFIG.CHARTS.COLORS.SUCCESS,
                    CONFIG.CHARTS.COLORS.WARNING,
                    CONFIG.CHARTS.COLORS.INFO,
                    CONFIG.CHARTS.COLORS.ACCENT
                ],
                borderWidth: 2,
                borderColor: '#ffffff'
            }]
        };
        
        this.charts.inventory = new Chart(ctx, {
            type: 'doughnut',
            data: data,
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    },
                    tooltip: {
                        backgroundColor: 'rgba(0, 0, 0, 0.8)',
                        titleColor: '#ffffff',
                        bodyColor: '#ffffff',
                        borderColor: CONFIG.CHARTS.COLORS.PRIMARY,
                        borderWidth: 1
                    }
                },
                animation: {
                    duration: CONFIG.CHARTS.ANIMATION_DURATION
                }
            }
        });
    }
    
    updateChartData(chartData) {
        if (this.charts.inventory && chartData.inventory) {
            this.charts.inventory.data.labels = chartData.inventory.labels;
            this.charts.inventory.data.datasets[0].data = chartData.inventory.data;
            this.charts.inventory.update();
        }
    }
    
    setupEventListeners() {
        // Refresh button
        const refreshBtn = document.getElementById('refreshBtn');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', () => this.refresh());
        }
        
        // Chart type selector
        const chartSelector = document.getElementById('inventoryChartType');
        if (chartSelector) {
            chartSelector.addEventListener('change', (e) => {
                this.updateChartType(e.target.value);
            });
        }
        
        // Activity feed controls
        const pauseBtn = document.getElementById('pauseUpdates');
        if (pauseBtn) {
            pauseBtn.addEventListener('click', () => this.toggleActivityPause());
        }
        
        const clearBtn = document.getElementById('clearActivity');
        if (clearBtn) {
            clearBtn.addEventListener('click', () => this.clearActivityFeed());
        }
        
        // Quick action buttons
        this.setupQuickActions();
    }
    
    setupQuickActions() {
        // Add Product button
        const addProductBtn = document.getElementById('addProductBtn');
        if (addProductBtn) {
            addProductBtn.addEventListener('click', () => {
                window.ModalManager.openModal('addProductModal');
            });
        }
        
        // Process Order button
        const processOrderBtn = document.getElementById('processOrderBtn');
        if (processOrderBtn) {
            processOrderBtn.addEventListener('click', () => {
                window.ModalManager.openModal('createOrderModal');
            });
        }
        
        // Generate Report button
        const generateReportBtn = document.getElementById('generateReportBtn');
        if (generateReportBtn) {
            generateReportBtn.addEventListener('click', () => {
                if (window.NavigationManager) {
                    window.NavigationManager.showSection('reports');
                }
            });
        }
        
        // View Alerts button
        const viewAlertsBtn = document.getElementById('viewAlertsBtn');
        if (viewAlertsBtn) {
            viewAlertsBtn.addEventListener('click', () => {
                if (window.NavigationManager) {
                    window.NavigationManager.showSection('alerts');
                }
            });
        }
    }
    
    async refresh() {
        try {
            // Show refresh animation
            const refreshBtn = document.getElementById('refreshBtn');
            if (refreshBtn) {
                refreshBtn.classList.add('loading');
            }
            
            await this.loadDashboardData();
            
            // Show success notification
            if (window.NotificationManager) {
                window.NotificationManager.show('Dashboard refreshed successfully', 'success');
            }
            
        } catch (error) {
            console.error('Error refreshing dashboard:', error);
            
            // Show error notification
            if (window.NotificationManager) {
                window.NotificationManager.show('Failed to refresh dashboard', 'error');
            }
        } finally {
            // Remove refresh animation
            const refreshBtn = document.getElementById('refreshBtn');
            if (refreshBtn) {
                refreshBtn.classList.remove('loading');
            }
        }
    }
    
    updateChartType(type) {
        // This would update the chart based on the selected type
        console.log('Updating chart type to:', type);
        
        // For now, just log the change
        this.addActivityItem({
            icon: '📊',
            text: `Chart view changed to ${type}`,
            time: new Date()
        });
    }
    
    startAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
        }
        
        this.refreshInterval = setInterval(() => {
            this.loadDashboardData();
        }, CONFIG.UI.REFRESH_INTERVAL);
    }
    
    stopAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
            this.refreshInterval = null;
        }
    }
    
    updateLastUpdated() {
        this.lastUpdated = new Date();
        const element = document.getElementById('lastUpdated');
        if (element) {
            element.textContent = `Last updated: ${this.formatTime(this.lastUpdated)}`;
        }
    }
    
    // Activity feed management
    addActivityItem(activity) {
        if (this.activityPaused) return;
        
        const activityItem = {
            id: Date.now(),
            icon: activity.icon || '📡',
            text: activity.text,
            time: activity.time || new Date()
        };
        
        this.activityFeed.unshift(activityItem);
        
        // Limit activity items
        if (this.activityFeed.length > this.maxActivityItems) {
            this.activityFeed = this.activityFeed.slice(0, this.maxActivityItems);
        }
        
        this.renderActivityFeed();
    }
    
    renderActivityFeed() {
        const feedContainer = document.getElementById('activityFeed');
        if (!feedContainer) return;
        
        if (this.activityFeed.length === 0) {
            feedContainer.innerHTML = `
                <div class="activity-item placeholder">
                    <div class="activity-icon">📡</div>
                    <div class="activity-content">
                        <div class="activity-text">No recent activity</div>
                        <div class="activity-time">--</div>
                    </div>
                </div>
            `;
            return;
        }
        
        feedContainer.innerHTML = this.activityFeed.map(item => `
            <div class="activity-item">
                <div class="activity-icon">${item.icon}</div>
                <div class="activity-content">
                    <div class="activity-text">${item.text}</div>
                    <div class="activity-time">${this.formatTimeAgo(item.time)}</div>
                </div>
            </div>
        `).join('');
    }
    
    toggleActivityPause() {
        this.activityPaused = !this.activityPaused;
        const pauseBtn = document.getElementById('pauseUpdates');
        if (pauseBtn) {
            pauseBtn.textContent = this.activityPaused ? '▶️ Resume' : '⏸️ Pause';
        }
    }
    
    clearActivityFeed() {
        this.activityFeed = [];
        this.renderActivityFeed();
    }
    
    // Real-time event handlers
    handleProductUpdate(data) {
        this.addActivityItem({
            icon: '📦',
            text: `Product ${data.name} updated`,
            time: new Date()
        });
        
        // Refresh metrics if needed
        if (data.significantChange) {
            this.loadDashboardData();
        }
    }
    
    handleInventoryChange(data) {
        this.addActivityItem({
            icon: '📊',
            text: `Inventory updated: ${data.productName} (${data.change > 0 ? '+' : ''}${data.change})`,
            time: new Date()
        });
    }
    
    handleOrderCreated(data) {
        this.addActivityItem({
            icon: '📋',
            text: `New order created: ${data.orderId}`,
            time: new Date()
        });
    }
    
    handleAlertTriggered(data) {
        this.addActivityItem({
            icon: '🚨',
            text: `Alert: ${data.message}`,
            time: new Date()
        });
    }
    
    // Utility methods
    formatCurrency(amount) {
        return new Intl.NumberFormat('en-US', {
            style: 'currency',
            currency: 'USD'
        }).format(amount);
    }
    
    formatTime(date) {
        return new Intl.DateTimeFormat('en-US', {
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        }).format(date);
    }
    
    formatTimeAgo(date) {
        const now = new Date();
        const diff = now - date;
        const minutes = Math.floor(diff / 60000);
        
        if (minutes < 1) return 'Just now';
        if (minutes < 60) return `${minutes}m ago`;
        
        const hours = Math.floor(minutes / 60);
        if (hours < 24) return `${hours}h ago`;
        
        const days = Math.floor(hours / 24);
        return `${days}d ago`;
    }
    
    showLoadingState() {
        // Add loading class to metrics
        const metricsGrid = document.querySelector('.metrics-grid');
        if (metricsGrid) {
            metricsGrid.classList.add('loading');
        }
    }
    
    hideLoadingState() {
        // Remove loading class from metrics
        const metricsGrid = document.querySelector('.metrics-grid');
        if (metricsGrid) {
            metricsGrid.classList.remove('loading');
        }
    }
    
    showError(message) {
        if (window.NotificationManager) {
            window.NotificationManager.show(message, 'error');
        }
    }
    
    // Cleanup
    destroy() {
        this.stopAutoRefresh();
        
        // Destroy charts
        Object.values(this.charts).forEach(chart => {
            if (chart && typeof chart.destroy === 'function') {
                chart.destroy();
            }
        });
        
        this.charts = {};
    }
}

// Create global dashboard manager instance
const dashboardManager = new DashboardManager();

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { DashboardManager, dashboardManager };
}

// Global access
window.DashboardManager = dashboardManager;