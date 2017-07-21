// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/** 
 * File:   BarPeriod.cpp
 * Author: kan
 * 
 * Created on 2015.11.11
 * @lastupdate 2016.03.25
 */

#include <map>
#include <list>
#include <vector>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <cassert>

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
    for( TDeal lDeal : lDeals ) {
        TSimpleTick lTick {
            lDeal.CloseTime,
            lDeal.DealSide == TDealSide::Buy ? lDeal.ClosePrice - lDeal.OpenPrice : lDeal.OpenPrice - lDeal.ClosePrice,
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
TPrice PnLsToMoneyResult( const TPriceSeries & aPnl ) {
    TPrice lResult = 0;
    for( size_t i = 0; i < aPnl.size(); ++i ) {
        lResult += aPnl[ i ].Price;
    }

    return lResult;
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

    for( auto &lDeal : aDeals ) {
        TInnerDate lEmptyDate = lDeal.OpenTime;
        TSimpleTick lEmptyTick = { lEmptyDate, 0.0, 0 };
        lPnLs.push_back( lEmptyTick );

        TInnerDate lBeginDate = lDeal.CloseTime;

        const TPrice lDealPnl =
            lDeal.DealSide == TDealSide::Buy ?
                lDeal.ClosePrice - lDeal.OpenPrice :
                lDeal.OpenPrice - lDeal.ClosePrice;
        
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
    for( auto ldPnl : lQuantPnL ) {
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
    for( auto lTick : lPnLs ) {

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

    const TPrice lResult = isZero( std::abs( aoPnl ) + aoMaxDD ) ? gMaxBedAttraction : aoPnl / ( std::abs( aoPnl ) + aoMaxDD ) ;
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
TPriceSeries DealsToDaily( const TPriceSeries & aPnls ) {
    
    if( aPnls.empty() ) {
        return TPriceSeries();
    }
    TInnerDate lMinDate = gMaxInteger;
    TInnerDate lMaxDate = 0;
    
    for( auto &lDeal : aPnls ) {
        const TInnerDate lDealDate = Trunc( lDeal.DateTime / gOneDay );
        lMinDate = IsLess( lMinDate, lDealDate ) ? lMinDate : lDealDate;
        lMaxDate = IsGreat( lMaxDate, lDealDate ) ? lMaxDate : lDealDate;
    }
    
    TPriceSeries lResult( ToSize_t( lMaxDate-lMinDate+1 ) );
    for( auto &lDeal : aPnls ) {
        const size_t lDealDate = ToSize_t( Trunc( lDeal.DateTime / gOneDay ) - lMinDate );
        const TPrice lDealPnl = lDeal.Price;
        
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
