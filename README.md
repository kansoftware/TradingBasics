# Trading Basics library

## About
This is classes, types and functions for creating trading strategies and bots with C++ compilers.

## Depends
git cmake gcc g++

## Installing
```bash
git clone https://github.com/kansoftware/TradingBasics.git
cd TradingBasics
mkdir build; cd build
cmake -DCMAKE_C_FLAGS="-O3 -march=native -DNDEBUG" -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" ..
cmake --build . -- -j 4
```

## Testing
```bash
cd ..
mkdir buildtest; cd buildtest
cmake -DBUILD_TESTING=ON ..
cmake --build . -- -j 4
make test
```


## Usage
Into your project include folder TradingBasics/include. Add builded lib to your project.
The sample for cmake:
```cmake
set( TRADING_BASICS_PATH "#YOUR_PATH_TO#/TradingBasics/" )

add_library(TRADING_BASICS SHARED IMPORTED)
set_target_properties(TRADING_BASICS PROPERTIES
    IMPORTED_LOCATION "${TRADING_BASICS_PATH}build/libTradingBasics.a"
    INTERFACE_INCLUDE_DIRECTORIES "${TRADING_BASICS_PATH}include"
)

target_link_libraries( #YOUR_TARGET_PROJECT# TRADING_BASICS )

```
or
```cmake
set( TRADING_BASICS_PATH "$ENV{HOME}/cpp/companents/TradingBasics/" )
include_directories( "${TRADING_BASICS_PATH}include" )
link_directories( "${TRADING_BASICS_PATH}build" )

target_link_libraries( #YOUR_TARGET_PROJECT# -lTradingBasics )
```
