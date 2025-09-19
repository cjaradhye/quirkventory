// Sample Data for Demo and Testing
const SAMPLE_DATA = {
    // Sample Products
    products: [
        {
            id: "ELEC001",
            name: "MacBook Pro 16\"",
            category: "Electronics",
            price: 2499.99,
            quantity: 15,
            status: "in-stock",
            lastUpdated: "2025-09-20T10:30:00Z",
            supplier: "Apple Inc.",
            sku: "MBP16-512-SG",
            description: "16-inch MacBook Pro with M3 Pro chip, 512GB SSD"
        },
        {
            id: "ELEC002", 
            name: "iPhone 15 Pro",
            category: "Electronics",
            price: 999.99,
            quantity: 8,
            status: "low-stock",
            lastUpdated: "2025-09-20T09:15:00Z",
            supplier: "Apple Inc.",
            sku: "IP15P-128-TB",
            description: "iPhone 15 Pro 128GB in Titanium Blue"
        },
        {
            id: "FURN001",
            name: "Ergonomic Office Chair",
            category: "Furniture",
            price: 299.99,
            quantity: 25,
            status: "in-stock",
            lastUpdated: "2025-09-19T16:45:00Z",
            supplier: "Herman Miller",
            sku: "HM-AERON-B",
            description: "Aeron Chair Size B with PostureFit SL"
        },
        {
            id: "FURN002",
            name: "Standing Desk 60\"",
            category: "Furniture", 
            price: 499.99,
            quantity: 12,
            status: "in-stock",
            lastUpdated: "2025-09-19T14:20:00Z",
            supplier: "IKEA",
            sku: "BEKANT-60-WHT",
            description: "BEKANT desk with OLOV adjustable legs"
        },
        {
            id: "BOOK001",
            name: "Clean Code",
            category: "Books",
            price: 39.99,
            quantity: 50,
            status: "in-stock", 
            lastUpdated: "2025-09-18T12:00:00Z",
            supplier: "Pearson",
            sku: "CC-MARTIN-2E",
            description: "A Handbook of Agile Software Craftsmanship"
        },
        {
            id: "BOOK002",
            name: "JavaScript: The Good Parts",
            category: "Books",
            price: 29.99,
            quantity: 3,
            status: "low-stock",
            lastUpdated: "2025-09-18T11:30:00Z",
            supplier: "O'Reilly Media",
            sku: "JS-GOOD-CROCKFORD",
            description: "Classic JavaScript programming guide"
        },
        {
            id: "CLTH001",
            name: "Premium Cotton T-Shirt",
            category: "Clothing",
            price: 24.99,
            quantity: 100,
            status: "in-stock",
            lastUpdated: "2025-09-17T10:15:00Z",
            supplier: "Uniqlo",
            sku: "UNIQ-TEE-WHT-L",
            description: "100% organic cotton crew neck t-shirt"
        },
        {
            id: "CLTH002",
            name: "Wireless Headphones",
            category: "Electronics",
            price: 249.99,
            quantity: 0,
            status: "out-of-stock",
            lastUpdated: "2025-09-16T08:45:00Z",
            supplier: "Sony",
            sku: "WH-1000XM5-B",
            description: "Industry-leading noise canceling headphones"
        },
        {
            id: "HOME001",
            name: "Smart Coffee Maker",
            category: "Home & Garden",
            price: 149.99,
            quantity: 18,
            status: "in-stock",
            lastUpdated: "2025-09-20T07:30:00Z",
            supplier: "Keurig",
            sku: "K-ELITE-BLK",
            description: "K-Elite Single Serve Coffee Maker"
        },
        {
            id: "SPRT001",
            name: "Yoga Mat Premium",
            category: "Sports",
            price: 59.99,
            quantity: 35,
            status: "in-stock",
            lastUpdated: "2025-09-19T13:20:00Z",
            supplier: "Manduka",
            sku: "MANDUKA-PRO-PRP",
            description: "Professional grade yoga mat, 6mm thickness"
        }
    ],

    // Sample Orders
    orders: [
        {
            id: "ORD001234",
            customerId: "CUST001",
            customerName: "Acme Corporation",
            status: "completed",
            orderDate: "2025-09-20T08:30:00Z",
            totalAmount: 5499.96,
            items: [
                { productId: "ELEC001", productName: "MacBook Pro 16\"", quantity: 2, unitPrice: 2499.99 },
                { productId: "FURN001", productName: "Ergonomic Office Chair", quantity: 2, unitPrice: 299.99 }
            ]
        },
        {
            id: "ORD001235",
            customerId: "CUST002", 
            customerName: "Tech Startup Inc.",
            status: "processing",
            orderDate: "2025-09-20T09:15:00Z",
            totalAmount: 1799.95,
            items: [
                { productId: "ELEC002", productName: "iPhone 15 Pro", quantity: 1, unitPrice: 999.99 },
                { productId: "FURN002", productName: "Standing Desk 60\"", quantity: 1, unitPrice: 499.99 },
                { productId: "FURN001", productName: "Ergonomic Office Chair", quantity: 1, unitPrice: 299.99 }
            ]
        },
        {
            id: "ORD001236",
            customerId: "CUST003",
            customerName: "Educational Institute",
            status: "pending",
            orderDate: "2025-09-20T10:00:00Z", 
            totalAmount: 399.90,
            items: [
                { productId: "BOOK001", productName: "Clean Code", quantity: 5, unitPrice: 39.99 },
                { productId: "BOOK002", productName: "JavaScript: The Good Parts", quantity: 5, unitPrice: 29.99 }
            ]
        },
        {
            id: "ORD001237",
            customerId: "CUST004",
            customerName: "Retail Store LLC",
            status: "cancelled",
            orderDate: "2025-09-19T15:45:00Z",
            totalAmount: 2499.90,
            items: [
                { productId: "CLTH001", productName: "Premium Cotton T-Shirt", quantity: 100, unitPrice: 24.99 }
            ]
        },
        {
            id: "ORD001238",
            customerId: "CUST005",
            customerName: "Fitness Center Pro",
            status: "completed",
            orderDate: "2025-09-19T11:20:00Z",
            totalAmount: 1049.75,
            items: [
                { productId: "SPRT001", productName: "Yoga Mat Premium", quantity: 15, unitPrice: 59.99 },
                { productId: "HOME001", productName: "Smart Coffee Maker", quantity: 2, unitPrice: 149.99 }
            ]
        }
    ],

    // Sample Alerts
    alerts: [
        {
            id: "ALT001",
            type: "low-stock",
            severity: "warning",
            message: "iPhone 15 Pro is running low on stock (8 units remaining)",
            productId: "ELEC002",
            timestamp: "2025-09-20T09:15:00Z",
            isRead: false
        },
        {
            id: "ALT002", 
            type: "out-of-stock",
            severity: "critical",
            message: "Wireless Headphones are out of stock",
            productId: "CLTH002",
            timestamp: "2025-09-16T08:45:00Z",
            isRead: false
        },
        {
            id: "ALT003",
            type: "low-stock",
            severity: "warning", 
            message: "JavaScript: The Good Parts is running low on stock (3 units remaining)",
            productId: "BOOK002",
            timestamp: "2025-09-18T11:30:00Z",
            isRead: true
        },
        {
            id: "ALT004",
            type: "order-processing",
            severity: "info",
            message: "Order ORD001235 is ready for fulfillment",
            orderId: "ORD001235",
            timestamp: "2025-09-20T09:20:00Z",
            isRead: false
        }
    ],

    // Dashboard Statistics
    dashboardStats: {
        totalProducts: 10,
        totalInventory: 286,
        totalValue: 125847.35,
        lowStockCount: 2,
        outOfStockCount: 1,
        ordersToday: 3,
        ordersThisWeek: 12,
        ordersThisMonth: 47,
        topSellingProducts: [
            { id: "CLTH001", name: "Premium Cotton T-Shirt", soldUnits: 150 },
            { id: "ELEC001", name: "MacBook Pro 16\"", soldUnits: 8 },
            { id: "FURN001", name: "Ergonomic Office Chair", soldUnits: 12 }
        ],
        revenueThisMonth: 45670.25,
        categories: [
            { name: "Electronics", count: 3, value: 45000.00 },
            { name: "Furniture", count: 2, value: 18750.00 },
            { name: "Books", count: 2, value: 2100.00 },
            { name: "Clothing", count: 1, value: 2500.00 },
            { name: "Home & Garden", count: 1, value: 2700.00 },
            { name: "Sports", count: 1, value: 2100.00 }
        ]
    },

    // Activity Feed Data
    activities: [
        {
            id: "ACT001",
            icon: "📦",
            text: "New product added: Smart Coffee Maker",
            time: new Date(Date.now() - 5 * 60000) // 5 minutes ago
        },
        {
            id: "ACT002", 
            icon: "📋",
            text: "Order ORD001235 created by Tech Startup Inc.",
            time: new Date(Date.now() - 15 * 60000) // 15 minutes ago
        },
        {
            id: "ACT003",
            icon: "📊",
            text: "Inventory updated: iPhone 15 Pro (-2 units)",
            time: new Date(Date.now() - 25 * 60000) // 25 minutes ago
        },
        {
            id: "ACT004",
            icon: "🚨",
            text: "Low stock alert: iPhone 15 Pro (8 units remaining)",
            time: new Date(Date.now() - 30 * 60000) // 30 minutes ago
        },
        {
            id: "ACT005",
            icon: "✅",
            text: "Order ORD001234 completed and shipped",
            time: new Date(Date.now() - 45 * 60000) // 45 minutes ago
        },
        {
            id: "ACT006",
            icon: "👤",
            text: "User admin logged in",
            time: new Date(Date.now() - 60 * 60000) // 1 hour ago
        },
        {
            id: "ACT007",
            icon: "📈",
            text: "Daily sales report generated",
            time: new Date(Date.now() - 90 * 60000) // 1.5 hours ago
        }
    ]
};

