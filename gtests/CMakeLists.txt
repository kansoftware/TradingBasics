cmake_minimum_required( VERSION 2.8 )
project( TradingBasics_tests )

find_package( Threads REQUIRED )

set( GOOGLE_SOURCE_FILES "$ENV{HOME}/cpp/googletest" )
include_directories( "${GOOGLE_SOURCE_FILES}/googletest/include" )
link_directories( "${GOOGLE_SOURCE_FILES}/build/googletest" )

set( TRADING_BASICS_PATH "$ENV{HOME}/cpp/companents/TradingBasics/" )
include_directories( "${TRADING_BASICS_PATH}include" )
link_directories( "${TRADING_BASICS_PATH}build" )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -O0 -g -isystem ${GOOGLE_SOURCE_FILES}/googletest/include" ) #-fPIC

set( SOURCE_FILES
    main.cpp
    TestBasic.cpp
    TestStringHasher.cpp
    TestPnLs.cpp
)

add_executable( TradingBasics_tests ${SOURCE_FILES} )
target_link_libraries( TradingBasics_tests
    -lgtest
    -lpthread
    -lTradingBasics
)

add_test( TradingBasics_tests TradingBasics_tests )
