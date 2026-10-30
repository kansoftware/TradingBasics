/**
 * \file Indicators.h
 * \brief Модуль реализующий индикаторы для одноногих стратегий
 * \author kan <kansoftware.ru>
 * \since 2015-11-10
 * \date 2019-07-31
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 *
 * Общие соглашения модуля:
 *  - Индикатор возвращает (или заполняет) TPriceSeries той же длины, что и
 *    входная серия; DateTime точек результата копируется из входа.
 *  - Точки прогрева (warm-up) помечаются bad-ценой GetBadPrice() и Volume=0;
 *    посчитанные точки несут Volume=1, если иное не оговорено в доке функции.
 *  - Параметр aLag (там, где есть) — дополнительный сдвиг серии вправо:
 *    первые aLag точек результата всегда bad, seed/окно MA строятся
 *    на отрезке [aLag, period+aLag) входной серии.
 */

#ifndef BACKTESTER_INDICATORS_H
#define BACKTESTER_INDICATORS_H

#include "BasisOfStrategy.h"
#include "BarPeriod.h"

/**
 *  \brief Число ядер для многопоточных расчётов
 *  \deprecated нигде не используется
 */
const size_t gUsingCPU = 4;

/**
 *  \brief True Range: максимум размаха бара с учётом гэпа к предыдущему Close
 *  \param aBars Входная серия баров
 *  \return Серия TR: первая точка = High-Low первого бара, далее
 *          max(High, prevClose) - min(Low, prevClose). Volume точек = Volume бара.
 *          Пустой вход -> пустая серия. Bad-значений нет (все точки считаются).
 */
TPriceSeries _TrueRange( const TBarSeries & aBars );

/**
 *  \brief Average True Range: сглаженный Wilder'ом True Range
 *  \param aBars Входная серия баров
 *  \param aPeriod Период усреднения (> 0)
 *  \return _SmoothedMA(_TrueRange(aBars), aPeriod, 1):
 *          первые aPeriod точек bad, далее рекуррентное сглаживание.
 */
TPriceSeries _AverageTrueRange( const TBarSeries & aBars, const int aPeriod );

/**
 *  \brief Направленное движение +DM/-DM (Directional Movement)
 *  \param aBars Входная серия баров
 *  \param aoDMp Выход: +DM (рост High относительно предыдущего бара)
 *  \param aoDMn Выход: -DM (падение Low относительно предыдущего бара)
 *  \return true при успехе; false, если в серии меньше двух баров.
 *          Первая точка обеих серий нулевая, Volume = Volume бара.
 */
bool _DM( const TBarSeries & aBars,
            TPriceSeries & aoDMp,
            TPriceSeries & aoDMn );

/**
 *  \brief Индикаторы направленного движения +DI/-DI (Wilder)
 *  \param aBars Входная серия баров
 *  \param aPeriod Период сглаживания (>= 2)
 *  \param aoDMIp Выход: +DI, в процентах (0..100)
 *  \param aoDMIn Выход: -DI, в процентах (0..100)
 *  \return true при успехе; false при aPeriod < 2, короткой серии или
 *          ошибке _DM. Первые aPeriod точек — bad, далее сглаженные значения.
 */
bool _DI( const TBarSeries & aBars, const int aPeriod,
            TPriceSeries & aoDMIp,
            TPriceSeries & aoDMIn );

/**
 *  \brief Average Directional Index: сила тренда (0..100), без направления
 *  \param aBars Входная серия баров
 *  \param aPeriod Период (>= 2)
 *  \return Серия ADX = _SmoothedMA(DX, aPeriod, aPeriod): первые 2*aPeriod-1
 *          точек bad. Пустая серия, если _DI завершился неудачей
 *          (короткий вход, aPeriod < 2).
 */
TPriceSeries _ADX( const TBarSeries & aBars, const int aPeriod );

/**
 *  \brief Simple Moving Average: среднее по окну из aPeriod точек
 *  \param aPrices Входная серия цен
 *  \param aPeriod Период усреднения (> 0)
 *  \param aLag Число bad-точек в начале результата (сдвиг серии вправо)
 *  \return Серия SMA: первые aLag точек и ещё (aPeriod-1) точек после них —
 *          bad; значение в точке i — среднее точек окна [i-period+1, i].
 *          Бросает std::logic_error при aPeriod <= 0.
 *          Если длина входа <= aPeriod — все точки bad.
 */
