# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++20

# Define source and header files
SRCS = main.cpp Orderbook.cpp
HEADERS = Constants.h OrderType.h LevelInfo.h Order.h Orderbook.h Trade.h \
          OrderModify.h OrderbooklevelInfos.h TradeInfo.h Side.h Usings.h

# Output executable name
OUTPUT = OrderBook

# Default target
all: $(OUTPUT)

# Link the object files to create the final executable
$(OUTPUT): $(SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build .o files from .cpp files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

# Clean up the compiled files
clean:
	rm -f $(OUTPUT) *.o

