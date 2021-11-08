// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/** 
 * File:   PnlAction.cpp
 * Author: kan
 * 
 * Created on 2015.11.11
 * @lastupdate 2018.05.18
 */

#include <map>
#include <list>
#include <vector>
#include <set>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <climits>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <boost/math/distributions/students_t.hpp>

#include "DelphisRound.h"
#include "Comparers.h"

//#define FULLDATA
#ifdef FULLDATA
    #include <iostream>
    #include "Indicators.h"
#endif

#include "PnlAction.h"

//------------------------------------------------------------------------------------------
TPriceSeries DealsToPnLs( const TDeals & aDeals ) {
    
    //<editor-fold desc="Удалить все пустые сделки">
    TDeals lDeals( aDeals );
    auto it = lDeals.begin();
    while( it != lDeals.end() ) {
        if( isZero( it->ClosePrice - it->OpenPrice ) ) {
            it = lDeals.erase( it );
        } else {
            ++it;
        }
    }
    //</editor-fold>
    
    TPriceSeries lPnLs; //( lDeals.size() );
    lPnLs.reserve(lDeals.size());

    //<editor-fold desc="Собственно конвертация">
    for( const TDeal& lDeal : lDeals ) {
        lPnLs.emplace_back(
            lDeal.CloseTime,
            ( (lDeal.DealSide == TDealSide::Buy) ? (lDeal.ClosePrice - lDeal.OpenPrice) : (lDeal.OpenPrice - lDeal.ClosePrice) ),
            ToDouble( lDeal.Volume )
        );
    }
    //</editor-fold>

    return lPnLs;
}

//------------------------------------------------------------------------------------------
bool CalcDrawDown(
    const TPriceSeries & aPnL,
    TPrice & aoMaxDD,
    TInnerDate & aoBegin,
    TInnerDate & aoReturn ) {

    aoBegin = -1;
    aoReturn = -1;

    if( aPnL.empty() ) {
        aoMaxDD = -1;
        return false;
    }

    TPrice lMax = 0.0;
    TPrice lMin = 0.0;
    TPrice lPnl = 0.0;
    TPrice lDelta = 0.0;
    size_t lBegin = 0;
    bool lHaveMin = false;
    bool lResult = false;

    for( size_t i = 0; i < aPnL.size() ; ++i ) {
        lPnl += aPnL[i].Price;

        if( IsGreat( lPnl, lMax ) ) { //очередной максимум
            lMax = lPnl;
            lMin = lPnl;

            if( lHaveMin ) {
                    aoBegin = aPnL[ lBegin ].DateTime;
                    aoReturn = aPnL[ i ].DateTime;
                    lHaveMin = false;
                    lResult = true;
            }

            lBegin = i;

        } else if( IsGreat( lMin, lPnl ) ) { //новый минимум
            lMin = lPnl;

            if( IsGreat( lMax - lMin, lDelta ) ) {
                lDelta = lMax - lMin;
                lHaveMin = true;
            }
        }
    }

    aoMaxDD = lDelta;

    return lResult;
}

//------------------------------------------------------------------------------------------
TPriceSeries ReductionOfTheIncome(
    const TPriceSeries & aPnL,
    const size_t aProfitNum,
    const double aProfitCoef,
    const size_t aLossNum,
    const double aLossCoef ) {

    std::multimap< TPrice, size_t > lProfits;
    for( size_t i = 0; i < aPnL.size(); ++i ) {

        TPrice lValue = aPnL[ i ].Price ;
        //lProfits.insert( std::pair< TPrice, size_t >( lValue, i ) ) ;
        lProfits.emplace( lValue, i ) ;
    }

    TPriceSeries lPnLVector( aPnL );
    size_t i = aProfitNum;
    for( auto it = lProfits.rbegin(); it != lProfits.rend(); ++it ) {
        if( it->first > 0 ) {
            const size_t lRealID = it->second ;
            TSimpleTick lTick = aPnL[ lRealID ] ;
            lTick.Price  /= aProfitCoef ;
            lPnLVector[ lRealID ] = lTick;
        }
        if( --i == 0 ) break;
    }

    i = aLossNum;
    for( auto it = lProfits.begin(); it != lProfits.end() ;++it ) {
        if( it->first < 0 ) {
            const size_t lRealID = it->second ;
            TSimpleTick lTick = aPnL[ lRealID ] ;
            lTick.Price  *= aLossCoef ;
            lPnLVector[ lRealID ] = lTick;
        }
        if( --i == 0 ) break;
    }

    return lPnLVector;
}

