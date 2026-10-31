/**
 * \file PnlAction.h
 * \brief Модуль работы со сделками и PnL-сериями: деньги, статистика, просадка, коэффициенты качества
 * \author kan <kansoftware.ru>
 * \since 2015-11-11
 * \date 2021-11-08
 * \note Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */

#ifndef BACKTESTER_PNLACTION_H
#define BACKTESTER_PNLACTION_H

#include <list>
#include <vector>
#include <map>
#include <string>

#include "BasisOfStrategy.h"

const double gMaxBedAttraction = -99.9;     /*!< Сентинел «плохой/отфильтрованный результат» для коэффициентов качества (ниже любого реального) */
const double gFiltredByCount = -33.3;       /*!< Сентинел внешней конвенции: отфильтровано по числу сделок */
const double gFiltredByCountPnl = -66.6;    /*!< Сентинел внешней конвенции: отфильтровано по числу сделок и PnL */
const double gFiltredByPnl = 0.0;           /*!< Сентинел внешней конвенции: отфильтровано по PnL */

const size_t gMinDealsForAllTime = 6UL;     /*!< Минимум сделок для расчёта коэффициентов по всей истории */
const size_t gMinDealsStatictics = 30UL;    /*!< Горизонт N для статистических оценок (по умолчанию) */
const size_t gWellDealsStatictics = 1000UL; /*!< Число сэмплов Монте-Карло (по умолчанию) */

typedef std::map<std::string, double> TValues;  /*!< Именованные дополнительные значения сделки */

/**
 *  \struct TDeal
 *  \brief Сделка: время/цены открытия и закрытия, сторона, объём
 */
struct TDeal {
    TInnerDate OpenTime = 0.0;              /*!< Время открытия (unix, секунды) */
    TPrice OpenPrice = GetBadPrice();       /*!< Цена открытия */
    TPrice StopLoss = GetBadPrice();        /*!< StopLoss (если был) */
    TPrice TakeProfit = GetBadPrice();      /*!< TakeProfit (если был) */
    TPrice ClosePrice = GetBadPrice();      /*!< Цена закрытия */
    TInnerDate CloseTime = 0.0;             /*!< Время закрытия (unix, секунды) */
    TDealSide DealSide = TDealSide::None;   /*!< Сторона: Buy/Sell (None — сделка не в позиции) */
    size_t Volume = 0;                      /*!< Объём сделки */
    TValues Additive;                       /*!< Дополнительные именованные значения */

    TDeal(){;}

    TDeal(
        const TInnerDate aOpenTime,
        const TPrice aOpenPrice,
        const TPrice aStopLoss,
        const TPrice aTakeProfit,
        const TPrice aClosePrice,
        const TInnerDate aCloseTime,
        const TDealSide aDealSide,
        const size_t aVolume = 1,
        const TValues aAdditive={} ) :
        OpenTime( aOpenTime ),
        OpenPrice( aOpenPrice ),
        StopLoss( aStopLoss ),
        TakeProfit( aTakeProfit ),
        ClosePrice( aClosePrice ),
        CloseTime( aCloseTime ),
        DealSide( aDealSide ),
        Volume( aVolume ),
        Additive( aAdditive ){;}

    void Reset() {
        OpenTime = 0.0;
        ResetPrice( OpenPrice );
        ResetPrice( ClosePrice );
        CloseTime = 0.0;
        DealSide = TDealSide::None;
        ResetPrice( StopLoss );
        ResetPrice( TakeProfit );
        Volume = 0;
        Additive.clear();
    }

    bool InAction() const { return DealSide == TDealSide::Buy or DealSide == TDealSide::Sell; }
};

typedef std::list< TDeal > TDeals;
typedef std::vector< double > TDoubles;
typedef std::vector< int > TIntegers;
typedef std::vector< bool > TBools;

TDoubles ToDoublesArray( const TPriceSeries & aPriceSeries );   /*!< \brief Цены серии в массив double */