TPriceSeries _SimpleMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );

/**
 *  \brief Exponential Moving Average: seed = среднее, далее рекуррентность
 *  \param aPrices Входная серия цен
 *  \param aPeriod Период (> 0); вес w = 2/(aPeriod+1)
 *  \param aLag Сдвиг серии вправо на aLag точек
 *  \return Первые aLag точек — bad; точка aPeriod+aLag-1 — seed (среднее
 *          точек [aLag, aPeriod+aLag)); далее
 *          v[i] = price[i]*w + v[i-1]*(1-w). Точки (aLag, aPeriod+aLag-1) — bad.
 *          Если длина входа <= aPeriod+aLag — все точки bad.
 *          Бросает std::logic_error при aPeriod <= 0.
 */
TPriceSeries _ExponentMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );

/**
 *  \brief Smoothed Moving Average (Wilder): seed = среднее, далее рекуррентность
 *  \param aPrices Входная серия цен
 *  \param aPeriod Период сглаживания (> 0)
 *  \param aLag Сдвиг серии вправо на aLag точек
 *  \return Первые aLag точек — bad; точка aPeriod+aLag-1 — seed (среднее
 *          точек [aLag, aPeriod+aLag)); далее
 *          v[i] = (price[i] + v[i-1]*(aPeriod-1))/aPeriod.
 *          Точки (aLag, aPeriod+aLag-1) — bad.
 *          Если длина входа <= aPeriod+aLag — все точки bad.
 *          Бросает std::logic_error при aPeriod <= 0.
 */
TPriceSeries _SmoothedMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );

/**
 *  \brief KAMA: Kaufman Adaptive Moving Average
 *  \param aPrices Входная серия цен
 *  \param aPeriod Период окна эффективности (> 0)
 *  \param aCoeff Показатель степени сглаживания (обычно 2)
 *  \param aFastPeriod Период быстрой константы (обычно 2)
 *  \param aSlowPeriod Период медленной константы (обычно 30)
 *  \param aLag Число bad-точек в начале результата
 *  \return Первые max(aLag, aPeriod) точек — bad; далее
 *          v[i] = c*price[i] + (1-c)*v[i-1], где c — коэффициент,
 *          зависящий от Efficiency Ratio окна [i-aPeriod, i].
 *          На плоской серии совпадает с ценой.
 *          Бросает std::logic_error при aPeriod <= 0, aFastPeriod <= 0
 *          или aSlowPeriod <= 0.
 */
TPriceSeries _KAMA( const TPriceSeries & aPrices, const int aPeriod, const double aCoeff = 2.0, const double aFastPeriod = 2.0, const double aSlowPeriod = 30.0, const size_t aLag = 0 );

/**
 *  \brief Задержка стабилизации SAR
 *  \deprecated нигде не используется
 */
const size_t gSARStabilizationDelay = 200 ;

/**
 *  \brief Parabolic SAR по всей серии баров
 *  \param aBars Входная серия баров
 *  \param aAf Шаг ускорения (acceleration factor), обычно 0.02
 *  \param aMaxAf Максимальный шаг ускорения, обычно 0.2
 *  \return Серия SAR; старт — Buy: точка 0 = Low первого бара (прогрев),
 *          далее текущие значения SAR, Volume=1. Направление переключается,
 *          когда цена пересекает SAR. Пустой вход -> пустая серия.
 */
TPriceSeries _ParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );

/**
 *  \brief Intraday Parabolic SAR: как _ParabolicSar, но с сбросом на границе дня
 *  \param aBars Входная серия баров
 *  \param aAf Шаг ускорения, обычно 0.02
 *  \param aMaxAf Максимальный шаг ускорения, обычно 0.2
 *  \return Серия SAR; в первый бар нового дня (не IsOneDay с предыдущим)
 *          состояние сбрасывается: SAR = Low бара, направление — Buy.
 *          Пустой вход -> пустая серия.
 */
TPriceSeries _IntradayParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );

