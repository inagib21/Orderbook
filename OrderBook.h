#pragma once // Ensures this file is included only once during compilation.

#include <map>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Usings.h" // Custom type aliases and utilities.
#include "Order.h" // Order class definition.
#include "OrderModify.h" // Order modification class definition.
#include "OrderbookLevelInfos.h" // Class providing order book level information.
#include "Trade.h" // Trade-related definitions and data structures.

// The `Orderbook` class manages the collection of buy and sell orders, tracks order book levels, 
// and facilitates the matching and execution of trades.
class Orderbook
{
private:
    // Represents an entry in the order book, tying an order to its location in the price level.
    struct OrderEntry
    {
        OrderPointer order_{ nullptr }; // Pointer to the order.
        OrderPointers::iterator location_; // Iterator pointing to the order's location within its level.
    };

    // Data structure for storing quantity and count of orders at a given price level.
    struct LevelData
    {
        Quantity quantity_{ }; // Total quantity at this level.
        Quantity count_{ }; // Number of orders at this level.

        // Actions to track updates to levels: adding, removing, or matching orders.
        enum class Action
        {
            Add,    // Adding a new order.
            Remove, // Removing an existing order.
            Match,  // Matching orders for execution.
        };
    };

    // Internal data members
    std::unordered_map<Price, LevelData> data_; // Tracks price level data for analysis and matching.
    std::map<Price, OrderPointers, std::greater<Price>> bids_; // Buy orders, sorted by descending price.
    std::map<Price, OrderPointers, std::less<Price>> asks_; // Sell orders, sorted by ascending price.
    std::unordered_map<OrderId, OrderEntry> orders_; // Mapping of orders by ID for quick lookup.
    mutable std::mutex ordersMutex_; // Mutex for thread-safe operations on the orders.
    std::thread ordersPruneThread_; // Thread to manage periodic pruning of "Good-For-Day" orders.
    std::condition_variable shutdownConditionVariable_; // Condition variable to signal shutdown.
    std::atomic<bool> shutdown_{ false }; // Flag to indicate if the system is shutting down.

    // Internal helper methods
    void PruneGoodForDayOrders(); // Removes "Good-For-Day" orders when necessary.
    void CancelOrders(OrderIds orderIds); // Cancels a batch of orders.
    void CancelOrderInternal(OrderId orderId); // Internal logic for cancelling a single order.
    void OnOrderCancelled(OrderPointer order); // Handles the event of an order being cancelled.
    void OnOrderAdded(OrderPointer order); // Handles the event of an order being added.
    void OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled); // Handles matched orders.
    void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action); // Updates level data.

    // Matching logic
    bool CanFullyFill(Side side, Price price, Quantity quantity) const; // Checks if an order can be fully filled.
    bool CanMatch(Side side, Price price) const; // Checks if orders can be matched at a given price.
    Trades MatchOrders(); // Matches orders in the order book and generates trades.

public:
    // Constructors and destructor
    Orderbook(); // Default constructor.
    Orderbook(const Orderbook&) = delete; // Copy constructor is deleted to prevent copying.
    void operator=(const Orderbook&) = delete; // Copy assignment is deleted to prevent copying.
    Orderbook(Orderbook&&) = delete; // Move constructor is deleted to prevent moving.
    void operator=(Orderbook&&) = delete; // Move assignment is deleted to prevent moving.
    ~Orderbook(); // Destructor to clean up resources.

    // Public interface for managing orders
    Trades AddOrder(OrderPointer order); // Adds a new order to the book.
    void CancelOrder(OrderId orderId); // Cancels an existing order by ID.
    Trades ModifyOrder(OrderModify order); // Modifies an existing order.

    // Utility methods
    std::size_t Size() const; // Returns the total number of orders in the book.
    OrderbookLevelInfos GetOrderInfos() const; // Retrieves detailed information about order book levels.
};
