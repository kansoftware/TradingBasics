#include <gtest/gtest.h>

#include "BasisOfStrategy.h"
#include "PnlAction.h"
#include "DelphisRound.h"
#include "VolatilityBuffer.h"
#include "VolatilityBarBuffer.h"
#include "RubberBuffer.h"

//------------------------------------------------------------------------------------------
TPriceSeries GetSimplePnl(){
    TPriceSeries lPnLs( 10 );

    for( size_t i=0; i<lPnLs.size(); ++i ) {
        TSimpleTick lTick {
            gStartingTime + ToDouble(i)*gOneDay,
            1.0,
            2.0
        };

        lPnLs[ i ] = lTick;
    }
    
    return lPnLs;
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsToMoneyResult ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    
    EXPECT_EQ( PnLsToMoneyResult( lPnLs, false ), 10.0 );
    EXPECT_EQ( PnLsToMoneyResult( lPnLs, true ), 20.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnlsToDaily ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    const TPriceSeries lPnls_Daily( PnlsToDaily(lPnLs) );
    
    EXPECT_EQ( lPnLs.size(), lPnls_Daily.size() );
    
    double lCumPnl = 0.0;
    for( size_t i = 0; i<lPnLs.size(); ++i ){
        lCumPnl += lPnLs[i].Price*lPnLs[i].Volume;
        EXPECT_DOUBLE_EQ( lCumPnl, lPnls_Daily[i].Price );
        EXPECT_DOUBLE_EQ( lPnls_Daily[i].Volume, 1.0 );
    }
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsAmplifier ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    
    const TPriceSeries lPnLs_amp( PnLsAmplifier( lPnLs, {1,2,4} ) );
    EXPECT_EQ( lPnLs_amp.size(), 22 );
    EXPECT_GE( lPnLs_amp.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp2( PnLsAmplifier( lPnLs, {1,2,4}, gStartingTime, gStartingTime + gOneDay * 10.0 ) );
    EXPECT_EQ( lPnLs_amp2.size(), 22 );
    EXPECT_GE( lPnLs_amp2.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp2.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp2.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp3( PnLsAmplifier( lPnLs, {1,2,4}, gStartingTime - gOneDay * 3.0, gStartingTime + gOneDay * 14.0 ) );
    EXPECT_EQ( lPnLs_amp3.size(), 21 );
    EXPECT_GE( lPnLs_amp3.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp3.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp3.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp4( PnLsAmplifier( lPnLs, {1,1,1} ) );
    EXPECT_EQ( lPnLs_amp4.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp4.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp4.back().Price, lPnLs.back().Price );
    
//    for( const TSimpleTick& lTick : lPnLs_amp4 ){
//        std::cout << lTick << std::endl;
//    }
}

//------------------------------------------------------------------------------------------
TEST( ITime, main ) {
    EXPECT_EQ( ITime(""), 0.0 );
    EXPECT_EQ( ITime("00:00"), 0.0 );
    EXPECT_EQ( ITime("10:00"), 36000.0 );
    EXPECT_EQ( ITime("10:01:01"), 36061.0 );
    EXPECT_EQ( ITime(1592826299), 42299.0 ); //Mon, 22 Jun 2020 11:44:59 GMT
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBuffer, main ) {
    TVolatilityBuffer<double> lbuf(5);
    EXPECT_FALSE( lbuf.isFill() );
    EXPECT_TRUE( std::isnan(lbuf.getMax()) );
    for (size_t i = 0; i < 4; i++) {
        EXPECT_FALSE( lbuf.add( ToDouble(i) ) );
        EXPECT_TRUE( std::isnan(lbuf.getErr()) );
    }
    EXPECT_TRUE( lbuf.add( ToDouble(4) ) );
    EXPECT_NEAR( lbuf.getErr(), 0.7559289, 0.0000001 );
    EXPECT_EQ( lbuf.getMin(), 0 );
    EXPECT_EQ( lbuf.getMax(), 4 );
    EXPECT_EQ( lbuf.getMean(), 2 );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBarBuffer, main ) {
    TVolatilityBarBuffer lbuf(5);
    EXPECT_FALSE( lbuf.isFill() );
    EXPECT_TRUE( std::isnan(lbuf.getMax()) );
    for (size_t i = 0; i < 4; i++) {
        EXPECT_FALSE( lbuf.add( TSimpleBar{gStartingTime+ToDouble(i),ToDouble(i),ToDouble(i),ToDouble(i),ToDouble(i),1.0} ) );
        EXPECT_TRUE( std::isnan(lbuf.getErr()) );
    }
    EXPECT_TRUE( lbuf.add( TSimpleBar{gStartingTime+4.0,4.0,4.0,4.0,4.0,1.0} ) );
    //EXPECT_NEAR( lbuf.getErr(), 0.7559289, 0.0000001 );
    EXPECT_EQ( lbuf.getMin(), 0 );
    EXPECT_EQ( lbuf.getMax(), 4 );
    EXPECT_EQ( lbuf.getMean(), 0 );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBarRollBuffer, main ) {
    TVolatilityBarRollBuffer lbuf(5);
    EXPECT_FALSE( lbuf.isFill() );
    
    for (size_t i = 0; i < 4; i++) {
        EXPECT_FALSE( lbuf.add( TSimpleBar{gStartingTime+ToDouble(i),ToDouble(i)*2+1.0,ToDouble(i)*2+1.0,ToDouble(i),ToDouble(i),1.0} ) );
        EXPECT_TRUE( std::isnan(lbuf.getErr()) );
    }
    EXPECT_TRUE( lbuf.add( TSimpleBar{gStartingTime+4.0,4.0,5.0,4.0,4.0,1.0} ) );

    EXPECT_EQ( lbuf.getMin(), 1.0 );
    EXPECT_EQ( lbuf.getMax(), 4.0 );
    EXPECT_TRUE( lbuf.isFill() );

    EXPECT_EQ( lbuf.getMean(), 2.2 );
    EXPECT_NEAR( lbuf.getErr(), 0.623355, 0.000001 );
    //d <- c(1,2,3,4,1);sqrt(sum((d - mean(d))^2)/(length(d)-1.5))/sqrt(length(d))

    for (size_t i = 0; i < 4; i++) {
        EXPECT_TRUE( lbuf.add( TSimpleBar{gStartingTime+ToDouble(i),ToDouble(i)+1.0,ToDouble(i)+1.0,ToDouble(i),ToDouble(i),1.0} ) );
        EXPECT_TRUE( lbuf.isFill() );
    }

    EXPECT_EQ( lbuf.getMean(), 1.0 );
    EXPECT_NEAR( lbuf.getErr(), 0.0, 0.000001 );
}

//------------------------------------------------------------------------------------------
TEST( RubberBuffer, main ) {
    TRubberBuffer<double> lbuf(3);
    EXPECT_FALSE( lbuf.isFill() );
    EXPECT_FALSE( lbuf.calcStats() );
    for (size_t i = 1; i < 11; i++) {
        lbuf.add(ToDouble( i ));
    }

    EXPECT_TRUE( lbuf.isFill() );
    EXPECT_TRUE( std::isnan(lbuf.getSD()) );

    EXPECT_TRUE( lbuf.calcStats() );
    EXPECT_NEAR( lbuf.getSD(), 3.02765, 0.00001 );
    EXPECT_EQ( lbuf.getMean(), 5.5 );
    EXPECT_EQ( lbuf.getMin(), 1.0 );
    EXPECT_EQ( lbuf.getMax(), 10.0 );
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
TEST( PnLs, CalcDrawDown ) {
    // Кумулятив: 1, -2, 3 — просадка 3 между максимумами
    TPriceSeries lPnl;
    lPnl.push_back( { gStartingTime,                1.0, 1.0 } );
    lPnl.push_back( { gStartingTime + gOneDay,     -3.0, 1.0 } );
    lPnl.push_back( { gStartingTime + 2 * gOneDay,  5.0, 1.0 } );

    TPrice lMaxDD = 0.0;
    TInnerDate lBegin = 0.0;
    TInnerDate lReturn = 0.0;

    EXPECT_TRUE( CalcDrawDown( lPnl, lMaxDD, lBegin, lReturn ) );
    EXPECT_DOUBLE_EQ( lMaxDD, 3.0 );
    EXPECT_DOUBLE_EQ( lBegin, gStartingTime );
    EXPECT_DOUBLE_EQ( lReturn, gStartingTime + 2 * gOneDay );

    lMaxDD = 0.0;
    EXPECT_FALSE( CalcDrawDown( TPriceSeries(), lMaxDD, lBegin, lReturn ) );
    EXPECT_DOUBLE_EQ( lMaxDD, -1.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsToMoneyStatValueOnConstantSeries ) {
    // Нулевой разброс: значение стягивается к среднему * N
    const TPriceSeries lPnLs( GetSimplePnl() );

    EXPECT_DOUBLE_EQ( PnLsToMoneyStatValue( lPnLs ), 30.0 );        // mean=1 * N=30
    EXPECT_DOUBLE_EQ( PnLsToMoneyStatValue( lPnLs, true ), 60.0 );  // mean=2 * N=30
    EXPECT_DOUBLE_EQ( PnLsToMoneyStatValueGost( lPnLs ), 30.0 );
    EXPECT_DOUBLE_EQ( PnLsToMoneyStatValueGost( lPnLs, true ), 60.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsToMoneyMonteCarloQuantileRegression ) {
    // Регресс: quantile=1.0 давал std::advance за end(); aSamples==0 — разыменование cbegin()
    const TPriceSeries lPnLs( GetSimplePnl() );                    // константный ряд: любая выборка == 1.0

    EXPECT_DOUBLE_EQ( PnLsToMoneyMonteCarloQuantile( lPnLs, false, 30, 1000, 1.0 ), 30.0 );
    EXPECT_DOUBLE_EQ( PnLsToMoneyMonteCarloQuantile( lPnLs, false, 30, 1000, 0.0 ), 30.0 );
    EXPECT_DOUBLE_EQ( PnLsToMoneyMonteCarloQuantile( lPnLs, false, 30, 0, 0.05 ), 0.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsToMoneyMonteCarlo ) {
    const TPriceSeries lPnLs( GetSimplePnl() );

    // Константный ряд: минимум выборочных средних == среднему
    EXPECT_DOUBLE_EQ( PnLsToMoneyMonteCarlo( lPnLs, false, 30, 100 ), 30.0 );

    // Чередование 1/3: худшая выборка не выше среднего * N
    TPriceSeries lMixed;
    for( size_t i = 0; i < 10; ++i ) {
        lMixed.push_back( { gStartingTime + ToDouble(i) * gOneDay, ( i % 2 ) ? 3.0 : 1.0, 1.0 } );
    }
    EXPECT_LE( PnLsToMoneyMonteCarlo( lMixed, false, 30, 100 ), 60.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToPnLs ) {
    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime,                100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + gOneDay, TDealSide::Buy,  2 ) );
    lDeals.push_back( TDeal( gStartingTime + gOneDay,      100.0, GetBadPrice(), GetBadPrice(),  90.0, gStartingTime + 2 * gOneDay, TDealSide::Sell, 1 ) );
    lDeals.push_back( TDeal( gStartingTime + 2 * gOneDay,  100.0, GetBadPrice(), GetBadPrice(), 100.0, gStartingTime + 3 * gOneDay, TDealSide::Buy,  1 ) ); // нулевая — отбрасывается

    const TPriceSeries lPnLs( DealsToPnLs( lDeals ) );

    ASSERT_EQ( lPnLs.size(), 2UL );
    EXPECT_DOUBLE_EQ( lPnLs[0].Price, 10.0 );                      // Buy: close - open
    EXPECT_DOUBLE_EQ( lPnLs[0].Volume, 2.0 );
    EXPECT_DOUBLE_EQ( lPnLs[1].Price, 10.0 );                      // Sell: open - close
    EXPECT_DOUBLE_EQ( lPnLs[1].Volume, 1.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToPNLCoefficientQuickFewDeals ) {
    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime,           100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + gOneDay, TDealSide::Buy, 1 ) );
    lDeals.push_back( TDeal( gStartingTime + gOneDay, 100.0, GetBadPrice(), GetBadPrice(),  90.0, gStartingTime + 2 * gOneDay, TDealSide::Sell, 1 ) );

    // 2 сделки <= aMinDeals=5 -> плохой коэффициент
    EXPECT_DOUBLE_EQ( DealsToPNLCoefficientQuick( lDeals, 100.0, 5 ), gMaxBedAttraction );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToCoeffStandard ) {
    // 5 часовых баров Close=100; Buy 100->110 (T0..T1), Sell 100->90 (T1..T3)
    TBarSeries lBars;
    for( size_t i = 0; i < 5; ++i ) {
        lBars.push_back( TSimpleBar{ gStartingTime + ToDouble(i) * gOneHour, 100.0, 100.0, 100.0, 100.0, 1.0 } );
    }

    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime,                 100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime +     gOneHour, TDealSide::Buy,  1 ) );
    lDeals.push_back( TDeal( gStartingTime + gOneHour,      100.0, GetBadPrice(), GetBadPrice(),  90.0, gStartingTime + 3 * gOneHour, TDealSide::Sell, 1 ) );

    TPrice lPnl = 0.0;
    TPrice lMaxDD = 0.0;
    size_t lMaxPos = 0;
    size_t lMeadPos = 0;

    const TPrice lCoeff = DealsToCoeff( lBars, lDeals, 1, lPnl, lMaxDD, lMaxPos, lMeadPos );

    EXPECT_DOUBLE_EQ( lPnl, 20.0 );
    EXPECT_DOUBLE_EQ( lMaxDD, 0.0 );                               // кривая pnl монотонна
    EXPECT_EQ( lMaxPos, 1UL );
    EXPECT_EQ( lMeadPos, 1UL );
    EXPECT_DOUBLE_EQ( lCoeff, 1.0 );                               // 20 / (20 + 0)
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToCoeffWithDrawDown ) {
    // 6 баров; Buy 100->110, затем убыточный Sell 100->120
    TBarSeries lBars;
    for( size_t i = 0; i < 6; ++i ) {
        lBars.push_back( TSimpleBar{ gStartingTime + ToDouble(i) * gOneHour, 100.0, 100.0, 100.0, 100.0, 1.0 } );
    }

    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime,                 100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + 2 * gOneHour, TDealSide::Buy,  1 ) );
    lDeals.push_back( TDeal( gStartingTime + 2 * gOneHour,  100.0, GetBadPrice(), GetBadPrice(), 120.0, gStartingTime + 3 * gOneHour, TDealSide::Sell, 1 ) );

    TPrice lPnl = 0.0;
    TPrice lMaxDD = 0.0;
    size_t lMaxPos = 0;
    size_t lMeadPos = 0;

    const TPrice lCoeff = DealsToCoeff( lBars, lDeals, 1, lPnl, lMaxDD, lMaxPos, lMeadPos );

    EXPECT_DOUBLE_EQ( lPnl, -10.0 );
    EXPECT_DOUBLE_EQ( lMaxDD, 20.0 );
    EXPECT_EQ( lMaxPos, 1UL );
    EXPECT_EQ( lMeadPos, 1UL );
    EXPECT_NEAR( lCoeff, -1.0 / 3.0, 1e-9 );                       // -10 / (10 + 20)
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToCoeffEmptyBarsRegression ) {
    // Регресс: пустой aBars -> rbegin()/медиана по пустому балансу
    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime,            100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + gOneDay, TDealSide::Buy, 1 ) );
    lDeals.push_back( TDeal( gStartingTime + gOneDay,  100.0, GetBadPrice(), GetBadPrice(),  90.0, gStartingTime + 2 * gOneDay, TDealSide::Sell, 1 ) );

    TPrice lPnl = 0.0;
    TPrice lMaxDD = 0.0;
    size_t lMaxPos = 1;
    size_t lMeadPos = 1;

    EXPECT_DOUBLE_EQ( DealsToCoeff( TBarSeries(), lDeals, 1, lPnl, lMaxDD, lMaxPos, lMeadPos ), gMaxBedAttraction );
    EXPECT_EQ( lPnl, GetBadPrice() );
    EXPECT_EQ( lMaxDD, GetBadPrice() );
    EXPECT_EQ( lMaxPos, 0UL );
    EXPECT_EQ( lMeadPos, 0UL );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToCoeffDealBeyondBars ) {
    // Регресс: сделка позже последнего бара — не падает, сделка пропускается
    TBarSeries lBars;
    for( size_t i = 0; i < 5; ++i ) {
        lBars.push_back( TSimpleBar{ gStartingTime + ToDouble(i) * gOneHour, 100.0, 100.0, 100.0, 100.0, 1.0 } );
    }

    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime + 100 * gOneHour, 100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + 101 * gOneHour, TDealSide::Buy, 1 ) );

    TPrice lPnl = 0.0;
    TPrice lMaxDD = 0.0;
    size_t lMaxPos = 0;
    size_t lMeadPos = 0;

    EXPECT_DOUBLE_EQ( DealsToCoeff( lBars, lDeals, 0, lPnl, lMaxDD, lMaxPos, lMeadPos ), gMaxBedAttraction );
    EXPECT_DOUBLE_EQ( lPnl, 0.0 );
    EXPECT_DOUBLE_EQ( lMaxDD, 0.0 );
    EXPECT_EQ( lMaxPos, 0UL );
    EXPECT_EQ( lMeadPos, 0UL );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, DealsToCoeffCloseBeforeOpenBar ) {
    // Регресс: закрытие раньше первого бара открытия — itEnd клампится к itBegin+1
    TBarSeries lBars;
    for( size_t i = 0; i < 5; ++i ) {
        lBars.push_back( TSimpleBar{ gStartingTime + ToDouble(i) * gOneHour, 100.0, 100.0, 100.0, 100.0, 1.0 } );
    }

    TDeals lDeals;
    lDeals.push_back( TDeal( gStartingTime + 2 * gOneHour,  100.0, GetBadPrice(), GetBadPrice(), 110.0, gStartingTime + 0.5 * gOneHour, TDealSide::Buy, 1 ) );

    TPrice lPnl = 0.0;
    TPrice lMaxDD = 0.0;
    size_t lMaxPos = 0;
    size_t lMeadPos = 0;

    const TPrice lCoeff = DealsToCoeff( lBars, lDeals, 0, lPnl, lMaxDD, lMaxPos, lMeadPos );

    EXPECT_DOUBLE_EQ( lPnl, 10.0 );
    EXPECT_DOUBLE_EQ( lMaxDD, 0.0 );
    EXPECT_EQ( lMaxPos, 0UL );
    EXPECT_EQ( lMeadPos, 0UL );
    EXPECT_DOUBLE_EQ( lCoeff, 1.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnlsToDailyNoCumSum ) {
    // cumSum=false: в каждой дневной корзине только своя сумма
    const TPriceSeries lPnLs( GetSimplePnl() );

    const TPriceSeries lDaily( PnlsToDaily( lPnLs, false ) );

    ASSERT_EQ( lDaily.size(), lPnLs.size() );
    for( size_t i = 0; i < lDaily.size(); ++i ) {
        EXPECT_DOUBLE_EQ( lDaily[i].Price, 2.0 );                  // 1.0 * Volume=2
        EXPECT_DOUBLE_EQ( lDaily[i].Volume, 1.0 );
        EXPECT_DOUBLE_EQ( lDaily[i].DateTime, gStartingTime + ToDouble(i) * gOneDay );
    }
}

//------------------------------------------------------------------------------------------
TEST( PnLs, IsGrows ) {
    TPriceSeries lUp;
    TPriceSeries lDown;
    for( size_t i = 0; i < 5; ++i ) {
        lUp.push_back( { gStartingTime + ToDouble(i) * gOneDay, ToDouble(i) + 1.0, 1.0 } );
        lDown.push_back( { gStartingTime + ToDouble(i) * gOneDay, 5.0 - ToDouble(i), 1.0 } );
    }

    EXPECT_TRUE( IsGrows( lUp, 2 ) );
    EXPECT_FALSE( IsGrows( lDown, 2 ) );
    EXPECT_FALSE( IsGrows( lUp, 5 ) );                             // размер <= периода
    EXPECT_THROW( IsGrows( lUp, 0 ), std::logic_error );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, ReductionOfTheIncome ) {
    TPriceSeries lPnL;
    const double lPrices[5] = { 10.0, -5.0, 3.0, -1.0, 2.0 };
    for( size_t i = 0; i < 5; ++i ) {
        lPnL.push_back( { gStartingTime + ToDouble(i) * gOneDay, lPrices[i], 1.0 } );
    }

    const TPriceSeries lReduced( ReductionOfTheIncome( lPnL, 1, 2.0, 1, 2.0 ) );

    ASSERT_EQ( lReduced.size(), lPnL.size() );
    EXPECT_DOUBLE_EQ( lReduced[0].Price, 5.0 );                    // лучшая прибыль /2
    EXPECT_DOUBLE_EQ( lReduced[1].Price, -10.0 );                  // худший убыток *2
    EXPECT_DOUBLE_EQ( lReduced[2].Price, 3.0 );                    // остальные без изменений
    EXPECT_DOUBLE_EQ( lReduced[3].Price, -1.0 );
    EXPECT_DOUBLE_EQ( lReduced[4].Price, 2.0 );
}
//------------------------------------------------------------------------------------------