//------------------------------------------------------------------------------------------
TPrice PnLsToMoneyResult( const TPriceSeries & aPnls, const bool aUseVolume ) {
    TPrice lResult = 0.0;
    for( const TSimpleTick& lPnl : aPnls ) {
        lResult += lPnl.Price * ( aUseVolume ? lPnl.Volume : 1.0 );
    }
    
    return lResult;
}

//------------------------------------------------------------------------------------------
double Student_t_value( const size_t Sn, const double tail=0.2 ){//80%
    using boost::math::students_t;
    students_t dist( ToDouble(Sn) - 1 );
    const double T = quantile(complement(dist, tail / 2.0));
    
    return T;
}

//------------------------------------------------------------------------------------------
TPrice PnLsToMoneyStatValue( const TPriceSeries & aPnl, const bool aUseVolume, const size_t N, const double aQuantile ) {
    const size_t lSize = aPnl.size();
    if( lSize < 2 or N < 2 ) {
        return 0.0;
    }
    
    std::vector< TPrice > lPnl( lSize );
    TPrice sum = 0.0;
    for( size_t i=0; i<lSize; ++i ){
        const TPrice lValue = aPnl[i].Price * ( aUseVolume ? aPnl[i].Volume : 1.0 );
        sum += lValue;
        lPnl[i] = lValue;
    }
    
    const TPrice mean = sum / ToDouble(lSize);
    
    TPrice lstdev = 0.0;
    for( size_t i=0; i<lSize; ++i ){
        lstdev += pow(lPnl[i] - mean, 2);
    }
    
    lstdev /= ToDouble(lSize);
    
    const double lStudAN = Student_t_value( lSize, aQuantile ) / sqrt( ToDouble(N) );

    return (mean - sqrt(lstdev)*lStudAN)*ToDouble(N);
}

//------------------------------------------------------------------------------------------
TPrice PnLsToMoneyStatValueGost( const TPriceSeries & aPnl, const bool aUseVolume, const size_t N ) {
    const size_t lSize = aPnl.size();
    if( lSize < 2 ) {
        return 0.0;
    }
    
    std::vector< TPrice > lPnl( lSize );
    TPrice sum = 0.0;
    for( size_t i=0; i<lSize; ++i ){
        const TPrice lValue = aPnl[i].Price * ( aUseVolume ? aPnl[i].Volume : 1.0 );
        sum += lValue;
        lPnl[i] = lValue;
    }
    
    const TPrice mean = sum / ToDouble(lSize);
    
    TPrice lstdev = 0.0;
    for( size_t i=0; i<lSize; ++i ){
        lstdev += pow(lPnl[i] - mean, 2);
    }
    
    lstdev /= (ToDouble(lSize) - 1.5);//ГОСТ Р 8.736-2011
    
    return ( mean - sqrt( lstdev ) / sqrt( ToDouble(lSize) ) )*ToDouble(N);
}

//------------------------------------------------------------------------------------------
TPrice PnLsToMoneyMonteCarlo( const TPriceSeries & aPnl, const bool aUseVolume, const size_t N, const size_t aSamples ){
    const size_t lSize = aPnl.size();
    if( lSize < 2 or N < 2 ) {
        return 0.0;
    }
    
    TPrice lResult = ToDouble( ULLONG_MAX ); //очень большое число
    
    for( size_t i=0; i<aSamples; ++i ){
        TPrice lPnlTest = 0.0;
        for( size_t j=0; j<N; ++j ){
            const size_t lID = rand() % lSize;
            const TPrice lPnlSample = aPnl[lID].Price * (aUseVolume ? aPnl[lID].Volume : 1.0);
            lPnlTest += lPnlSample;
        }
        lPnlTest /= ToDouble(N);
        
        lResult = std::min(lResult, lPnlTest);
    }
    
    return lResult * ToDouble(N);
}

