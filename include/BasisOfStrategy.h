/**
 * \file BasisOfStrategy.h
 * \brief Модуль реализующий объекты для одно-ногих стратегий
 * \author kan <kansoftware.ru>
 * \since 2015-11-11
 * \date 2021-10-29
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */


#ifndef BACKTESTER_BASISOFSTRATEGY_H
#define BACKTESTER_BASISOFSTRATEGY_H

#include <vector>
#include <map>
#include <string>
#include <functional>

#include "Prices.h"
#include "BarPeriod.h"

/**
 *  \enum TMAPoint
 *  \brief Тип цены бара для построения ценовой серии из серии баров
 *  \see BarsToPriceSeries
 */
enum class TMAPoint {
    MAP_Open = 0,       /*!< Цена открытия бара */
    MAP_High = 1,       /*!< Максимум бара */
    MAP_Low = 2,        /*!< Минимум бара */
    MAP_Close = 3,      /*!< Цена закрытия бара */
    MAP_Volume = 4,     /*!< Объём бара (в поле Price тика) */
    MAP_Mid = 5,        /*!< Средняя (High+Low)/2 */
    MAP_Triple = 6      /*!< Взвешенная по объёму близкая (High+Low+Close)/3 */
};

/**
 *  \enum TDealSide
 *  \brief Сторона сделки/направление позиции
 */
enum class TDealSide { None=0, Buy=1, Sell=2, Any };

using TInnerDate = double;  /*!< Дата-время как double (unix-время, секунды) */
using TInnerTime = double;  /*!< Время суток как double (секунды от полуночи) */

const double gOneDay = 86400.0;         /*!< Секунд в сутках */
const double gOneHour = 3600.0;         /*!< Секунд в часе */
const double gStartingTime = 975888000;//GMT: Mon, 04 Dec 2000 00:00:00 GMT
/*!< Начало отсчёта времени по умолчанию: DateTime пустых баров и тиков */

/**
 *  \struct TSimpleTick
 *  \brief Точка ценовой серии: дата-время, цена и Volume-маркер
 *
 *  Volume — произвольный числовой маркер; индикаторы используют его как
 *  флаг посчитанности (0 — прогрев/bad, 1 — значение посчитано) или как
 *  маркер направления ветки (±1 в ZigZag).
 */
struct TSimpleTick {
    TInnerDate DateTime;    /*!< Дата-время (unix, секунды) */
    TPrice Price;           /*!< Цена (bad-значение — GetBadPrice()) */
    double Volume;          /*!< Маркер: объём/флаг посчитанности/направление */

    TSimpleTick(
        const TInnerDate aDateTime=gStartingTime,
        const TPrice aPrice=0.0,
        const double aVolume=0.0 ):
        DateTime(aDateTime),
        Price(aPrice),
        Volume(aVolume){;}
};

using TPriceSeries = std::vector< TSimpleTick >;    /*!< Временной ряд точек TSimpleTick */

/**
 *  \brief Коллекция индикаторов по имени
 *  \deprecated нигде не используется
 */
using TIndicators = std::map< std::string, TPriceSeries >;

/**
 *  \struct TSimpleBar
 *  \brief Бар OHLCV
 *
 *  Volume = -1 по умолчанию — признак пустого бара (см. IsValidBar).
 */
struct TSimpleBar {
    TInnerDate DateTime=gStartingTime;  /*!< Дата-время (unix, секунды) */
    TPrice Open=0.0;                    /*!< Цена открытия */
    TPrice High=0.0;                    /*!< Максимум */
    TPrice Low=0.0;                     /*!< Минимум */
    TPrice Close=0.0;                   /*!< Цена закрытия */
    double Volume=-1.0;                 /*!< Объём; -1 — пустой бар */
};

/**
 *  \brief Сброс бара в дефолтное состояние (см. TSimpleBar)
 *  \param aBar Бар, который приводится к виду gEmptyBar
 */
void Reset( TSimpleBar &aBar );

/**
 *  \brief Слияние двух баров в один (агрегация в бар старшего периода)
 *  \param aStartBar Бар-начало: берётся Open и DateTime
 *  \param aFinishBar Бар-конец: берётся Close
 *  \return Бар с Open первого, Close второго, экстремумами (High/Low) и
 *          суммой Volume. Невалидный операнд (не IsValidBar) заменяется другим.
 */
TSimpleBar operator+( const TSimpleBar &aStartBar, const TSimpleBar &aFinishBar );

/**
 *  \brief Проверка валидности бара
 *  \param aBar Проверяемый бар
 *  \return true, если Volume >= 0 и все цены OHLC валидны (IsValidPrice)
 */
bool IsValidBar( const TSimpleBar & aBar );

using TBarSeries = std::vector< TSimpleBar > ;      /*!< Временной ряд баров */
const TSimpleBar gEmptyBar{gStartingTime,0.0,0.0,0.0,0.0,-1.0}; /*!< Пустой бар */

