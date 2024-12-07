#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <vector>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>

// Enum to define order types
enum class OrderType {
    GoodTillCancel,  // The order remains in the book until canceled
    FillandKill      // The order is executed immediately or canceled
};

// Enum to define order sides
enum class Side {
    Buy,  // Buy side of the order
    Sell  // Sell side of the order
};

// Type aliases for better code readability
using Price = std::int32_t;       // Price represented as a signed 32-bit integer
using Quantity = std::uint32_t;   // Quantity represented as an unsigned 32-bit integer
using OrderId = std::uint64_t;    // Order ID represented as an unsigned 64-bit integer

// Structure representing a single level in the order book (price and total quantity)
struct LevelInfo {
    Price price_;      // The price level
    Quantity quantity_; // The total quantity at the price level
};

// Type alias for a vector of price levels in the order book
using LevelInfos = std::vector<LevelInfo>;

// Class to hold information about the bid and ask levels in the order book
class OrderbookLevelInfos {
public:
    // Constructor to initialize bid and ask levels
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : bids_{bids}, asks_{asks} {}

    // Getters for bid and ask levels
    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }

private:
    LevelInfos bids_; // Vector of bid levels
    LevelInfos asks_; // Vector of ask levels
};

// Class representing an individual order
class Order {
public:
    // Constructor to initialize an order with its details
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_{orderType}, orderId_{orderId}, side_{side},
          price_{price}, initialQuantity_{quantity}, remainingQuantity_{quantity} {}

    // Getters for order properties
    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    OrderType GetOrderType() const { return orderType_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return GetInitialQuantity() - GetRemainingQuantity(); }
    bool isFilled() const {return GetRemainingQuantity() == 0 s;}
    // Update remaining quantity when an order is partially or fully filled
    void Fill(Quantity quantity) {
        if (quantity > GetRemainingQuantity()) {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity", GetOrderId()));
        }
        remainingQuantity_ -= quantity;
    }

private:
    OrderType orderType_;        // Type of the order
    OrderId orderId_;            // Unique identifier for the order
    Side side_;                  // Buy or Sell side
    Price price_;                // Price of the order
    Quantity initialQuantity_;   // Original quantity of the order
    Quantity remainingQuantity_; // Quantity left to be executed
};

// Type aliases for managing orders
using OrderPointer = std::shared_ptr<Order>;     // Pointer to an Order object
using OrderPointers = std::list<OrderPointer>;   // List of Order pointers

// Class to modify an existing order
class OrderModify {
public:
    // Constructor to initialize modification details
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
        : orderId_{orderId}, price_{price}, side_{side}, quantity_{quantity} {}

    // Getters for order modification details
    OrderId GetOrderId() const { return orderId_; }
    Price GetPrice() const { return price_; }
    Side GetSide() const { return side_; }
    Quantity GetQuantity() const { return quantity_; }

    // Convert the modification to a new Order pointer
    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderId orderId_;  // Order ID to be modified
    Price price_;      // Updated price
    Side side_;        // Updated side
    Quantity quantity_; // Updated quantity
};

// Structure representing information about a trade
struct TradeInfo {
    OrderId orderId_;  // Order ID involved in the trade
    Price price_;      // Trade price
    Quantity quantity; // Quantity traded
};

// Class representing a single trade involving a bid and an ask
class Trade {
public:
    // Constructor to initialize trade details
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade_{bidTrade}, askTrade_{askTrade} {}

    // Getters for bid and ask trade details
    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_; // Trade information on the bid side
    TradeInfo askTrade_; // Trade information on the ask side
};

// Type alias for a collection of trades
using Trades = std::vector<Trade>;

// Orderbook class to manage bid and ask orders
class Orderbook {
private:
    // Internal structure to represent an entry in the order book
    struct OrderEntry {
        OrderPointer order_{nullptr};              // Pointer to the order
        OrderPointers::iterator location_;         // Location of the order in the order pointers list
    };

    // Maps for bid and ask orders, sorted by price
    std::map<Price, OrderPointers, std::greater<Price>> bids_; // Bid orders sorted descending
    std::map<Price, OrderPointers, std::less<Price>> asks_;    // Ask orders sorted ascending

    // Map to track orders by their unique ID
    std::unordered_map<OrderId, OrderEntry> orders_;

    // Check if a trade can match for a given side and price
    bool canMatch(Side side, Price price) const {
        if (side == Side::Buy) {
            // Check if the best ask price satisfies the buy condition
            if (asks_.empty()) return false;
            const auto& [bestAsk, _] = *asks_.begin();
            return price >= bestAsk;
        } else {
            // Check if the best bid price satisfies the sell condition
            if (bids_.empty()) return false;
            const auto& [bestBid, _] = *bids_.begin();
            return price <= bestBid;
        }
    }

    Trades MaatchOrders()
    {
        Trades trades;
        trades.reserve(orders_.size());
        
        while(true)
        {
            if (bids_.empty()  || asks_.empty())
                break;

            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

               if (bidPrice< askPrice)
                    break;

                while (bids.size() && asks.size())
                {
                    auto&& bid = bids.front();
                    auto&& ask = asks.front();

                    Quantity quantity  = std:: min(bid-> GetRemainingQuantity(), ask->GetRemainingQuantity());

                    bid->Fill(quantity);
                    ask->Fill(quantity);

                    if(bid-> isFilled() ) 
                    {
                        bids.pop_front();
                        orders_.erase(bid->GetOrderId());
                    }
                    if (ask->isFilled())
                    {
                        asks.pop_front();
                        orders_.erase(ask->GetOrderId());
                    }
                    if (bids.empty())
                        bids_.erase(bidPrice);
                    
                    if (asks.empty())
                        asks_.erase(askPrice);

                    trades.push_back(Trade{ 
                        TradeInfo{bid -> GetOrderId(), bid ->GetPrice(),quantity},
                        TradeInfo{ask -> GetOrderId(), ask ->GetPrice(),quantity}
                        }) ;  


                }    

        }

    }
};

int main() {
    return 0;
}
 