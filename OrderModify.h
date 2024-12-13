#pragma once // Ensures this file is included only once during compilation to avoid duplicate definitions.

#include "Order.h" // Includes the Order class definition for use in creating or interacting with orders.

// The `OrderModify` class represents a modification request for an existing order.
class OrderModify
{
public:
    // Constructor to initialize the modification details.
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
        : orderId_{ orderId } // Initializes the ID of the order to be modified.
        , price_{ price }     // Specifies the updated price for the order.
        , side_{ side }       // Specifies the side (Buy or Sell) of the modified order.
        , quantity_{ quantity } // Specifies the updated quantity for the order.
    { }

    // Getter for the order ID of the modification.
    OrderId GetOrderId() const { return orderId_; }

    // Getter for the updated price of the order.
    Price GetPrice() const { return price_; }

    // Getter for the side (Buy or Sell) of the modified order.
    Side GetSide() const { return side_; }

    // Getter for the updated quantity of the order.
    Quantity GetQuantity() const { return quantity_; }

    // Converts the modification details into a new `Order` object of the specified type.
    OrderPointer ToOrderPointer(OrderType type) const
    {
        // Creates and returns a shared pointer to a new `Order` object using the modification details.
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderId orderId_;   // The unique identifier for the order being modified.
    Price price_;       // The updated price of the order.
    Side side_;         // The side (Buy or Sell) of the order.
    Quantity quantity_; // The updated quantity of the order.
};
