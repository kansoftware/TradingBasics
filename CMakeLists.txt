cmake_minimum_required(VERSION 3.3)
project( TradingBasics )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall" )

include_directories( "include" )

set( SOURCE_FILES
     src/BarPeriod.cpp
     src/BasisOfStrategy.cpp
     src/CIndicatorES.cpp
     src/Forecasting.cpp
     src/Indicators.cpp
     src/PnlAction.cpp
     src/PowellsMethod.cpp
     src/Prices.cpp
)

add_library(TradingBasics ${SOURCE_FILES})
