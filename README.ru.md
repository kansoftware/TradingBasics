# Библиотека TradingBasics

**Языки:** [English](README.md) | Русский

## О библиотеке

Классы, типы и функции, пригодные для создания торговых стратегий и ботов на компиляторах C++.

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

## Зависимости

git cmake gcc g++ libboost-math-dev

## Установка

```bash
git clone https://github.com/kansoftware/TradingBasics.git
cd TradingBasics
mkdir build; cd build
cmake -DCMAKE_C_FLAGS="-march=native -DNDEBUG" -DCMAKE_CXX_FLAGS="-march=native -DNDEBUG" ..
cmake --build . -- -j 4

```

Для корректной работы нужен пакет libboost-math-dev, либо можно задать переменную BOOST_ROOT.

```bash
cmake -DBOOST_ROOT=/you_path_to_boost/boost ..

```

## Тестирование

### Зависимости
sudo apt-get install libgtest-dev

```bash
cmake -DBUILD_TESTING=ON ..
cmake --build . -- -j `nproc`
ctest
cpack -G DEB
```

## Использование

Подключите каталоги TradingBasics в свой проект и соберите проект вместе с библиотекой.

Установка пакета:
```bash
sudo apt install ./TradingBasics-1.0.0-Linux.deb
```

Добавьте в cmake следующие строки:

```cmake
include_directories( /usr/include/kansoftware/)
link_directories( /usr/lib/kansoftware/)
target_link_libraries( #YOUR_TARGET_PROJECT# TradingBasics )

```
