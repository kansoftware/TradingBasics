# Trading basics library

## About

These classes, types and functions are suitable for creating trading strategies and bots using C++ compilers.

## Карта модулей

### include/ — заголовки

| Файл | Назначение |
|---|---|
| `BarPeriod.h` | enum `TBarPeriod` — периоды баров; имена и конверсии периодов |
| `BasisOfStrategy.h` | базовые типы и утилиты стратегий: бары/серии цен, дата-время, `RoundTo`/`TruncTo`/`CeilTo`, `trim` |
| `CIndicatorES.h` | модель Хольта с демпфированным трендом (экспоненциальное сглаживание) на базе метода Пауэлла |
| `Comparers.h` | сравнение чисел с плавающей точкой с допуском (epsilon) |
| `DelphisRound.h` | округления и приведения типов как в Delphi (`ToDouble`/`ToInt`/`ToSize_t` и др.) |
| `FileManipulator.h` | шаблоны типизированной записи/чтения файлов (сырой бинарный IO) |
| `Forecasting.h` | прогноз следующего значения ряда моделью Хольта (`TForecastResult`) |
| `Indicators.h` | технические индикаторы: TrueRange/ADX, MA (SMA/EMA/KAMA), SAR, ZigZag, MACD, Bollinger |
| `LiveSAR.h` | Parabolic SAR с пошаговым пересчётом на живом ряде |
| `PnlAction.h` | сделки и PnL-серии: деньги, просадка, статистика, коэффициенты качества |
| `PointerValidater.h` | проверки указателей (`isValidPointer`), `FreeAndNil` |
| `PowellsMethod.h` | метод Пауэлла безусловной минимизации функции N переменных |
| `Prices.h` | базовые определения для работы с ценами |
| `RollRangeBar.h` | скользящий индикатор Min-Max Range для баров |
| `RollRange.h` | скользящий индикатор Min-Max Range (`RollRange`, `RollRange_with_tollerance`) |
| `RollSMA.h` | SMA по окну из n значений, пересчёт за O(1) на Add |
| `RubberBuffer.h` | «резиновый» (растущий) буфер: min/max/Mean/err(TR) |
| `StringHasher.h` | перевод чисел в код-строку по алфавиту [0-9a-z] (`IntToCode`) |
| `VolatilityBarBuffer.h` | min/max/ATR/err(TR) по буферу баров (аналог `VolatilityBuffer.h`) |
| `VolatilityBuffer.h` | min/max/Mean/err(TR) по заранее заполненному буферу |

### src/ — реализации

| Файл | Назначение |
|---|---|
| `BarPeriod.cpp` | реализация `BarPeriod.h`: имена периодов (`getBarPeriodName`) |
| `BasisOfStrategy.cpp` | реализация `BasisOfStrategy.h`: `trim`, бары/серии, дата-время, округление цены к шагу |
| `CIndicatorES.cpp` | реализация `CIndicatorES.h`: подбор параметров и расчёт модели Хольта |
| `Forecasting.cpp` | реализация `Forecasting.h`: функция `Forecast` |
| `Indicators.cpp` | реализация `Indicators.h`: перечисленные индикаторы |
| `PnlAction.cpp` | реализация `PnlAction.h`: `DealsToPnLs`, `CalcDrawDown`, `PnLsToMoneyResult` |
| `PowellsMethod.cpp` | реализация метода Пауэлла |
| `StringHasher.cpp` | реализация `StringHasher.h`: `IntToCode`, алфавит [0-9a-z] |

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
sudo apt install ./TradingBasics-0.0.4-Linux.deb
```

Add the following lines to cmake:

```cmake
include_directories( /usr/include/kansoftware/)
link_directories( /usr/lib/kansoftware/)
target_link_libraries( #YOUR_TARGET_PROJECT# TradingBasics )

```
