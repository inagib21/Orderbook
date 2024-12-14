# Orderbook

Overview

The Orderbook project is a C++ implementation of a financial order book, designed to manage buy and sell orders for assets. It includes features for adding, modifying, matching, and canceling orders. The project demonstrates core concepts in financial systems, including order matching, bid-ask spreads, and trade execution.

## Features

Order Management:

Add new orders (market and limit orders).

Modify existing orders.

Cancel orders.

Order Matching:

Match buy and sell orders based on price and time priority.

Support for Good Till Cancel (GTC) and Market orders.

Thread-Safe Design:

Concurrent order processing using mutexes and condition variables.

Background thread for pruning Good For Day orders.

Trade Execution:

Generates trade records upon order matching.

Order Book Insights:

Provides order book level information, including bid/ask levels and quantities.