// Sample Data Loader
class SampleDataLoader {
    static loadDashboardData() {
        if (window.DashboardManager) {
            // Update metrics
            window.DashboardManager.updateMetrics(SAMPLE_DATA.dashboardStats);
            
            // Load activities
            SAMPLE_DATA.activities.forEach(activity => {
                window.DashboardManager.addActivityItem(activity);
            });
            
            // Update chart data
            const chartData = {
                inventory: {
                    labels: SAMPLE_DATA.dashboardStats.categories.map(cat => cat.name),
                    data: SAMPLE_DATA.dashboardStats.categories.map(cat => cat.count)
                }
            };
            window.DashboardManager.updateChartData(chartData);
        }
    }
    
    static loadInventoryData() {
        const inventoryTableBody = document.getElementById('inventoryTableBody');
        if (inventoryTableBody) {
            inventoryTableBody.innerHTML = SAMPLE_DATA.products.map(product => `
                <tr>
                    <td>${product.id}</td>
                    <td>${product.name}</td>
                    <td>${product.category}</td>
                    <td>$${product.price.toFixed(2)}</td>
                    <td>${product.quantity}</td>
                    <td>
                        <span class="status-badge ${product.status}">
                            ${product.status.replace('-', ' ').toUpperCase()}
                        </span>
                    </td>
                    <td>
                        <button class="btn secondary btn-sm" onclick="editProduct('${product.id}')">Edit</button>
                        <button class="btn warning btn-sm" onclick="deleteProduct('${product.id}')">Delete</button>
                    </td>
                </tr>
            `).join('');
        }
        
        // Update category filter
        const categoryFilter = document.getElementById('categoryFilter');
        if (categoryFilter) {
            const categories = [...new Set(SAMPLE_DATA.products.map(p => p.category))];
            categoryFilter.innerHTML = '<option value="">All Categories</option>' +
                categories.map(cat => `<option value="${cat}">${cat}</option>`).join('');
        }
    }
    
