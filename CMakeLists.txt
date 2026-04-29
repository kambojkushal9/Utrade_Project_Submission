cmake_minimum_required(VERSION 3.14)
project(LimitOrderBook VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Release mode by default for max performance
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Aggressive optimizations for "Top 1%" performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Oi /GL /Gy)
        add_link_options(/LTCG /OPT:REF /OPT:ICF)
    else()
        add_compile_options(-O3 -march=native -flto)
        add_link_options(-flto)
    endif()
endif()

add_executable(OrderBookEngine 
    src/main.cpp 
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)

target_include_directories(OrderBookEngine PRIVATE include)