/**
 *  \brief Максимальная просадка кумулятивной кривой aPnL (peak-to-trough)
 *  \param aPnL Серия PnL-точек (DateTime/Price)
 *  \param aoMaxDD Величина максимальной просадки (>= 0); для пустой серии -1
 *  \param aoBegin Дата пика, с которого началась максимальная просадка (-1, если не определена)
 *  \param aoReturn Дата возврата кумалятива выше пика (-1, если возврата не было)
 *  \return true, если после максимальной просадки кумулятив вернулся к новому максимуму
 */
bool CalcDrawDown(
    const TPriceSeries & aPnL,
    TPrice & aoMaxDD,
    TInnerDate & aoBegin,
    TInnerDate & aoReturn );

TPrice PnLsToMoneyResult( const TPriceSeries & aPnl, const bool aUseVolume = false );   /*!< \brief Суммарный PnL: сумма Price (× Volume при aUseVolume) */

/**
 *  \brief Статистическая оценка ожидаемого PnL на горизонт N сделок
 *
 *  (mean - tStudent_квантиль * sd/sqrt(N)) * N, где mean/sd — по всей серии;
 *  aQuantile — хвост распределения Стьюдента (0.2 -> 80% интервал).
 *  \return 0.0 при aPnl.size() < 2 или N < 2
 */
TPrice PnLsToMoneyStatValue( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const double aQuantile=0.2 );

/**
 *  \brief Оценка как PnLsToMoneyStatValue, но по ГОСТ Р 8.736-2011
 *
 *  sd с делителем (n - 1.5) и фиксированный t = 2.04 (N > 30, P = 95%).
 *  \return 0.0 при aPnl.size() < 2
 */
TPrice PnLsToMoneyStatValueGost( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics  );

/**
 *  \brief Худший (минимальный) из aSamples бутстрэп-сэмплов среднего по N сделок, × N
 *
 *  Воспроизводимая псевдослучайность: rand() без srand — детерминированная
 *  последовательность при каждом запуске процесса.
 *  \return 0.0 при aPnl.size() < 2 или N < 2
 */
TPrice PnLsToMoneyMonteCarlo( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const size_t aSamples=gWellDealsStatictics );

/**
 *  \brief Квантиль aQuantile бутстрэп-распределения среднего по N сделок, × N
 *
 *  aQuantile=0.05 — нижний 5% квантиль (пессимистичная оценка); aQuantile=1.0 —
 *  максимум по выборкам, 0.0 — минимум. Индекс квантиля клампится в [0, aSamples-1].
 *  Воспроизводимая псевдослучайность: rand() без srand.
 *  \return 0.0 при aPnl.size() < 2, N < 2, aSamples == 0 или aQuantile > 1.0
 */
TPrice PnLsToMoneyMonteCarloQuantile( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const size_t aSamples=gWellDealsStatictics, const double aQuantile=0.05 );

TPriceSeries PnLsAmplifier( const TPriceSeries &aPnl, const std::vector<double> &aAmplifiers, const TInnerDate aBegin=-1.0, const TInnerDate aEnd=-1.0 );   /*!< \brief Растяжение серии: точки зоны i дублируются ToSize_t(aAmplifiers[i]) раз; aBegin/aEnd<=0 — границы по серии */

/**
 *  \brief Ухудшение лучших прибылей и худших убытков (защита от переоптимизации)
 *  \param aProfitNum Сколько лучших прибыльных точек делить на aProfitCoef
 *  \param aProfitCoef Делитель лучших прибылей (> 0)
 *  \param aLossNum Сколько худших убыточных точек умножать на aLossCoef
 *  \param aLossCoef Множитель худших убытков (> 0)
 *  \return Копия серии с изменёнными Price (DateTime без изменений)
 */
TPriceSeries ReductionOfTheIncome(
    const TPriceSeries & aPnL,
    const size_t aProfitNum,
    const double aProfitCoef,
    const size_t aLossNum,
    const double aLossCoef );

