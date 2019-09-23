cmake_minimum_required( VERSION 3.0 )
project( TradingBasics )

find_package(Boost 1.50 REQUIRED)
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -Wlogical-op -Wconversion -Wno-unused-parameter -Wsuggest-attribute=noreturn -Wsuggest-override -Wold-style-cast" )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -gdwarf-2" )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1" )
    
    message( WARNING "debug mode" )

else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast" )

    message( WARNING "release mode" )

endif() 


enable_testing()
if( BUILD_TESTING )
    add_subdirectory( "gtests" )
endif()

include_directories( "include" )

set( SOURCE_FILES
     src/BarPeriod.cpp
     src/BasisOfStrategy.cpp
     src/CIndicatorES.cpp
     src/Forecasting.cpp
     src/Indicators.cpp
     src/PnlAction.cpp
     src/PowellsMethod.cpp
     src/StringHasher.cpp
)

add_library(TradingBasics ${SOURCE_FILES})
