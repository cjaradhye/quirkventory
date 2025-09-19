# UML Class Diagram - Inventory Management System

## Class Hierarchy Overview

```
┌─────────────────┐    ┌─────────────────┐
│     Product     │    │      User       │
│  (Abstract)     │    │   (Abstract)    │
├─────────────────┤    ├─────────────────┤
│ # id: string    │    │ # userId: string│
│ # name: string  │    │ # username: str │
│ # price: double │    │ # password: str │
│ # category: str │    │ # email: string │
│ # quantity: int │    │ # createdDate   │
├─────────────────┤    ├─────────────────┤
│ + Product()     │    │ + User()        │
│ + ~Product()    │    │ + ~User()       │
│ + getId()       │    │ + authenticate()│
│ + setPrice()    │    │ + getRole()     │
│ + virtual       │    │ + virtual       │
│   getInfo()     │    │   getPermissions│
│ + virtual       │    │ + virtual       │
│   isExpired()   │    │   canModify()   │
└─────────────────┘    └─────────────────┘
         │                       │
         │                       │
┌─────────────────┐              │
│ PerishableProduct│              │
├─────────────────┤    ┌─────────┴─────────┐
│ - expiryDate    │    │                   │
│ - storageTemp   │    │                   │
├─────────────────┤    ▼                   ▼
│ + setExpiry()   │ ┌──────────┐    ┌──────────┐
│ + override      │ │   Staff  │    │ Manager  │
│   isExpired()   │ ├──────────┤    ├──────────┤
│ + override      │ │+ role    │    │+ dept    │
│   getInfo()     │ │+ shift   │    │+ budget  │
└─────────────────┘ ├──────────┤    ├──────────┤
                    │+ canAdd()│    │+ override│
                    │+ canView│    │  canModify│
                    └──────────┘    │+ generate│
                                   │  Reports()│
                                   └──────────┘

┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│    Inventory    │    │      Order      │    │  Notification   │
│                 │    │                 │    │   (Abstract)    │
├─────────────────┤    ├─────────────────┤    ├─────────────────┤
│ - products: map │    │ - orderId: str  │    │ # message: str  │
│ - lowStockLimit │    │ - items: vector │    │ # timestamp     │
│ - mutex: mutex  │    │ - totalAmount   │    │ # priority: int │
├─────────────────┤    │ - status: enum  │    ├─────────────────┤
│ + addProduct()  │    │ - customerId    │    │ + virtual send()│
│ + removeProduct│    │ - orderDate     │    │ + virtual       │
│ + updateStock() │    ├─────────────────┤    │   format()      │
│ + checkLowStock │    │ + addItem()     │    └─────────────────┘
│ + generateReport│    │ + processOrder()│             │
│ + searchProduct │    │ + validateOrder │             │
└─────────────────┘    │ + updateStatus()│    ┌────────┴────────┐
         │              └─────────────────┘    │                 │
         │                       │             ▼                 ▼
         │                       │     ┌──────────────┐  ┌──────────────┐
         │                       │     │ EmailAlert   │  │ SystemAlert  │
         │                       │     ├──────────────┤  ├──────────────┤
         │                       │     │ + override   │  │ + override   │
         │                       │     │   send()     │  │   send()     │
         │                       │     └──────────────┘  └──────────────┘
         │                       │
         └───────────────────────┘
                 Uses
```

## Relationships

1. **Inheritance:**
   - `PerishableProduct` extends `Product`
   - `Staff` and `Manager` extend `User`
   - `EmailAlert` and `SystemAlert` extend `Notification`

2. **Composition:**
   - `Inventory` contains multiple `Product` objects
   - `Order` contains multiple `OrderItem` objects

3. **Association:**
   - `Inventory` uses `Notification` for alerts
   - `Order` processes using `Inventory`

## Key Design Patterns

- **Polymorphism:** Virtual methods in base classes
- **Encapsulation:** Private/protected members with public interfaces
- **Template Pattern:** Order processing workflow
- **Observer Pattern:** Notification system for stock alerts