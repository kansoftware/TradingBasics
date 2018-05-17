// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/** 
 * File:   PnlAction.cpp
 * Author: kan
 * 
 * Created on 2015.11.11
 * @lastupdate 2018.05.15
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
    
    TPriceSeries lPnLs( lDeals.size() );

    //<editor-fold desc="Собственно конвертация">
    size_t i = 0;
    for( const TDeal& lDeal : lDeals ) {
        TSimpleTick lTick {
            lDeal.CloseTime,
            ((lDeal.DealSide == TDealSide::Buy) ? (lDeal.ClosePrice - lDeal.OpenPrice) : (lDeal.OpenPrice - lDeal.ClosePrice)),
            ToDouble( lDeal.Volume )
        };

        lPnLs[ i++ ] = lTick;
    }
    //</editor-fold>
    
    assert( i == lPnLs.size() );
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
        lProfits.insert( std::pair< TPrice, size_t >( lValue, i ) ) ;
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
TPrice PnLsToMoneyResult( const TPriceSeries & aPnl, const bool aUseVolume ) {
    TPrice lResult = 0;
    for( const TSimpleTick& lPnl : aPnl ) {
        lResult += lPnl.Price * ( aUseVolume ? lPnl.Volume : 1.0 );
    }
    
    return lResult;
}

//------------------------------------------------------------------------------------------
const double gStudentCV[100] = { //The table values are critical values of the t distribution for t = 0.975
    8.98485231514687,
    2.48410726821528,
    1.5912,
    1.24164382654608,
    1.04944305546641,
    0.924841269006278,
    0.836012347396855,
    0.768666666666667,
    0.715338829506689,
    0.671827577988173,
    0.635373971243183,
    0.604290393767765,
    0.577391187027602,
    0.553784978729711,
    0.53285,
    0.514151271514522,
    0.497284628982459,
    0.481979515288663,
    0.468009027690706,
    0.45520251903228,
    0.443372209733127,
    0.432438043312967,
    0.422251206826773,
    0.41278,
    0.403920791930604,
    0.395581159439759,
    0.387753752860166,
    0.380378330721873,
    0.373400724869855,//n=30
    0.366807908329282,
    0.360536070057491,
    0.354578777416161,
    0.348913871472434,
    0.343504495291858,
    0.33835,
    0.333417586153995,
    0.328693043495151,
    0.324163434563018,
    0.319816951161129,
    0.315642790153062,
    0.311631045583441,
    0.307757364788229,
    0.304028964304988,
    0.300438093456872,
    0.296962843891578,
    0.293611641386112,
    0.290363884132193,
    0.287228571428571,
    0.284200357494497,
    0.281260257677866,
    0.278404028485442,
    0.275627707619439,
    0.27294119872779,
    0.270340660850511,
    0.267809127458345,
    0.26533032176727,
    0.262941131801594,
    0.260599143110315,
    0.258327989192035,
    0.256112170929772,
    0.253949453949581,
    0.251850327181815,
    0.2497875,
    0.247784189288862,
    0.245826016634825,
    0.243923512601799,
    0.24205055378626,
    0.240229969832652,
    0.238448107562212,
    0.236691733910182,
    0.23498336850131,
    0.233321526935486,
    0.231681544016432,
    0.230074082272066,
    0.228509541721773,
    0.226975280134661,
    0.225470325809213,
    0.223993750195362,
    0.222533485120779,
    0.221122222222222,
    0.219725607431122,
    0.218354042397491,
    0.217006780945183,
    0.21569395420909,
    0.214403902960397,
    0.213125250800405,
    0.211878871914206,
    0.210653378693664,
    0.209448190358486,
    0.208262748511543,
    0.207096516041487,
    0.20594897609041,
    0.204819631082071,
    0.203708001806507,
    0.202613626557215,
    0.201546213779023,
    0.200484975517277,
    0.199449754438191,
    0.19842,
    0.197415378537662
};
TPrice PnLsToMoneyStatValue( const TPriceSeries & aPnl, const bool aUseVolume, const size_t N ) {
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
    
    const double lStudAN = ( N<102 ) ?
        ( gStudentCV[N-2] ) :
        ( 1.970 //t.975 n is very big
          /
          sqrt( ToDouble(N) )
        );
    
    return (mean - sqrt(lstdev)*lStudAN)*ToDouble(lSize);
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
    
    return ( mean - sqrt( lstdev ) / sqrt( ToDouble(N) ) )*ToDouble(lSize);
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
    
    return lResult * ToDouble(lSize);
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
    
    return lResult * ToDouble(lSize);    
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
        #ifdef FULLDATA
        std::cout << *it << std::endl;
        #endif
        
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
TPriceSeries PnlsToDaily( const TPriceSeries & aPnls ) {
    
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
    
    for( size_t i = 1; i < lResult.size(); ++i ){
        lResult[i].Price += lResult[i-1].Price;
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
