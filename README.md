# TradingBasics
## About
Это общая для нескольких моих проектов библиотека включающая в себя типы, классы
и функции облегчающая работу с C++ при создании торговых стратигий и роботов.

## Depends
git cmake gcc g++

## Installing
```bash
git clone https://github.com/kansoftware/TradingBasics.git
cd TradingBasics
mkdir build
cd build
cmake -DCMAKE_C_FLAGS="-O3 -march=native -DNDEBUG" -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" ..
cmake --build . -- -j 4 VERBOSE=1
```

## Usage
Into your project include folder TradingBasics/include. Add builded lib to your project.