/**
 *  \brief Absolute ZigZag: разворот при движении против ветки на aGap в абсолютных единицах цены
 *  \param aBars Входная серия баров
 *  \param aGap Минимальный разворот в единицах цены (> 0)
 *  \return Серия того же размера: Price ненулевой только в точках разворотов
 *          (и в стартовой точке), промежуточные точки — 0. Volume — маркер
 *          ветки: +1 — восходящая ветка (Buy), -1 — нисходящая (Sell), 0 не
 *          используется (маркер ставится в каждой точке). Пустой вход ->
 *          пустая серия. Бросает std::logic_error при aGap <= 0.
 */
TPriceSeries _AbsoluteZigZag( const TBarSeries & aBars, const double aGap );

/**
 *  \brief MACD: линия = EMA(fast) - EMA(slow), сигнал = EMA(линии, smooth)
 *  \param aPrices Входная серия цен
 *  \param aFastPeriod Период быстрой EMA (> 0)
 *  \param aSlowPeriod Период медленной EMA (> 0)
 *  \param aSmoothPeriod Период сигнальной EMA (> 0)
 *  \return Серия сигнальной линии; bad до точки smooth+max(fast,slow)-2.
 *          Линия MACD до точки max(fast,slow)-1 — bad.
 *          Бросает std::logic_error при любом периоде <= 0.
 */
TPriceSeries _MACD( const TPriceSeries & aPrices, const int aFastPeriod, const int aSlowPeriod, const int aSmoothPeriod );

/**
 *  \brief ZigZag: разворот при движении против ветки на aGap (доля цены)
 *  \param aBars Входная серия баров
 *  \param aGap Минимальный разворот как доля от головы ветки (> 0);
 *              разворот вниз при Low < head*(1-aGap), вверх при High >= head*(1+aGap)
 *  \return Серия того же размера: Price ненулевой только в точках разворотов
 *          (и в стартовой точке), промежуточные точки — 0. Volume — маркер
 *          ветки: +1 — восходящая ветка (Buy), -1 — нисходящая (Sell).
 *          Пустой вход -> пустая серия.
 *          Бросает std::logic_error при aGap <= 0.
 */
TPriceSeries _ZigZag( const TBarSeries & aBars, const double aGap );

/**
 *  \brief Прогноз по прошлым данным с доверительными границами
 *  \param aPrices Входная серия цен
 *  \param aForecastPeriod Горизонт прогноза (число прогревочных точек)
 *  \param aConfidenceIntervals Ширина доверительного интервала
 *  \param aoUpperBorder Выход: верхняя граница прогноза
 *  \param aoForecast Выход: точечный прогноз
 *  \param aoLowerBorder Выход: нижняя граница прогноза
 *  \return true при успехе; false, если длина входа меньше aForecastPeriod.
 *          Первые aForecastPeriod точек всех серий — bad; точка i — прогноз
 *          по окну [i-aForecastPeriod, i); точка с неспрогнозированным
 *          значением — bad.
 */
bool _Forecasting( const TPriceSeries & aPrices, const size_t aForecastPeriod, const double aConfidenceIntervals,
                    TPriceSeries & aoUpperBorder,
                    TPriceSeries & aoForecast,
                    TPriceSeries & aoLowerBorder );

/**
 *  \brief Скользящие минимум (по Low) и максимум (по High) окна баров
 *  \param aBars Входная серия баров
 *  \param aPeriod Размер окна (> 0)
 *  \param aoMin Выход: минимум окна по Low
 *  \param aoMax Выход: максимум окна по High
 *  \param aTouch Режим подсчёта касаний (см. Volume)
 *  \return true при успехе; false при aPeriod <= 0 или коротком входе.
 *          Первые aPeriod точек — bad. Volume: без aTouch — 1; при aTouch — 1,
 *          если текущий бар сам достиг экстремума окна, иначе 0, причём
 *          подряд идущие равные экстремумы аккумулируют число касаний в Volume.
 */
bool _RollMinMax( const TBarSeries & aBars, const int aPeriod,
            TPriceSeries & aoMin,
            TPriceSeries & aoMax,
            const bool aTouch = false );