//------------------------------------------------------------------------------------------
TPrice PnLsToMoneyMonteCarloQuantile( const TPriceSeries & aPnl, const bool aUseVolume, const size_t N, const size_t aSamples, const double aQuantile ) {
    const size_t lSize = aPnl.size();
    if( lSize < 2 or N < 2 or IsGreat( aQuantile, 1.0 ) ) {
        return 0.0;
    }
    
    std::multiset<TPrice> lData;
    
    for( size_t i=0; i<aSamples; ++i ){
        TPrice lPnlTest = 0.0;
        for( size_t j=0; j<N; ++j ){
            const size_t lID = rand() % lSize;
            const TPrice lPnlSample = aPnl[lID].Price * (aUseVolume ? aPnl[lID].Volume : 1.0);
            lPnlTest += lPnlSample;
        }
        lPnlTest /= ToDouble(N);
        
        lData.insert( lPnlTest );
    }
    
    const size_t lID = RoundToSize_t( aQuantile * ToDouble( aSamples ) );
    
    auto iter = lData.cbegin();
    std::advance(iter, lID);
    
    const TPrice lResult = *iter;
    
    return lResult * ToDouble(N);    
}

//------------------------------------------------------------------------------------------
TPrice DealsToPNLCoefficientQuick(
    const TDeals & aDeals,
    const TPrice aFirstPrice,
    const size_t aMinDeals ) {
    
    TPriceSeries lPnL( DealsToPnLs( aDeals ) );
    if( lPnL.size() <= aMinDeals ) {
        return gMaxBedAttraction;
    }

    TPrice lMaxDD = -1;
    TInnerDate lBegin;
    TInnerDate lReturn;

    bool lDDOk = CalcDrawDown( lPnL, lMaxDD, lBegin, lReturn );
    bool lGoodResult = lDDOk or ( isZero( lMaxDD ) );

    if( not lGoodResult ) { return gMaxBedAttraction; }

    const size_t lDealsToReduction = std::max( std::min( aMinDeals / 2, 3UL ), 1UL );
    const double lReductionCoeff = 2.0;
    const TPriceSeries lPnLPes( ReductionOfTheIncome( lPnL, lDealsToReduction, lReductionCoeff, lDealsToReduction, lReductionCoeff ) );
    const TPrice lPnlMoneyReduction = PnLsToMoneyResult( lPnLPes );
    
    const TPrice lPnlMoney = PnLsToMoneyResult( lPnL );
    const TPrice lResult =  lPnlMoneyReduction / ( lMaxDD + std::abs( lPnlMoney ) );
//    TPrice lResult =  lPnlMoney / ( lMaxDD + std::abs( lPnlMoney ) );
//    TPrice lResult =  lPnlMoney / ( lMaxDD + 1 );
    
    return lResult ;
}