/**
 *  \brief Принадлежность двух меток времени одним суткам (по UTC-дням)
 *  \param aLeft Первая метка (unix, секунды)
 *  \param aRight Вторая метка (unix, секунды)
 *  \return true, если Trunc(aLeft/86400) == Trunc(aRight/86400)
 */
bool IsOneDay( const TInnerDate aLeft, const TInnerDate aRight );

/**
 *  \brief Принадлежность двух меток времени одному часу (окну aDuration часов)
 *  \param aLeft Первая метка (unix, секунды)
 *  \param aRight Вторая метка (unix, секунды)
 *  \param aDuration Длительность окна в часах (> 0, по умолчанию 1)
 *  \return true, если обе метки попали в одно окно длиной aDuration часов
 */
bool IsOneHour( const TInnerDate aLeft, const TInnerDate aRight, const double aDuration = 1.0 );

/**
 *  \brief Сброс цены в bad-значение (GetBadPrice)
 *  \param aPrice Сбрасываемая цена
 */
inline void ResetPrice( TPrice & aPrice ) { aPrice = GetBadPrice(); }

/**
 *  \brief Извлечение ценовой серии из серии баров
 *  \param aBar Входная серия баров
 *  \param aType Тип извлекаемой цены (см. TMAPoint): Open/High/Low/Close/
 *              Volume или расчётные Mid=(High+Low)/2, Triple=(High+Low+Close)/3
 *  \return Серия той же длины и DateTime, что и вход; Volume точки = Volume бара
 */
TPriceSeries BarsToPriceSeries( const TBarSeries & aBar, const TMAPoint aType );

/**
 *  \brief Ресемплинг серии баров в бары большего периода
 *  \param aBars Входная серия баров (мелкий период)
 *  \param aBarPeriod Целевой период баров
 *  \return Серия агрегированных баров (слияние через operator+); DateTime
 *          результата — начало целевого периода (кратно длине периода от gStartingTime)
 *  \note Конвенция нумерации бакетов: индекс бара — Trunc((DateTime -
 *        gStartingTime) / длина периода), и бакет с индексом 0 (т.е. бары
 *        из [gStartingTime, gStartingTime+длина периода)) ОТДЕЛЬНЫМ баром
 *        не эмитится — он сливается со следующим бакетом. Результат
 *        начинается с бара второго бакета, если вход начинается ровно с
 *        gStartingTime. Данные, попадающие в бакет 0, теряют собственную
 *        агрегацию — подавайте серию, начинающуюся после
 *        gStartingTime+длины периода.
 */
TBarSeries _CreateBars( const TBarSeries & aBars, const TBarPeriod aBarPeriod );

/**
 *  \brief Текстовое представление даты-времени
 *  \param aDate Дата-время (unix, секунды)
 *  \return Строка формата ctime без завершающего перевода строки
 */
std::string DateToStr( const TInnerDate aDate );

/**
 *  \brief Время суток из даты-времени
 *  \param aDate Дата-время (unix, секунды)
 *  \return Секунды от полуночи (aDate % 86400)
 */
TInnerTime ITime( const TInnerDate aDate );

/**
 *  \brief Разбор строки времени в секунды от полуночи
 *  \param aTime Строка "HH:MM:SS" или "HH:MM"
 *  \return Секунды от полуночи; строка без ':' (невалидная) -> 0.0
 */
TInnerTime ITime( const std::string& aTime );

/**
 *  \brief Вывод бара в поток: DateToStr, OHLC, Volume через табуляцию
 */
std::ostream& operator<<( std::ostream &out, const TSimpleBar &aBar );

/**
 *  \brief Вывод тика в поток: DateToStr, Price, Volume через табуляцию
 */
std::ostream& operator<<( std::ostream &out, const TSimpleTick &aTick );

/**
 *  \brief Округление цены к ближайшему кратному шага
 *  \param aPrice Округляемая цена
 *  \param aPriceStep Шаг сетки (> 0, иначе assert)
 *  \return round(aPrice/aPriceStep)*aPriceStep
 */
TPrice RoundTo( const TPrice aPrice, const TPrice aPriceStep );

/**
 *  \brief Округление цены вниз к кратному шага
 *  \param aPrice Округляемая цена
 *  \param aPriceStep Шаг сетки (> 0, иначе assert)
 *  \return trunc(aPrice/aPriceStep)*aPriceStep
 */
TPrice TruncTo( const TPrice aPrice, const TPrice aPriceStep );

/**
 *  \brief Округление цены вверх к кратному шага
 *  \param aPrice Округляемая цена
 *  \param aPriceStep Шаг сетки (> 0, иначе assert)
 *  \return ceil(aPrice/aPriceStep)*aPriceStep
 */
TPrice CeilTo( const TPrice aPrice, const TPrice aPriceStep );

/**
 *  \brief Тип указателя на функцию скользящего среднего
 *  \deprecated нигде не используется
 */
using TfunMA = std::function< TPriceSeries ( const TPriceSeries & aPrices, const int aPeriod ) >;

#endif //BACKTESTER_BASISOFSTRATEGY_H