    static loadOrderData() {
        const ordersList = document.getElementById('ordersList');
        if (ordersList) {
            ordersList.innerHTML = SAMPLE_DATA.orders.map(order => `
                <div class="order-card">
                    <div class="order-header">
                        <div class="order-id">${order.id}</div>
                        <div class="order-status status-${order.status}">${order.status.toUpperCase()}</div>
                    </div>
                    <div class="order-details">
                        <div class="order-customer">${order.customerName}</div>
                        <div class="order-date">${new Date(order.orderDate).toLocaleDateString()}</div>
                        <div class="order-total">$${order.totalAmount.toFixed(2)}</div>
                    </div>
                    <div class="order-items">
                        ${order.items.map(item => `
                            <div class="order-item">
                                ${item.quantity}x ${item.productName} @ $${item.unitPrice}
                            </div>
                        `).join('')}
                    </div>
                    <div class="order-actions">
                        <button class="btn secondary btn-sm">View Details</button>
                        ${order.status === 'pending' ? '<button class="btn primary btn-sm">Process</button>' : ''}
                        ${order.status === 'processing' ? '<button class="btn warning btn-sm">Cancel</button>' : ''}
                    </div>
                </div>
            `).join('');
        }
    }
    
    static loadAlertData() {
        const alertsList = document.getElementById('alertsList');
        if (alertsList) {
            alertsList.innerHTML = SAMPLE_DATA.alerts.map(alert => `
                <div class="alert-item ${alert.isRead ? 'read' : 'unread'} severity-${alert.severity}">
                    <div class="alert-icon">
                        ${alert.type === 'low-stock' ? '⚠️' : 
                          alert.type === 'out-of-stock' ? '🚫' : 
                          alert.type === 'order-processing' ? '📋' : '🔔'}
                    </div>
                    <div class="alert-content">
                        <div class="alert-message">${alert.message}</div>
                        <div class="alert-time">${new Date(alert.timestamp).toLocaleString()}</div>
                    </div>
                    <div class="alert-actions">
                        ${!alert.isRead ? '<button class="btn secondary btn-sm">Mark Read</button>' : ''}
                        <button class="btn secondary btn-sm">Dismiss</button>
                    </div>
                </div>
            `).join('');
        }
    }
    
