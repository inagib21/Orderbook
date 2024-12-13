#include "Orderbook.h"

#include <numeric>
#include <chrono>
#include <ctime>

// Function to clean up "Good For Day" orders after the market closes
void Orderbook::PruneGoodForDayOrders()
{
    // Use the chrono library to handle time
    using namespace std::chrono;
    const auto end = hours(16); // Set market closing time to 4 PM

    while (true) // Continuous loop to keep checking and pruning orders
    {
        // Get the current system time
        const auto now = system_clock::now();
        const auto now_c = system_clock::to_time_t(now); // Convert time to a raw time format
        std::tm now_parts; // Struct to break time into individual parts
        localtime_s(&now_parts, &now_c); // Populate the struct with the current local time

        // If the current time is after market close, move to the next day
        if (now_parts.tm_hour >= end.count())
            now_parts.tm_mday += 1;

        // Set time to the closing hour (4 PM) for pruning
        now_parts.tm_hour = end.count();
        now_parts.tm_min = 0;
        now_parts.tm_sec = 0;

        // Calculate the next pruning time
        auto next = system_clock::from_time_t(mktime(&now_parts));
        auto till = next - now + milliseconds(100); // Add a small buffer of 100 milliseconds

        {
            // Lock the orders mutex to ensure safe access to shared resources
            std::unique_lock ordersLock{ ordersMutex_ };

            // Exit if shutdown is signaled or wait timeout expires
            if (shutdown_.load(std::memory_order_acquire) ||
                shutdownConditionVariable_.wait_for(ordersLock, till) == std::cv_status::no_timeout)
                return;
        }

        // List to store IDs of orders to be canceled
        OrderIds orderIds;

        {
            // Lock again to safely access and filter orders
            std::scoped_lock ordersLock{ ordersMutex_ };

            // Iterate through all orders and find "Good For Day" orders
            for (const auto& [_, entry] : orders_)
            {
                const auto& [order, _] = entry;

                // Skip orders that are not "Good For Day"
                if (order->GetOrderType() != OrderType::GoodForDay)
                    continue;

                // Add eligible order IDs to the list
                orderIds.push_back(order->GetOrderId());
            }
        }

        // Cancel the identified orders
        CancelOrders(orderIds);
    }
}

// Function to cancel multiple orders
void Orderbook::CancelOrders(OrderIds orderIds)
{
    // Lock orders to safely modify the order list
    std::scoped_lock ordersLock{ ordersMutex_ };

    // Iterate through the list of order IDs and cancel each order
    for (const auto& orderId : orderIds)
        CancelOrderInternal(orderId);
}

// Function to cancel a specific order internally
void Orderbook::CancelOrderInternal(OrderId orderId)
{
    // Check if the order ID exists in the list
    if (!orders_.contains(orderId))
        return;

    // Retrieve and remove the order from the main list
    const auto [order, iterator] = orders_.at(orderId);
    orders_.erase(orderId);

    // Determine if the order was a "sell" or "buy" and update the respective list
    if (order->GetSide() == Side::Sell)
    {
        auto price = order->GetPrice(); // Get the price of the order
        auto& orders = asks_.at(price); // Find the list of sell orders at that price
        orders.erase(iterator); // Remove the specific order
        if (orders.empty()) // If no orders are left, remove the price level
            asks_.erase(price);
    }
    else
    {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price); // Find the list of buy orders at that price
        orders.erase(iterator);
        if (orders.empty())
            bids_.erase(price);
    }

    // Trigger an event to notify the system that the order was canceled
    OnOrderCancelled(order);
}

// Event handler for when an order is canceled
void Orderbook::OnOrderCancelled(OrderPointer order)
{
    // Update internal data for the price level where the order was
    UpdateLevelData(order->GetPrice(), order->GetRemainingQuantity(), LevelData::Action::Remove);
}

// Event handler for when a new order is added
void Orderbook::OnOrderAdded(OrderPointer order)
{
    // Update data for the price level where the new order was added
    UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(), LevelData::Action::Add);
}

// Event handler for when an order is matched
void Orderbook::OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled)
{
    // Update data based on whether the order was fully matched or partially filled
    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}

// Function to update internal price level data
void Orderbook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action)
{
    auto& data = data_[price]; // Retrieve or create data for the specified price level

    // Update the order count based on the action (Add or Remove)
    data.count_ += action == LevelData::Action::Remove ? -1 : action == LevelData::Action::Add ? 1 : 0;

    // Update the quantity at the price level based on the action
    if (action == LevelData::Action::Remove || action == LevelData::Action::Match)
    {
        data.quantity_ -= quantity;
    }
    else
    {
        data.quantity_ += quantity;
    }

    // If no orders remain at this price level, remove the level
    if (data.count_ == 0)
        data_.erase(price);
}
 