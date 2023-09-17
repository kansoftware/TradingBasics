# Trading basics library

## About

These classes, types and functions are suitable for creating trading strategies and bots using C++ compilers.

## Depends

git cmake gcc g++ libboost-math-dev

## Installing

```bash
git clone https://github.com/kansoftware/TradingBasics.git
cd TradingBasics
mkdir build; cd build
cmake -DCMAKE_C_FLAGS="-march=native -DNDEBUG" -DCMAKE_CXX_FLAGS="-march=native -DNDEBUG" ..
cmake --build . -- -j 4

```

To work correctly, you need the libboost-math-dev package or you can set the BOOST_ROOT variable.

```bash
cmake -DBOOST_ROOT=/you_path_to_boost/boost ..

```

## Testing

### Depends
sudo apt-get install libgtest-dev

```bash
cmake -DBUILD_TESTING=ON ..
cmake --build . -- -j `nproc`
ctest
cpack -G DEB
```

## Usage

Include TradingBasics folders in your project. And build your project with the library.

Install package:
```bash
sudo apt -f TradingBasics-0.0.1-Linux.deb
```

Add the following lines to cmake:

```cmake
include_directories( /usr/include/kansoftware/)
link_directories( /usr/lib/kansoftware/)
target_link_libraries( #YOUR_TARGET_PROJECT# TradingBasics )

```