//------------------------------------------------------------------------------------------
TPrice DealsToPNLCoefficient(
    const TDeals & aDeals,
    const TInnerDate aFirstPoint,
    const TInnerDate aLastPoint,
    const TPrice aMinPnl,
    const size_t aMinDeals,
    const size_t aQuantTime ) {

    if( aDeals.size() < aMinDeals ) {
        return gMaxBedAttraction;
    }

    //region перевести список сделок в список d_pnl
    TPrice lResult = gMaxBedAttraction ;
    TPrice lPnLValue = 0.0;
    //TInnerDate lPnLTime = 0.0;
    std::list< TSimpleTick > lPnLs;
    lPnLs.push_back( {aFirstPoint, 0.0, 0} );

    for( const auto &lDeal : aDeals ) {
        TInnerDate lEmptyDate = lDeal.OpenTime;
        TSimpleTick lEmptyTick = { lEmptyDate, 0.0, 0 };
        lPnLs.push_back( lEmptyTick );

        TInnerDate lBeginDate = lDeal.CloseTime;

        const TPrice lDealPnl =
            (lDeal.DealSide == TDealSide::Buy) ?
                (lDeal.ClosePrice - lDeal.OpenPrice) :
                (lDeal.OpenPrice - lDeal.ClosePrice);
        
        TSimpleTick lRealTick = { lBeginDate, lDealPnl, 1 };
        lPnLs.push_back( lRealTick );

        lPnLValue += lDealPnl;
        //lPnLTime += ( lDeal.CloseTime - lDeal.OpenTime );
    }

    lPnLs.push_back( {aLastPoint, 0.0, 0} );
    //endregion

    // рассчитать общий pnl, учесть aMinPnl
    if( lPnLValue <= aMinPnl ) {
        return gMaxBedAttraction;
    }

    //region  рассчитать ср. и макс. d_pnl, число сделок
    const size_t lWeekCouner = CeilToSize_t( ( aLastPoint - aFirstPoint ) / static_cast< double >( aQuantTime ) );
    std::vector< TPrice > lQuantPnL( lWeekCouner );
    size_t lIdx=0;
    TInnerDate lNextDate = aFirstPoint + static_cast<TInnerDate>( aQuantTime );

    std::list< TSimpleTick > lPnLsToQuanting( lPnLs );
    while( not lPnLsToQuanting.empty() ) {
        auto lTick = lPnLsToQuanting.front();
        lPnLsToQuanting.pop_front();

        if( lTick.DateTime > lNextDate ) {
            lNextDate += static_cast<TInnerDate>( aQuantTime );
            lIdx++;
        }
        lQuantPnL[ lIdx ] += lTick.Price;
    }

    TPrice lMaxPnl = 0.0; //если lPnLValue>0 значит точно есть положительный pnl за интервал
    TPrice lMidPnl = 0.0;
    size_t lDealCounter = 0 ;
    for( const auto ldPnl : lQuantPnL ) {
        if( ldPnl > lMaxPnl ){ lMaxPnl = ldPnl; }

        lMidPnl += ldPnl ;
        lDealCounter++ ;
    }
    //endregion

    // ... рассчитать разброс pnl,
    TPrice lPnLValatility = ( lMidPnl / ToDouble( lDealCounter ) ) / lMaxPnl;

    //region расчитать d_DD, учесть DD==0
    TInnerDate lDD_begin ;
    TPrice lCurrDD = 0.0;
    TPrice lSumCurrDD = 0.0;
    TInnerDate lPrioreDate = aFirstPoint ;
    for( const auto& lTick : lPnLs ) {

        if( isZero( lCurrDD ) and lTick.Price < 0.0 ) {

            lCurrDD = lTick.Price;
            lDD_begin = lPrioreDate ;

            lSumCurrDD += lCurrDD * ( lTick.DateTime - lDD_begin ) ;// 86400.0;

        } else if( lCurrDD < 0.0 ) {
            TPrice lPriorDD = lCurrDD;
            lCurrDD += lTick.Price;

            if( lCurrDD > 0.0 ) {
                lCurrDD = 0.0;

            } else {
                lPriorDD = lCurrDD;
            }

            lSumCurrDD += lPriorDD * ( lTick.DateTime - lPrioreDate ) ;// 86400.0;
        }

        lPrioreDate = lTick.DateTime ;
    }
    //endregion

    if( isPositiveValue( lSumCurrDD ) ) {
        return gMaxBedAttraction;
    }

    // ... рассчитать ср.DD
    const TPrice lTW_DD = -lSumCurrDD / ( aLastPoint - aFirstPoint );

    // Расчитать коэффициент            Это среднеинтервальный заработок
    lResult = lPnLValatility / ( lTW_DD + 1.0 ) * lMidPnl /  ToDouble( lDealCounter ) ; //lPnLValue / lPnLTime ;

    return lResult ;
}

