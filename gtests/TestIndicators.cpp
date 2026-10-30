/**
 * \file TestIndicators.cpp
 * \brief gtest для индикаторов Indicators.h и утилит BasisOfStrategy/BarPeriod
 *
 * Детерминированные захардкоженные данные; эталонные значения посчитаны
 * вручную по рекуррентностям из документации функций.
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Comparers.h"
#include "Prices.h"
#include "BasisOfStrategy.h"
#include "BarPeriod.h"
#include "Indicators.h"

//------------------------------------------------------------------------------------------
namespace {

/// Момент i-го бара/тика: gStartingTime + 60*i секунд (кратно минуте —
/// удобно для ресемплинга _CreateBars).
TInnerDate TimeOf( const size_t aIndex ) {
    return gStartingTime + 60.0 * static_cast< double >( aIndex );
}

TSimpleTick Tick( const size_t aIndex, const TPrice aPrice, const double aVolume = 1.0 ) {
    return TSimpleTick( TimeOf( aIndex ), aPrice, aVolume );
}

TPriceSeries SeriesOf( const std::vector< TPrice > &aPrices ) {
    TPriceSeries lResult;
    lResult.reserve( aPrices.size() );
    for( size_t i = 0; i < aPrices.size(); ++i ) {
        lResult.push_back( Tick( i, aPrices[i] ) );
    }
    return lResult;
}

void ExpectBadTick( const TSimpleTick &aTick ) {
    EXPECT_TRUE( IsEqual( aTick.Price, GetBadPrice() ) ) << "price = " << aTick.Price;
    EXPECT_TRUE( isZero( aTick.Volume ) ) << "volume = " << aTick.Volume;
}

void ExpectGoodTick( const TSimpleTick &aTick, const TPrice aPrice,
                     const double aVolume = 1.0 ) {
    EXPECT_TRUE( IsEqual( aTick.Price, aPrice ) ) << "price = " << aTick.Price;
    EXPECT_TRUE( IsEqual( aTick.Volume, aVolume ) ) << "volume = " << aTick.Volume;
}

/// Все точки серии bad (и длины совпадают)
void ExpectAllBad( const TPriceSeries &aSeries, const size_t aExpectedSize ) {
    ASSERT_EQ( aSeries.size(), aExpectedSize );
    for( size_t i = 0; i < aSeries.size(); ++i ) {
        ExpectBadTick( aSeries[i] );
    }
}

/// DateTime результата повторяет DateTime входа
void ExpectDateTimes( const TPriceSeries &aResult, const size_t aCount ) {
    ASSERT_EQ( aResult.size(), aCount );
    for( size_t i = 0; i < aCount; ++i ) {
        EXPECT_TRUE( IsEqual( aResult[i].DateTime, TimeOf( i ) ) );
    }
}

} // namespace

//------------------------------------------------------------------------------------------
// _SimpleMA
//------------------------------------------------------------------------------------------

TEST( Indicators, SimpleMA_Reference ) {
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 };
    const TPriceSeries lResult = _SimpleMA( SeriesOf( lPrices ), 3 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectDateTimes( lResult, lPrices.size() );

    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 20.0 );  // (10+20+30)/3
    ExpectGoodTick( lResult[3], 30.0 );  // (20+30+40)/3
    ExpectGoodTick( lResult[4], 40.0 );
    ExpectGoodTick( lResult[5], 50.0 );
}

TEST( Indicators, SimpleMA_WindowSlidesCorrectly ) {
    // Немонотонная серия: окно 4, проверяем скользящую сумму по индексам 3..6
    const std::vector< TPrice > lPrices{ 1.0, 2.0, 4.0, 8.0, 16.0, 3.0, 5.0, 7.0 };
    const TPriceSeries lResult = _SimpleMA( SeriesOf( lPrices ), 4 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectBadTick( lResult[2] );
    ExpectGoodTick( lResult[3], ( 1.0 + 2.0 + 4.0 + 8.0 ) / 4.0 );
    ExpectGoodTick( lResult[4], ( 2.0 + 4.0 + 8.0 + 16.0 ) / 4.0 );
    ExpectGoodTick( lResult[5], ( 4.0 + 8.0 + 16.0 + 3.0 ) / 4.0 );
    ExpectGoodTick( lResult[6], ( 8.0 + 16.0 + 3.0 + 5.0 ) / 4.0 );
    ExpectGoodTick( lResult[7], ( 16.0 + 3.0 + 5.0 + 7.0 ) / 4.0 );
}

TEST( Indicators, SimpleMA_ThrowsOnNonPositivePeriod ) {
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0 } );
    EXPECT_THROW( _SimpleMA( lSeries, 0 ), std::logic_error );
    EXPECT_THROW( _SimpleMA( lSeries, -3 ), std::logic_error );
}

TEST( Indicators, SimpleMA_AllBadWhenInputNotLongerThanPeriod ) {
    const std::vector< TPrice > lPrices{ 1.0, 2.0, 3.0 };

    // size == period -> все bad
    ExpectAllBad( _SimpleMA( SeriesOf( lPrices ), 3 ), 3 );
    // size < period -> все bad
    ExpectAllBad( _SimpleMA( SeriesOf( lPrices ), 10 ), 3 );
    // пустой вход -> пустая серия
    EXPECT_TRUE( _SimpleMA( TPriceSeries(), 3 ).empty() );
}

//------------------------------------------------------------------------------------------
// _ExponentMA
//------------------------------------------------------------------------------------------

TEST( Indicators, ExponentMA_ReferenceRecurrence ) {
    // period=3 -> w = 2/(3+1) = 0.5; seed = среднее первых 3 точек
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 };
    const TPriceSeries lResult = _ExponentMA( SeriesOf( lPrices ), 3 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 20.0 );  // seed = (10+20+30)/3
    ExpectGoodTick( lResult[3], 40.0 * 0.5 + 20.0 * 0.5 );  // = 30
    ExpectGoodTick( lResult[4], 50.0 * 0.5 + 30.0 * 0.5 );  // = 40
    ExpectGoodTick( lResult[5], 60.0 * 0.5 + 40.0 * 0.5 );  // = 50
}

TEST( Indicators, ExponentMA_LagRegressionNotLongerThanPeriodAndLag ) {
    // Регрессия №1: при lag>0 старый guard lResultSize > lPeriod допускал
    // OOB read/write при lPeriod < size < lPeriod+aLag (seed-цикл писал
    // lResult[i] до индекса lPeriod+aLag-1).
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0 };
    // size(4) == period(3)+lag(2)-1: старый код давал OOB (seed-цикл до i=4)
    ExpectAllBad( _ExponentMA( SeriesOf( lPrices ), 3, 2 ), 4 );
    // граничный случай size == period+lag: все bad
    const std::vector< TPrice > lPrices5{ 10.0, 20.0, 30.0, 40.0, 50.0 };
    ExpectAllBad( _ExponentMA( SeriesOf( lPrices5 ), 3, 2 ), 5 );
}

TEST( Indicators, ExponentMA_LagSeedIsWindowMean ) {
    // period=3, lag=2, size=8: точки 0..3 bad, точка 4 — seed =
    // среднее точек [2,5); далее рекуррентность с w=0.5
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0 };
    const TPriceSeries lResult = _ExponentMA( SeriesOf( lPrices ), 3, 2 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectDateTimes( lResult, lPrices.size() );

    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectBadTick( lResult[2] );
    ExpectBadTick( lResult[3] );
    ExpectGoodTick( lResult[4], ( 30.0 + 40.0 + 50.0 ) / 3.0 );  // seed = 40
    ExpectGoodTick( lResult[5], 60.0 * 0.5 + 40.0 * 0.5 );       // = 50
    ExpectGoodTick( lResult[6], 70.0 * 0.5 + 50.0 * 0.5 );       // = 60
    ExpectGoodTick( lResult[7], 80.0 * 0.5 + 60.0 * 0.5 );       // = 70
}

TEST( Indicators, ExponentMA_ThrowsOnNonPositivePeriod ) {
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0 } );
    EXPECT_THROW( _ExponentMA( lSeries, 0 ), std::logic_error );
    EXPECT_THROW( _ExponentMA( lSeries, -1 ), std::logic_error );
}

//------------------------------------------------------------------------------------------
// _SmoothedMA
//------------------------------------------------------------------------------------------

TEST( Indicators, SmoothedMA_ReferenceRecurrence ) {
    // v[i] = (price[i] + v[i-1]*(period-1)) / period
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 };
    const TPriceSeries lResult = _SmoothedMA( SeriesOf( lPrices ), 3 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 20.0 );                        // seed
    ExpectGoodTick( lResult[3], ( 40.0 + 20.0 * 2.0 ) / 3.0 ); // 26.666...
    EXPECT_NEAR( lResult[4].Price, 310.0 / 9.0, 1e-9 );        // (50 + 160/3)/3
    EXPECT_NEAR( lResult[5].Price, 1160.0 / 27.0, 1e-9 );      // (60 + 620/9)/3
    EXPECT_TRUE( IsEqual( lResult[4].Volume, 1.0 ) );
    EXPECT_TRUE( IsEqual( lResult[5].Volume, 1.0 ) );
}

TEST( Indicators, SmoothedMA_WithLagSeedAndRecurrence ) {
    // period=2, lag=1, size=5: точка 0 bad, точка 2 = seed = (p1+p2)/2,
    // далее v[i] = (p[i] + v[i-1]) / 2. Путь _AverageTrueRange (lag=1).
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0 };
    const TPriceSeries lResult = _SmoothedMA( SeriesOf( lPrices ), 2, 1 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 25.0 );                 // seed = (20+30)/2
    ExpectGoodTick( lResult[3], ( 40.0 + 25.0 ) / 2.0 );  // 32.5
    ExpectGoodTick( lResult[4], ( 50.0 + 32.5 ) / 2.0 );  // 41.25
}

TEST( Indicators, SmoothedMA_ThrowsOnNonPositivePeriod ) {
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0 } );
    EXPECT_THROW( _SmoothedMA( lSeries, 0 ), std::logic_error );
    EXPECT_THROW( _SmoothedMA( lSeries, -5 ), std::logic_error );
}

TEST( Indicators, SmoothedMA_AllBadWhenInputNotLongerThanPeriodAndLag ) {
    const std::vector< TPrice > lPrices{ 1.0, 2.0, 3.0 };
    ExpectAllBad( _SmoothedMA( SeriesOf( lPrices ), 3 ), 3 );
    ExpectAllBad( _SmoothedMA( SeriesOf( lPrices ), 2, 2 ), 3 );
}

//------------------------------------------------------------------------------------------
// _KAMA
//------------------------------------------------------------------------------------------

TEST( Indicators, KAMA_FlatSeriesMatchesPrice ) {
    const std::vector< TPrice > lPrices( 8, 10.0 );
    const TPriceSeries lResult = _KAMA( SeriesOf( lPrices ), 3 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    for( size_t i = 0; i < 3; ++i ) {
        ExpectBadTick( lResult[i] );
    }
    for( size_t i = 3; i < lResult.size(); ++i ) {
        ExpectGoodTick( lResult[i], 10.0 );
    }
}

TEST( Indicators, KAMA_TrendSeriesMovesAndLags ) {
    const std::vector< TPrice > lPrices{ 10.0, 10.0, 10.0, 10.0, 20.0, 20.0, 20.0, 20.0 };
    const TPriceSeries lResult = _KAMA( SeriesOf( lPrices ), 3 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    // Точка 3 = c*p3 + (1-c)*p2 при плоском seed = 10
    ExpectGoodTick( lResult[3], 10.0 );
    // На ступеньке вверх KAMA растёт, но отстаёт от цены (ER=1 -> c=(2/3)^2)
    EXPECT_TRUE( IsGreat( lResult[4].Price, 10.0 ) );
    EXPECT_TRUE( IsLess( lResult[4].Price, 20.0 ) );
    EXPECT_NEAR( lResult[4].Price, 14.444444, 1e-5 );

    for( size_t i = 5; i < lResult.size(); ++i ) {
        EXPECT_TRUE( IsGreat( lResult[i].Price, lResult[i - 1].Price ) ) << "i = " << i;
        EXPECT_TRUE( IsLess( lResult[i].Price, 20.0 ) );
    }
}

TEST( Indicators, KAMA_ThrowsOnNonPositivePeriod ) {
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0, 4.0 } );
    EXPECT_THROW( _KAMA( lSeries, 0 ), std::logic_error );
    EXPECT_THROW( _KAMA( lSeries, 3, 2.0, 0.0, 30.0 ), std::logic_error );
    EXPECT_THROW( _KAMA( lSeries, 3, 2.0, 2.0, -30.0 ), std::logic_error );
}

TEST( Indicators, KAMA_AllBadWhenInputNotLongerThanPeriod ) {
    const std::vector< TPrice > lPrices{ 1.0, 2.0, 3.0 };
    ExpectAllBad( _KAMA( SeriesOf( lPrices ), 3 ), 3 );
}
