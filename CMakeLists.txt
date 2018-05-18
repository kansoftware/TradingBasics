cmake_minimum_required( VERSION 3.0 )
project( TradingBasics )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -Wlogical-op -Wconversion -Wno-unused-parameter -Wsuggest-attribute=noreturn -Wsuggest-override -Wold-style-cast" )

enable_testing()
if( BUILD_TESTING )
    add_subdirectory( "gtests" )
endif()

include_directories( "include" )

find_package(Boost 1.50 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set( SOURCE_FILES
     src/BarPeriod.cpp
     src/BasisOfStrategy.cpp
     src/CIndicatorES.cpp
     src/Forecasting.cpp
     src/Indicators.cpp
     src/PnlAction.cpp
     src/PowellsMethod.cpp
     src/Prices.cpp
     src/StringHasher.cpp
)

add_library(TradingBasics ${SOURCE_FILES})