//------------------------------------------------------------------------------------------
TPrice DealsToCoeff(
    const TBarSeries & aBars,
    const TDeals & aDeals,
    const size_t aMinDeals,
    TPrice & aoPnl,
    TPrice & aoMaxDD,
    size_t & aoMaxPos,
    size_t & aoMeadPos ) {
    #ifdef FULLDATA
        std::cout << "aDeals.size() = " << aDeals.size() << std::endl;
    #endif
    
    if( aDeals.size() <= aMinDeals ) {
        aoPnl = GetBadPrice(); 
        aoMaxDD = GetBadPrice();
        aoMaxPos = 0; 
        aoMeadPos = 0;
        return gMaxBedAttraction;
    }
    
    TPriceSeries lBalance( aBars.size() );
    
    for( size_t i=0; i<aBars.size(); ++i ){
        lBalance[i]={aBars[i].DateTime,0.0,0.0};
    }
    
    auto lTickCompare = []( const TSimpleTick& aLeft, const TSimpleTick& aRigth )->bool {
        return aLeft.DateTime < aRigth.DateTime;
    };
    
    #ifdef FULLDATA
        std::cout << "\nDeals" << std::endl;
    #endif
    
    for( auto it=aDeals.begin(); it!=aDeals.end(); ++it ) {
        
        const TInnerDate lOpenTime = it->OpenTime;
        const TInnerDate lCloseTime = it->CloseTime;
        const TPrice lOpenPrice = it->OpenPrice;
        const TPrice lClosePrice = it->ClosePrice;
        
        auto itBegin = std::lower_bound ( 
            lBalance.begin(), 
            lBalance.end(),
            TSimpleTick{ lOpenTime,0.0,0.0 },
            lTickCompare 
        );
        assert( itBegin != lBalance.end() );
        
        auto itEnd = std::upper_bound( 
            itBegin, 
            lBalance.end(),
            TSimpleTick{ lCloseTime,0.0,0.0 },
            lTickCompare 
        );
        
        TPrice lBalanceVolume = 0.0;
            
        if( it->DealSide == TDealSide::Buy ) {
            for( auto itBalance=itBegin; itBalance != std::prev( itEnd ) ; ++itBalance ){
                itBalance->Volume += 1.0;
                itBalance->Price -= lOpenPrice;
            }
            lBalanceVolume = lClosePrice - lOpenPrice ;
        
        } else {
            for( auto itBalance=itBegin; itBalance != std::prev( itEnd ) ; ++itBalance ) {
                itBalance->Volume -= 1.0;
                itBalance->Price += lOpenPrice;
            }
            lBalanceVolume = lOpenPrice - lClosePrice;
        }
        
        for( auto itBalance=std::prev( itEnd ); itBalance != lBalance.end(); ++itBalance ) {
            itBalance->Price += lBalanceVolume;
        }
    }
    
    if( not isZero( lBalance.rbegin()->Volume ) ) { ///\todo вообще-то нужно учесть сторону для каждой конкретной сделки
        const TPrice lClosePrice = aBars.rbegin()->Close;
        const TPrice lBalancedSide = ( aDeals.rbegin()->DealSide == TDealSide::Buy ) ? 1.0 : -1.0;
        lBalance.rbegin()->Price += lBalancedSide * lBalance.rbegin()->Volume * lClosePrice;
        lBalance.rbegin()->Volume = 0;
    }
    
    TPrice lMax = 0.0;
    TPrice lMin = 0.0;
    aoPnl = 0.0;
    aoMaxDD = 0.0;
    
    #ifdef FULLDATA
    std::cout << "\nDD" << std::endl;
    #endif
    
    std::vector< size_t > lPosition( aBars.size(), 0 );
    
    for( size_t i=0; i<aBars.size(); ++i ) {
        lPosition[ i ] = ToSize_t( std::fabs( lBalance[i].Volume ) );
        aoPnl = lBalance[i].Price + aBars[i].Close * lBalance[i].Volume;
        
        if( IsGreat( aoPnl, lMax ) ) { //очередной максимум
            lMax = aoPnl;
            lMin = aoPnl;

        } else if( IsGreat( lMin, aoPnl ) ) { //новый минимум
            lMin = aoPnl;
            const TPrice lCurrentDD = lMax - lMin;
            if( IsGreat( lCurrentDD, aoMaxDD ) ) {
                aoMaxDD = lCurrentDD;
            }
        }
        #ifdef FULLDATA
        std::cout << RoundToSize_t( aBars[i].DateTime ) << " " << aBars[i].Close << " " << lBalance[i].Volume << " " << aoPnl << " " << lMax << " " << lMin << " " << aoMaxDD << std::endl;
        #endif
    }
    
    std::sort( lPosition.begin(), lPosition.end() );
    aoMaxPos = *lPosition.rbegin();
    aoMeadPos = lPosition[ lPosition.size() / 2 ];

    const TPrice lResult = isZero( std::abs( aoPnl ) + aoMaxDD ) ? gMaxBedAttraction : (aoPnl / ( std::abs( aoPnl ) + aoMaxDD )) ;
    #ifdef FULLDATA
    std::cout << "lResult = " << lResult << std::endl;
    #endif
    
    return lResult;
}

//------------------------------------------------------------------------------------------
TDoubles ToDoublesArray( const TPriceSeries & aPriceSeries ) {
    const size_t lArraySize = aPriceSeries.size();
    TDoubles lResult( lArraySize );
    
    for( size_t i=0; i<lArraySize; ++i ) {
        lResult[i]=aPriceSeries[i].Price;
    }
    
    return lResult;
}

