/**
 * \file PnlAction.h
 * \brief Модуль хранящий в себе типы периодов для баров и основные методы работы с ними
 * \author kan <kansoftware.ru>
 * \since 2015-11-11
 * \date 2021-11-08
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */

#ifndef BACKTESTER_PNLACTION_H
#define BACKTESTER_PNLACTION_H

#include <list>
#include <vector>
#include <map>
#include <string>

#include "BasisOfStrategy.h"

const double gMaxBedAttraction = -99.9;
const double gFiltredByCount = -33.3;
const double gFiltredByCountPnl = -66.6;
const double gFiltredByPnl = 0.0;

const size_t gMinDealsForAllTime = 6UL;
const size_t gMinDealsStatictics = 30UL;
const size_t gWellDealsStatictics = 1000UL;

typedef std::map<std::string, double> TValues;

struct TDeal {
    TInnerDate OpenTime = 0.0;
    TPrice OpenPrice = GetBadPrice();
    TPrice StopLoss = GetBadPrice();
    TPrice TakeProfit = GetBadPrice();
    TPrice ClosePrice = GetBadPrice();
    TInnerDate CloseTime = 0.0;
    TDealSide DealSide = TDealSide::None;
    size_t Volume = 0;
    TValues Additive;

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

TDoubles ToDoublesArray( const TPriceSeries & aPriceSeries );

bool CalcDrawDown(
    const TPriceSeries & aPnL,
    TPrice & aoMaxDD,
    TInnerDate & aoBegin,
    TInnerDate & aoReturn );

TPrice PnLsToMoneyResult( const TPriceSeries & aPnl, const bool aUseVolume = false );
TPrice PnLsToMoneyStatValue( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const double aQuantile=0.2 );
TPrice PnLsToMoneyStatValueGost( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics  );
TPrice PnLsToMoneyMonteCarlo( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const size_t aSamples=gWellDealsStatictics );
TPrice PnLsToMoneyMonteCarloQuantile( const TPriceSeries & aPnl, const bool aUseVolume = false, const size_t N=gMinDealsStatictics, const size_t aSamples=gWellDealsStatictics, const double aQuantile=0.05 );

TPriceSeries PnLsAmplifier( const TPriceSeries &aPnl, const std::vector<double> &aAmplifiers, const TInnerDate aBegin=-1.0, const TInnerDate aEnd=-1.0 );

TPriceSeries ReductionOfTheIncome(
    const TPriceSeries & aPnL,
    const size_t aProfitNum,
    const double aProfitCoef,
    const size_t aLossNum,
    const double aLossCoef );

TPriceSeries DealsToPnLs( const TDeals & aDeals );
TPrice DealsToPNLCoefficientQuick( const TDeals & aDeals, const TPrice aFirstPrice, const size_t aMinDeals = gMinDealsForAllTime );
TPrice DealsToCoeff( const TBarSeries & aBars, const TDeals & aDeals, const size_t aMinDeals, TPrice & aoPnl, TPrice & aoMaxDD, size_t & aoMaxPos, size_t & aoMeadPos );

TPrice DealsToPNLCoefficient(
    const TDeals & aDeals,
    const TInnerDate aFirstPoint,
    const TInnerDate aLastPoint,
    const TPrice aMinPnl = 0.0,
    const size_t aMinDeals = gMinDealsForAllTime,
    const size_t aQuantTime = 86400 * 7 );

TPriceSeries PnlsToDaily( const TPriceSeries & aPnls, const bool aCumSum = true );
bool IsGrows( const TPriceSeries & aDailyPnls, const size_t aPeriod, const size_t aTollerance = 1 );

#endif //BACKTESTER_PNLACTION_H