/**
 *  \brief Устаревшая реализация _RollMinMax на RollRange (эталон для сверки)
 *  \param aBars Входная серия баров
 *  \param aPeriod Размер окна (> 0)
 *  \param aoMin Выход: минимум окна по Low
 *  \param aoMax Выход: максимум окна по High
 *  \param aTouch Режим подсчёта касаний (см. _RollMinMax)
 *  \return true при успехе; false при aPeriod <= 0 или коротком входе.
 *          Семантика совпадает с _RollMinMax (min/max), квантиль не используется.
 */
bool _RollMinMax_old( const TBarSeries & aBars, const int aPeriod,
            TPriceSeries & aoMin,
            TPriceSeries & aoMax,
            const bool aTouch = false );

/**
 *  \brief Уровни поддержки/сопротивления: min/max окна с допуском и числом касаний
 *  \param aBars Входная серия баров
 *  \param aPeriod Размер окна (> 0)
 *  \param aMinTouch Минимальное число касаний уровня
 *  \param aTollerance Допуск при склейке близких уровней
 *  \param aoMin Выход: уровни поддержки (минимумы окна по Low)
 *  \param aoMax Выход: уровни сопротивления (максимумы окна по High)
 *  \return true при успехе; false при aPeriod <= 0 или коротком входе.
 *          Первые aPeriod точек — bad, далее значения окна, Volume=1.
 *          Значения вне допустимых пределов выравниваются по уровню (RollRange_with_tollerance).
 */
bool _SupportRessistance(
    const TBarSeries & aBars,
    const int aPeriod,
    const size_t aMinTouch,
    const double aTollerance,
    TPriceSeries & aoMin,
    TPriceSeries & aoMax );

/**
 *  \brief Минимум/максимум баров в окне вперёд от текущей точки
 *  \param aBars Входная серия баров
 *  \param aTimeDelta Ширина окна вперёд, в секундах
 *  \param aoMin Выход: минимум Low баров в интервале (i, i+aTimeDelta]
 *  \param aoMax Выход: максимум High баров в интервале (i, i+aTimeDelta]
 *  \return true при успехе; false, если в серии меньше 5 баров.
 *          Точка i учитывает только бары строго после i; последняя точка — bad.
 */
bool _ForwardMinMax( const TBarSeries & aBars, const size_t aTimeDelta,
            TPriceSeries & aoMin,
            TPriceSeries & aoMax );

/**
 *  \brief Полосы Боллинджера: mean ± aSigma*σ вокруг _SimpleMA
 *  \param aPrices Входная серия цен
 *  \param aPeriod Период окна (> 0)
 *  \param aSigma Множитель сигмы
 *  \param aoMin Выход: нижняя полоса
 *  \param aoMean Выход: средняя линия (_SimpleMA)
 *  \param aoMax Выход: верхняя полоса
 *  \return true при успехе; false, если длина входа <= aPeriod.
 *          Первые aPeriod-1 точек — bad; σ — популяционная (деление на aPeriod).
 */
bool _BollingerBands( const TPriceSeries & aPrices, const int aPeriod, const double aSigma,
    TPriceSeries & aoMin,
    TPriceSeries & aoMean,
    TPriceSeries & aoMax );

/**
 *  \brief Размер канала: максимум High минус минимум Low окна
 *  \param aBars Входная серия баров
 *  \param aPeriod Размер окна (> 0)
 *  \return Серия размахов окна; первые aPeriod точек — bad. Пустая серия,
 *          если _RollMinMax завершился неудачей (короткий вход, aPeriod <= 0).
 */
TPriceSeries _ChannelSize( const TBarSeries & aBars, const int aPeriod );

/**
 *  \brief Stochastic %K: положение Close в диапазоне окна, 0..100
 *  \param aBars Входная серия баров
 *  \param aPeriod Размер окна (> 0)
 *  \return Серия %K = 100*(Close - min)/(max - min); при нулевом диапазоне
 *          окна — 50. Первые aPeriod точек — bad. При коротком входе
 *          (aPeriod <= 0 или size < aPeriod) — серия входной длины
 *          из нулевых тиков.
 */
TPriceSeries _Stochastic( const TBarSeries & aBars, const int aPeriod );

#endif //BACKTESTER_INDICATORS_H