//------------------------------------------------------------------------------------------
TPriceSeries PnlsToDaily( const TPriceSeries & aPnls, const bool aCumSum ) {
    
    if( aPnls.empty() ) {
        return TPriceSeries();
    }
    TInnerDate lMinDate = gMaxInteger;
    TInnerDate lMaxDate = 0;
    
    for( const auto &lDeal : aPnls ) {
        const TInnerDate lDealDate = Trunc( lDeal.DateTime / gOneDay );
        lMinDate = IsLess( lMinDate, lDealDate ) ? lMinDate : lDealDate;
        lMaxDate = IsGreat( lMaxDate, lDealDate ) ? lMaxDate : lDealDate;
    }
    
    TPriceSeries lResult( ToSize_t( lMaxDate-lMinDate+1 ) );
    for( const auto &lDeal : aPnls ) {
        const size_t lDealDate = ToSize_t( Trunc( lDeal.DateTime / gOneDay ) - lMinDate );
        const TPrice lDealPnl = lDeal.Price * ((lDeal.Volume == 0) ? 1.0 : lDeal.Volume );
        
        TSimpleTick lDayDeal = lResult[ lDealDate ];
        lDayDeal.DateTime = ToDouble( Trunc( lDeal.DateTime / gOneDay ) * gOneDay );
        lDayDeal.Price += lDealPnl;
        lDayDeal.Volume += 1.0;
        
        lResult[ lDealDate ] = lDayDeal;
    }
    
    if(aCumSum){
        for( size_t i = 1; i < lResult.size(); ++i ){
            lResult[i].Price += lResult[i-1].Price;
        }
    }
    
    return lResult;
}

//------------------------------------------------------------------------------------------
bool IsGrows( const TPriceSeries & aDailyPnls, const size_t aPeriod, const size_t aTollerance ) {
    if( aPeriod == 0 ){
        throw std::logic_error( "aPeriod can be positive" );
    }
    
    if( aDailyPnls.size() <= aPeriod ) {
        return false;
    }
    
    size_t lTollerance = ( aTollerance == 0 ) ? 1 : aTollerance;
    
    for( size_t i=0; i < aDailyPnls.size() - aPeriod; ++i ) {
        if( IsGreat( aDailyPnls[i].Price, aDailyPnls[ i+aPeriod ].Price ) ) {
            if( --lTollerance == 0 ) {
                return false;
            }
        }
    }
    
    return true;
}

//------------------------------------------------------------------------------------------
TPriceSeries PnLsAmplifier( const TPriceSeries &aPnl, const std::vector<double> &aAmplifiers, const TInnerDate aBegin, const TInnerDate aEnd ) {
    
    if( aAmplifiers.size() < 2UL ){
        return aPnl;
    }
    
    //sort deals
    TPriceSeries lPnls( aPnl );
    std::sort( lPnls.begin(), lPnls.end(),
        []( const TSimpleTick& lh, const TSimpleTick& rh ){
            return IsLess( lh.DateTime, rh.DateTime );
        }
    );
    
    //days range detect
    const TInnerDate lMinDate =  trunc( (isPositiveValue( aBegin ) ? aBegin : lPnls.begin()->DateTime) / gOneDay );
    const TInnerDate lMaxDate =  trunc( (isPositiveValue( aEnd ) ? aEnd : lPnls.rbegin()->DateTime) / gOneDay );
    
    if( CeilToSize_t(lMaxDate-lMinDate) < aAmplifiers.size() ){
        return aPnl;
    }
    
    const double lMinDelta = trunc( (lMaxDate-lMinDate) / ToDouble( aAmplifiers.size() ) );
    if( IsLess( lMinDelta, 1.0 ) ){
        return aPnl;
    }
    
    std::vector<TInnerDate> ldates( aAmplifiers.size() );
    for( size_t i=0; i<aAmplifiers.size(); ++i ) {
        ldates[i] = lMinDate + lMinDelta * ToDouble(i+1) ;
    }
    
    TPriceSeries lPnls_result;
    lPnls_result.reserve( ToSize_t(*aAmplifiers.rbegin()) * aPnl.size() );
    for( auto it = lPnls.begin(); it != lPnls.end(); ++it ){
        const TSimpleTick lTick( *it );
        
        for( size_t i=0; i < ldates.size(); ++i ){
            if( not IsGreat( trunc(lTick.DateTime / gOneDay), ldates[i] ) ){
                for( size_t j=0; j < ToSize_t(aAmplifiers[i]); ++j ){
                    lPnls_result.push_back( lTick );
                }
                break;
            }
        }
    }
    
    lPnls_result.shrink_to_fit();
    return lPnls_result;
}

//------------------------------------------------------------------------------------------
