cmake_minimum_required( VERSION 3.7 )
project( TradingBasics_tests VERSION "0.0.1")


set( CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -O0 -g -isystem ${GOOGLE_SOURCE_FILES}/googletest/include" ) #-fPIC

set( THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# set( SRC_FILES
#     main.cpp
#     TestBasic.cpp
#     TestStringHasher.cpp
#     TestPnLs.cpp
# )
file( GLOB SRC_FILES
    "*.cpp"
)
add_executable( ${PROJECT_NAME} ${SRC_FILES} )

find_package( Threads REQUIRED )
target_link_libraries( ${PROJECT_NAME}
    gtest
    pthread
    TradingBasics
)

target_include_directories( ${PROJECT_NAME} PRIVATE "../include" )

#set( GOOGLE_SOURCE_FILES "$ENV{HOME}/cpp/googletest" )
#include_directories( "${GOOGLE_SOURCE_FILES}/googletest/include" )
#link_directories( "${GOOGLE_SOURCE_FILES}/build/lib" )

#set( TRADING_BASICS_PATH "$ENV{HOME}/cpp/companents/TradingBasics/" )
#include_directories( "${TRADING_BASICS_PATH}include" )
#link_directories( "${TRADING_BASICS_PATH}build" )

add_test( ${PROJECT_NAME} ${PROJECT_NAME} )