    static loadAllSampleData() {
        // Load all sample data
        this.loadDashboardData();
        this.loadInventoryData();
        this.loadOrderData();
        this.loadAlertData();
        
        console.log('Sample data loaded successfully');
    }
}

// Utility functions for sample data
function editProduct(productId) {
    const product = SAMPLE_DATA.products.find(p => p.id === productId);
    if (product && window.ModalManager) {
        // Populate form with product data
        document.getElementById('productId').value = product.id;
        document.getElementById('productName').value = product.name;
        document.getElementById('productCategory').value = product.category;
        document.getElementById('productPrice').value = product.price;
        document.getElementById('productQuantity').value = product.quantity;
        
        window.ModalManager.openModal('addProductModal');
    }
}

function deleteProduct(productId) {
    if (confirm('Are you sure you want to delete this product?')) {
        // Remove from sample data
        const index = SAMPLE_DATA.products.findIndex(p => p.id === productId);
        if (index > -1) {
            SAMPLE_DATA.products.splice(index, 1);
            SampleDataLoader.loadInventoryData();
            
            if (window.NotificationManager) {
                window.NotificationManager.show('Product deleted successfully', 'success');
            }
        }
    }
}

// Auto-load sample data when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    // Load sample data after a short delay to ensure other components are ready
    setTimeout(() => {
        SampleDataLoader.loadAllSampleData();
    }, 500);
});

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { SAMPLE_DATA, SampleDataLoader };
}

// Global access
window.SAMPLE_DATA = SAMPLE_DATA;
window.SampleDataLoader = SampleDataLoader;