TPriceSeries DealsToPnLs( const TDeals & aDeals );  /*!< \brief Сделки -> PnL-серия по моментам CloseTime (Buy: Close-Open, Sell: Open-Close); нулевые сделки (ClosePrice==OpenPrice) отбрасываются */
TPrice DealsToPNLCoefficientQuick( const TDeals & aDeals, const TPrice aFirstPrice, const size_t aMinDeals = gMinDealsForAllTime );  /*!< \brief Коэффициент PnL/(maxDD+|PnL|) с ухудшением крайних сделок; aFirstPrice не используется; сделок <= aMinDeals -> gMaxBedAttraction */

/**
 *  \brief Коэффициент качества стратегии по балансу, разложенному на сетку баров
 *
 *  Каждая сделка переносит позицию (Volume ±1) на бары от открытия (включительно,
 *  lower_bound по OpenTime) до закрытия (не включительно, upper_bound по CloseTime);
 *  реализованный PnL сделки добавляется к Price баланса с бара закрытия до конца
 *  ряда. Незаполненная позиция на последнем баре оценивается по Close последнего
 *  бара. Сделки позже последнего бара пропускаются; пустой aBars или
 *  aDeals.size() <= aMinDeals -> bad-значения в ao* и gMaxBedAttraction.
 *  \param aoPnl Итоговый PnL последнего бара (баланс + Close*позиция)
 *  \param aoMaxDD Максимальная просадка кривой aoPnl по барам
 *  \param aoMaxPos Максимум |позиции| по барам
 *  \param aoMeadPos Медиана |позиции| по барам
 *  \return aoPnl / (|aoPnl| + aoMaxDD); при нулевом знаменателе gMaxBedAttraction
 */
TPrice DealsToCoeff( const TBarSeries & aBars, const TDeals & aDeals, const size_t aMinDeals, TPrice & aoPnl, TPrice & aoMaxDD, size_t & aoMaxPos, size_t & aoMeadPos );

/**
 *  \brief Коэффициент качества по квантованной серии PnL сделок (устаревший вариант)
 *
 *  Сделки -> серия dPnL (в моменты Open — 0, в моменты Close — pnl сделки),
 *  квантование по окнам aQuantTime; коэффициент = (разброс/максимум окна) /
 *  (time-weighted DD + 1) * среднее по окнам.
 *  \param aFirstPoint/aLastPoint Границы расчётного интервала (unix, секунды)
 *  \param aMinPnl Минимальный суммарный PnL, иначе gMaxBedAttraction
 *  \param aMinDeals Минимум сделок (aDeals.size() < aMinDeals -> gMaxBedAttraction)
 *  \param aQuantTime Длина окна квантования, секунды
 *  \return Коэффициент или gMaxBedAttraction при фильтрации
 */
TPrice DealsToPNLCoefficient(
    const TDeals & aDeals,
    const TInnerDate aFirstPoint,
    const TInnerDate aLastPoint,
    const TPrice aMinPnl = 0.0,
    const size_t aMinDeals = gMinDealsForAllTime,
    const size_t aQuantTime = 86400 * 7 );

TPriceSeries PnlsToDaily( const TPriceSeries & aPnls, const bool aCumSum = true );  /*!< \brief Суммирование PnL в суточные (UTC) корзины; Volume результата — число сделок в дне; aCumSum — накопительный итог */

/**
 *  \brief Растёт ли серия на горизонте aPeriod шагов
 *  \param aDailyPnls Суточные PnL (обычно PnlsToDaily с aCumSum)
 *  \param aPeriod Горизонт сравнения в шагах (> 0, иначе std::logic_error)
 *  \param aTollerance Допустимое число падений (0 трактуется как 1)
 *  \return true, если нарушений (aPnls[i] > aPnls[i+aPeriod]) не больше aTollerance;
 *          false при aDailyPnls.size() <= aPeriod
 */
bool IsGrows( const TPriceSeries & aDailyPnls, const size_t aPeriod, const size_t aTollerance = 1 );

#endif //BACKTESTER_PNLACTION_H
