# TradingBasics library

**Languages:** English | [Русский](README.ru.md)

## About

These classes, types and functions are suitable for creating trading strategies and bots using C++ compilers.

## Module map

### include/ — headers

| File | Purpose |
|---|---|
| `BarPeriod.h` | enum `TBarPeriod` — bar periods; period names and conversions |
| `BasisOfStrategy.h` | basic strategy types and utilities: bars/price series, date-time, `RoundTo`/`TruncTo`/`CeilTo`, `trim` |
| `CIndicatorES.h` | Holt's model with damped trend (exponential smoothing) based on Powell's method |
| `Comparers.h` | floating-point comparison with tolerance (epsilon) |
| `DelphisRound.h` | rounding and type casts as in Delphi (`ToDouble`/`ToInt`/`ToSize_t` and others) |
| `FileManipulator.h` | templates for typed file writing/reading (raw binary IO) |
| `Forecasting.h` | next-value forecast of a series with the Holt model (`TForecastResult`) |
| `Indicators.h` | technical indicators: TrueRange/ADX, MA (SMA/EMA/KAMA), SAR, ZigZag, MACD, Bollinger |
| `LiveSAR.h` | Parabolic SAR with step-by-step recalculation on a live series |
| `PnlAction.h` | trades and PnL series: money, drawdown, statistics, quality ratios |
| `PointerValidater.h` | pointer checks (`isValidPointer`), `FreeAndNil` |
| `PowellsMethod.h` | Powell's method of unconstrained minimization of a function of N variables |
| `Prices.h` | basic definitions for working with prices |
| `RollRangeBar.h` | rolling Min-Max Range indicator for bars |
| `RollRange.h` | rolling Min-Max Range indicator (`RollRange`, `RollRange_with_tollerance`) |
| `RollSMA.h` | SMA over a window of n values, O(1) recalculation per Add |
| `RubberBuffer.h` | "rubber" (growing) buffer: min/max/Mean/err(TR) |
| `StringHasher.h` | converting numbers into a code string over the [0-9a-z] alphabet (`IntToCode`) |
| `VolatilityBarBuffer.h` | min/max/ATR/err(TR) over a buffer of bars (analog of `VolatilityBuffer.h`) |
| `VolatilityBuffer.h` | min/max/Mean/err(TR) over a pre-filled buffer |

### src/ — implementations

| File | Purpose |
|---|---|
| `BarPeriod.cpp` | implementation of `BarPeriod.h`: period names (`getBarPeriodName`) |
| `BasisOfStrategy.cpp` | implementation of `BasisOfStrategy.h`: `trim`, bars/series, date-time, rounding a price to the step |
| `CIndicatorES.cpp` | implementation of `CIndicatorES.h`: parameter fitting and calculation of the Holt model |
| `Forecasting.cpp` | implementation of `Forecasting.h`: the `Forecast` function |
| `Indicators.cpp` | implementation of `Indicators.h`: the indicators listed above |
| `PnlAction.cpp` | implementation of `PnlAction.h`: `DealsToPnLs`, `CalcDrawDown`, `PnLsToMoneyResult` |
| `PowellsMethod.cpp` | implementation of Powell's method |
| `StringHasher.cpp` | implementation of `StringHasher.h`: `IntToCode`, the [0-9a-z] alphabet |

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
sudo apt install ./TradingBasics-1.0.0-Linux.deb
```

Add the following lines to cmake:

```cmake
include_directories( /usr/include/kansoftware/)
link_directories( /usr/lib/kansoftware/)
target_link_libraries( #YOUR_TARGET_PROJECT# TradingBasics )

```
