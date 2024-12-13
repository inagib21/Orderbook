#pragma once // Ensures this file is included only once in a single compilation to prevent duplicate declarations.

#include <list>        // Provides the `std::list` container, used for storing a list of orders.
#include <exception>   // Includes exception handling classes like `std::logic_error`.
#include <format>      // Allows for formatted string generation, used for error messages.

#include "OrderType.h" // Custom header defining the types of orders, like Market or GoodTillCancel.
#include "Side.h"      // Custom header defining the side of the order (Buy or Sell).
#include "Usings.h"    // Header for shorthand type definitions (e.g., `Price`, `Quantity`, `OrderId`).
#include "Constants.h" // Header for constants used in the application (e.g., an invalid price).

// The `Order` class represents an individual order in the order book.
class Order
{
public:
    // Constructor for orders with all details, such as type, ID, side, price, and quantity.
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_{ orderType }  // Initializes the type of the order (e.g., Market or GoodTillCancel).
        , orderId_{ orderId }      // Initializes the unique identifier for this order.
        , side_{ side }            // Specifies whether this is a Buy or Sell order.
        , price_{ price }          // The price at which the order is placed.
        , initialQuantity_{ quantity }  // The initial quantity of this order.
        , remainingQuantity_{ quantity } // Initially, the remaining quantity is the same as the total quantity.
    { }

    // Constructor for market orders (type defaults to Market and price is set to invalid).
    Order(OrderId orderId, Side side, Quantity quantity)
        : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
    { }

    // Getter for the unique ID of the order.
    OrderId GetOrderId() const { return orderId_; }

    // Getter for the side of the order (Buy or Sell).
    Side GetSide() const { return side_; }

    // Getter for the price of the order.
    Price GetPrice() const { return price_; }

    // Getter for the type of the order (e.g., Market or GoodTillCancel).
    OrderType GetOrderType() const { return orderType_; }

    // Getter for the initial quantity of the order when it was created.
    Quantity GetInitialQuantity() const { return initialQuantity_; }

    // Getter for the remaining quantity of the order that has not been fulfilled yet.
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }

    // Calculates the filled quantity by subtracting remaining quantity from initial quantity.
    Quantity GetFilledQuantity() const { return GetInitialQuantity() - GetRemainingQuantity(); }

    // Checks if the order has been completely fulfilled.
    bool IsFilled() const { return GetRemainingQuantity() == 0; }

    // Reduces the remaining quantity of the order by the given amount to simulate filling the order.
    void Fill(Quantity quantity)
    {
        // Throws an exception if trying to fill more than the remaining quantity.
        if (quantity > GetRemainingQuantity())
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));

        remainingQuantity_ -= quantity; // Updates the remaining quantity.
    }

    // Converts a Market order to a GoodTillCancel order by assigning it a price.
    void ToGoodTillCancel(Price price) 
    { 
        // Throws an exception if the order type is not Market.
        if (GetOrderType() != OrderType::Market)
            throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", GetOrderId()));

        price_ = price;                 // Sets the new price.
        orderType_ = OrderType::GoodTillCancel; // Changes the type to GoodTillCancel.
    }

private:
    OrderType orderType_;         // Stores the type of the order (Market, GoodTillCancel, etc.).
    OrderId orderId_;             // Unique identifier for the order.
    Side side_;                   // Indicates whether the order is Buy or Sell.
    Price price_;                 // The price of the order.
    Quantity initialQuantity_;    // The original quantity of the order when it was created.
    Quantity remainingQuantity_;  // The quantity that is yet to be fulfilled.
};

// Alias for a shared pointer to an `Order` object.
using OrderPointer = std::shared_ptr<Order>;

// Alias for a list of shared pointers to `Order` objects.
using OrderPointers = std::list<OrderPointer>;
