cmake_minimum_required(VERSION 3.14)
project(LimitOrderBook VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Release mode by default for max performance
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(OrderBookEngine 
    src/main.cpp 
    src/OrderBook.cpp
)

target_include_directories(OrderBookEngine PRIVATE src)
