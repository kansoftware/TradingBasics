/**
 * \file BasisOfStrategy.h
 * \brief Модуль реализующий объекты для одно-ногих стратегий
 * \author kan <kansoftware.ru>
 * \since 2015-11-11
 * \date 2017-07-21
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */


#ifndef BACKTESTER_BASISOFSTRATEGY_H
#define BACKTESTER_BASISOFSTRATEGY_H

#include <vector>
#include <map>
#include <string>

#include "Prices.h"
#include "BarPeriod.h"

enum class TMAPoint {
    MAP_Open = 0,
    MAP_High = 1,
    MAP_Low = 2,
    MAP_Close = 3,
    MAP_Volume = 4,
    MAP_Mid = 5,
    MAP_Triple = 6
};

enum class TDealSide { None=0, Buy=1, Sell=2, Any };

typedef double TInnerDate;

const double gOneDay = 86400.0;
const double gOneHour = 3600.0;
const double gStartingTime = 975888000;

struct TSimpleTick {
    TInnerDate DateTime=gStartingTime;
    TPrice Price=0.0;
    double Volume=0.0;
};

typedef std::vector< TSimpleTick > TPriceSeries;

typedef std::map< std::string, TPriceSeries > TIndicators;

struct TSimpleBar {
    TInnerDate DateTime=gStartingTime;
    TPrice Open=0.0;
    TPrice High=0.0;
    TPrice Low=0.0;
    TPrice Close=0.0;
    double Volume=-1.0;
};

void Reset( TSimpleBar &aBar );
TSimpleBar operator+( const TSimpleBar &aStartBar, const TSimpleBar &aFinishBar );
bool IsValidBar( const TSimpleBar & aBar );

typedef std::vector< TSimpleBar > TBarSeries;
const TSimpleBar gEmptyBar{gStartingTime,0.0,0.0,0.0,0.0,-1.0};

bool IsOneDay( const TInnerDate aLeft, const TInnerDate aRight );
bool IsOneHour( const TInnerDate aLeft, const TInnerDate aRight, const double aDuration = 1.0 );
inline void ResetPrice( TPrice & aPrice ) { aPrice = GetBadPrice(); }

TPriceSeries BarsToPriceSeries( const TBarSeries & aBar, const TMAPoint aType );
TBarSeries _CreateBars( const TBarSeries & aBars, const TBarPeriod aBarPeriod );

std::string DateToStr( const TInnerDate aDate );

std::ostream& operator<<( std::ostream &out, const TSimpleBar &aBar );
std::ostream& operator<<( std::ostream &out, const TSimpleTick &aTick );

TPrice RoundTo( const TPrice aPrice, const TPrice aPriceStep );
TPrice TruncTo( const TPrice aPrice, const TPrice aPriceStep );
TPrice CeilTo( const TPrice aPrice, const TPrice aPriceStep );

#endif //BACKTESTER_BASISOFSTRATEGY_H
