/**
 * \file TestIndicators.cpp
 * \brief gtest для индикаторов Indicators.h и утилит BasisOfStrategy/BarPeriod
 *
 * Детерминированные захардкоженные данные; эталонные значения посчитаны
 * вручную по рекуррентностям из документации функций.
 */

#include <gtest/gtest.h>

#include <cmath>
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

TEST( Indicators, SimpleMA_WithLagWindowEndsAtCurrentBar ) {
    // Регрессия: при lag>0 окно выравнивается по конвенции _SmoothedMA —
    // bad в [0, period-1+lag), далее SMA = среднее [i-period+1 .. i]
    const std::vector< TPrice > lPrices{ 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0 };
    const TPriceSeries lResult = _SimpleMA( SeriesOf( lPrices ), 3, 2 );

    ASSERT_EQ( lResult.size(), lPrices.size() );
    ExpectDateTimes( lResult, lPrices.size() );

    // period-1+lag = 4 bad-точки
    for( size_t i = 0; i < 4; ++i ) {
        ExpectBadTick( lResult[i] );
    }
    ExpectGoodTick( lResult[4], ( 30.0 + 40.0 + 50.0 ) / 3.0 );  // окно [2..4]
    ExpectGoodTick( lResult[5], ( 40.0 + 50.0 + 60.0 ) / 3.0 );  // окно [3..5]
    ExpectGoodTick( lResult[6], ( 50.0 + 60.0 + 70.0 ) / 3.0 );  // окно [4..6]
    ExpectGoodTick( lResult[7], ( 60.0 + 70.0 + 80.0 ) / 3.0 );  // окно [5..7]

    // Граница guard'а: size == period+lag -> все bad (как у _SmoothedMA)
    ExpectAllBad( _SimpleMA( SeriesOf( lPrices ), 4, 4 ), 8 );
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

//------------------------------------------------------------------------------------------
// Бары для индикаторов от TBarSeries
//------------------------------------------------------------------------------------------

namespace {

TSimpleBar Bar( const size_t aIndex, const TPrice aOpen, const TPrice aHigh,
                const TPrice aLow, const TPrice aClose, const double aVolume = 1.0 ) {
    const TSimpleBar lBar{ TimeOf( aIndex ), aOpen, aHigh, aLow, aClose, aVolume };
    EXPECT_TRUE( IsValidBar( lBar ) );
    return lBar;
}

} // namespace

//------------------------------------------------------------------------------------------
// _TrueRange / _AverageTrueRange
//------------------------------------------------------------------------------------------

TEST( Indicators, TrueRange_Reference ) {
    const TBarSeries lBars{
        Bar( 0, 10.0, 12.0, 9.0, 11.0, 5.0 ),
        Bar( 1, 11.0, 13.0, 10.0, 12.0, 6.0 ),
        Bar( 2, 12.0, 12.5, 8.0, 8.5, 7.0 ),
        Bar( 3, 8.5, 15.0, 8.0, 14.0, 8.0 ),
        Bar( 4, 14.0, 14.5, 13.5, 14.0, 9.0 ) };

    const TPriceSeries lResult = _TrueRange( lBars );

    ASSERT_EQ( lResult.size(), lBars.size() );
    // Первая точка = High-Low первого бара; далее с учётом гэпа к prevClose
    ExpectGoodTick( lResult[0], 3.0, 5.0 );           // 12-9
    ExpectGoodTick( lResult[1], 3.0, 6.0 );           // max(13,11)-min(10,11)
    ExpectGoodTick( lResult[2], 4.5, 7.0 );           // max(12.5,12)-min(8,12)
    ExpectGoodTick( lResult[3], 7.0, 8.0 );           // max(15,8.5)-min(8,8.5)
    ExpectGoodTick( lResult[4], 1.0, 9.0 );           // max(14.5,14)-min(13.5,14)
}

TEST( Indicators, TrueRange_EmptyInputGivesEmptySeries ) {
    EXPECT_TRUE( _TrueRange( TBarSeries() ).empty() );
}

TEST( Indicators, AverageTrueRange_IsSmoothedTrueRangeWithLag ) {
    // period=2, lag=1: TR = [3, 3, 4.5, 7, 1] ->
    // seed = (TR[1]+TR[2])/2 = 3.75, далее v[i] = (TR[i]+v[i-1])/2
    const TBarSeries lBars{
        Bar( 0, 10.0, 12.0, 9.0, 11.0 ),
        Bar( 1, 11.0, 13.0, 10.0, 12.0 ),
        Bar( 2, 12.0, 12.5, 8.0, 8.5 ),
        Bar( 3, 8.5, 15.0, 8.0, 14.0 ),
        Bar( 4, 14.0, 14.5, 13.5, 14.0 ) };

    const TPriceSeries lResult = _AverageTrueRange( lBars, 2 );

    ASSERT_EQ( lResult.size(), lBars.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 3.75 );
    ExpectGoodTick( lResult[3], ( 7.0 + 3.75 ) / 2.0 );   // 5.375
    ExpectGoodTick( lResult[4], ( 1.0 + 5.375 ) / 2.0 );  // 3.1875
}

TEST( Indicators, AverageTrueRange_EmptyInputGivesEmptySeries ) {
    EXPECT_TRUE( _AverageTrueRange( TBarSeries(), 3 ).empty() );
}

//------------------------------------------------------------------------------------------
// _DI / _ADX (smoke)
//------------------------------------------------------------------------------------------

namespace {

/// 9 монотонно растущих баров: High/Low/Close шагают вверх на 1
TBarSeries RisingBars( const size_t aCount ) {
    TBarSeries lBars;
    for( size_t i = 0; i < aCount; ++i ) {
        const TPrice lBase = 10.0 + static_cast< TPrice >( i );
        lBars.push_back( Bar( i, lBase, lBase + 1.0, lBase - 1.0, lBase ) );
    }
    return lBars;
}

} // namespace

TEST( Indicators, DI_RisingTrendPositiveDIDominates ) {
    const TBarSeries lBars = RisingBars( 6 );
    TPriceSeries lDIp;
    TPriceSeries lDIn;

    ASSERT_TRUE( _DI( lBars, 3, lDIp, lDIn ) );
    ASSERT_EQ( lDIp.size(), lBars.size() );
    ASSERT_EQ( lDIn.size(), lBars.size() );

    // Первые period точек bad
    for( size_t i = 0; i < 3; ++i ) {
        ExpectBadTick( lDIp[i] );
        ExpectBadTick( lDIn[i] );
    }
    // Рост без откатов: +DM = 1 на каждом шаге, -DM = 0; сглаживание
    // sum = sum*(p-1)/p + DM, DI = 100*sumDM/sumTR
    EXPECT_NEAR( lDIp[3].Price, 100.0 * ( 7.0 / 3.0 ) / 6.0, 1e-6 );    // 38.888...
    EXPECT_TRUE( IsEqual( lDIn[3].Price, 0.0 ) );
    EXPECT_NEAR( lDIp[5].Price, 100.0 * ( 73.0 / 27.0 ) / 6.0, 1e-6 );  // 45.061...
    EXPECT_TRUE( IsEqual( lDIn[5].Price, 0.0 ) );
    EXPECT_TRUE( IsGreat( lDIp[5].Price, lDIn[5].Price ) );
}

TEST( Indicators, DI_FailsOnShortInputOrBadPeriod ) {
    TPriceSeries lDIp;
    TPriceSeries lDIn;
    EXPECT_FALSE( _DI( RisingBars( 2 ), 3, lDIp, lDIn ) );
    EXPECT_FALSE( _DI( RisingBars( 6 ), 1, lDIp, lDIn ) );
}

TEST( Indicators, ADX_RisingTrendSmoke ) {
    const TBarSeries lBars = RisingBars( 9 );
    const TPriceSeries lResult = _ADX( lBars, 3 );

    ASSERT_EQ( lResult.size(), lBars.size() );
    // Первые 2*period-1 точек bad
    for( size_t i = 0; i < 5; ++i ) {
        ExpectBadTick( lResult[i] );
    }
    // Чистый тренд: DX = 100 на каждом шаге -> ADX = 100
    for( size_t i = 5; i < lResult.size(); ++i ) {
        EXPECT_NEAR( lResult[i].Price, 100.0, 1e-6 );
        EXPECT_TRUE( IsEqual( lResult[i].Volume, 1.0 ) );
    }
}

TEST( Indicators, ADX_EmptyOnShortInputOrBadPeriod ) {
    EXPECT_TRUE( _ADX( RisingBars( 2 ), 3 ).empty() );
    EXPECT_TRUE( _ADX( RisingBars( 9 ), 1 ).empty() );
}

//------------------------------------------------------------------------------------------
// _ZigZag / _AbsoluteZigZag
//------------------------------------------------------------------------------------------

TEST( Indicators, ZigZag_EmptyInputGivesEmptySeries ) {
    // Регрессия №2: раньше читался aBars[0] (UB на пустом входе)
    EXPECT_TRUE( _ZigZag( TBarSeries(), 0.1 ).empty() );
}

TEST( Indicators, ZigZag_ThrowsOnNonPositiveGap ) {
    EXPECT_THROW( _ZigZag( TBarSeries(), 0.0 ), std::logic_error );
    EXPECT_THROW( _ZigZag( TBarSeries(), -0.1 ), std::logic_error );
}

TEST( Indicators, ZigZag_TurnDetectionOnSyntheticBars ) {
    // gap=0.1: разворот вниз при Low < head*0.9, вверх при High >= head*1.1;
    // Volume = +-1 маркер ветки, промежуточные точки Price = 0
    const TBarSeries lBars{
        Bar( 0, 100.0, 100.0, 90.0, 95.0 ),
        Bar( 1, 100.0, 105.0, 95.0, 100.0 ),
        Bar( 2, 100.0, 104.0, 96.0, 100.0 ),
        Bar( 3, 100.0, 103.0, 92.0, 95.0 ),
        Bar( 4, 95.0, 95.0, 93.0, 94.0 ),
        Bar( 5, 100.0, 104.0, 100.0, 102.0 ),
        Bar( 6, 105.0, 110.0, 102.0, 108.0 ) };

    const TPriceSeries lResult = _ZigZag( lBars, 0.1 );
    ASSERT_EQ( lResult.size(), lBars.size() );
    ExpectDateTimes( lResult, lBars.size() );

    // Старт: голова = High[0] = 100, ветка Buy
    EXPECT_TRUE( IsEqual( lResult[0].Price, 0.0 ) );  // голова переехала на бар 1
    EXPECT_TRUE( IsEqual( lResult[0].Volume, 1.0 ) );
    ExpectGoodTick( lResult[1], 105.0, 1.0 );         // новая голова вверх
    EXPECT_TRUE( IsEqual( lResult[2].Price, 0.0 ) );
    EXPECT_TRUE( IsEqual( lResult[2].Volume, 1.0 ) );
    ExpectGoodTick( lResult[3], 92.0, -1.0 );         // разворот вниз (92 < 105*0.9)
    EXPECT_TRUE( IsEqual( lResult[4].Price, 0.0 ) );
    EXPECT_TRUE( IsEqual( lResult[4].Volume, -1.0 ) );
    EXPECT_TRUE( IsEqual( lResult[5].Price, 0.0 ) );  // разворот вверх (104), голова потом переехала на бар 6
    EXPECT_TRUE( IsEqual( lResult[5].Volume, 1.0 ) );
    ExpectGoodTick( lResult[6], 110.0, 1.0 );         // голова вверх (110 > 104)
}

TEST( Indicators, AbsoluteZigZag_EmptyInputGivesEmptySeries ) {
    EXPECT_TRUE( _AbsoluteZigZag( TBarSeries(), 10.0 ).empty() );
}

TEST( Indicators, AbsoluteZigZag_ThrowsOnNonPositiveGap ) {
    EXPECT_THROW( _AbsoluteZigZag( TBarSeries(), 0.0 ), std::logic_error );
    EXPECT_THROW( _AbsoluteZigZag( TBarSeries(), -10.0 ), std::logic_error );
}

TEST( Indicators, AbsoluteZigZag_TurnDetectionOnSyntheticBars ) {
    // gap=10 (абсолютный): разворот вниз при Low < head-10, вверх при High >= head+10
    const TBarSeries lBars{
        Bar( 0, 100.0, 100.0, 95.0, 97.0 ),
        Bar( 1, 100.0, 110.0, 105.0, 108.0 ),
        Bar( 2, 108.0, 108.0, 101.0, 104.0 ),
        Bar( 3, 105.0, 105.0, 98.0, 100.0 ),
        Bar( 4, 100.0, 100.0, 96.0, 98.0 ),
        Bar( 5, 105.0, 109.0, 105.0, 107.0 ) };

    const TPriceSeries lResult = _AbsoluteZigZag( lBars, 10.0 );
    ASSERT_EQ( lResult.size(), lBars.size() );

    EXPECT_TRUE( IsEqual( lResult[0].Price, 0.0 ) );
    EXPECT_TRUE( IsEqual( lResult[0].Volume, 1.0 ) );
    ExpectGoodTick( lResult[1], 110.0, 1.0 );   // голова вверх
    EXPECT_TRUE( IsEqual( lResult[2].Price, 0.0 ) );
    EXPECT_TRUE( IsEqual( lResult[3].Price, 0.0 ) );  // голова вниз переехала на бар 4
    EXPECT_TRUE( IsEqual( lResult[3].Volume, -1.0 ) );
    ExpectGoodTick( lResult[4], 96.0, -1.0 );   // разворот вниз (96 < 110-10)
    ExpectGoodTick( lResult[5], 109.0, 1.0 );   // разворот вверх (109 >= 96+10)
}

//------------------------------------------------------------------------------------------
// _MACD
//------------------------------------------------------------------------------------------

TEST( Indicators, MACD_LineIsEmaDifferenceSignalIsEmaOfLine ) {
    // fast=2, slow=3, smooth=2; максимальный период 3 -> линия bad до точки 2,
    // сигнал = _ExponentMA(линия, 2, lag=2)
    const std::vector< TPrice > lPrices{ 10.0, 12.0, 11.0, 15.0, 14.0, 18.0 };
    const TPriceSeries lResult = _MACD( SeriesOf( lPrices ), 2, 3, 2 );

    ASSERT_EQ( lResult.size(), lPrices.size() );

    // Линия до точки max(fast,slow)-1 = 2 — bad, сигнал там тоже bad
    for( size_t i = 0; i < 3; ++i ) {
        ExpectBadTick( lResult[i] );
    }
    // line[2] = EMA2[2]-EMA3[2] = 11-11 = 0; seed сигнала = (0 + 2/3)/2 = 1/3
    // line[3] = 41/3 - 13 = 2/3; line[4] = 125/9 - 27/2 = 7/18
    // signal[4] = (7/18)*(2/3) + (1/3)*(1/3) = 10/27
    // line[5] = 449/27 - 63/4 = 95/108
    // signal[5] = (95/108)*(2/3) + (10/27)*(1/3) = 115/162
    ExpectGoodTick( lResult[3], 1.0 / 3.0 );
    EXPECT_NEAR( lResult[4].Price, 10.0 / 27.0, 1e-9 );
    EXPECT_NEAR( lResult[5].Price, 115.0 / 162.0, 1e-9 );
    EXPECT_TRUE( IsEqual( lResult[4].Volume, 1.0 ) );
    EXPECT_TRUE( IsEqual( lResult[5].Volume, 1.0 ) );
}

TEST( Indicators, MACD_ThrowsOnNonPositivePeriods ) {
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
    EXPECT_THROW( _MACD( lSeries, 0, 3, 2 ), std::logic_error );
    EXPECT_THROW( _MACD( lSeries, 2, -3, 2 ), std::logic_error );
    EXPECT_THROW( _MACD( lSeries, 2, 3, 0 ), std::logic_error );
}

//------------------------------------------------------------------------------------------
// _BollingerBands
//------------------------------------------------------------------------------------------

TEST( Indicators, BollingerBands_ReferenceOnFivePointWindow ) {
    // period=5, sigma=2: популяционная сигма окна {1..5} и {2..6} = sqrt(2)
    const std::vector< TPrice > lPrices{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    TPriceSeries lMin;
    TPriceSeries lMean;
    TPriceSeries lMax;

    ASSERT_TRUE( _BollingerBands( SeriesOf( lPrices ), 5, 2.0, lMin, lMean, lMax ) );
    ASSERT_EQ( lMin.size(), lPrices.size() );
    ASSERT_EQ( lMean.size(), lPrices.size() );
    ASSERT_EQ( lMax.size(), lPrices.size() );

    for( size_t i = 0; i < 4; ++i ) {
        ExpectBadTick( lMin[i] );
        ExpectBadTick( lMean[i] );
        ExpectBadTick( lMax[i] );
    }

    const double lSigma = std::sqrt( 2.0 );
    ExpectGoodTick( lMean[4], 3.0 );
    ExpectGoodTick( lMin[4], 3.0 - 2.0 * lSigma );
    ExpectGoodTick( lMax[4], 3.0 + 2.0 * lSigma );
    ExpectGoodTick( lMean[5], 4.0 );
    ExpectGoodTick( lMin[5], 4.0 - 2.0 * lSigma );
    ExpectGoodTick( lMax[5], 4.0 + 2.0 * lSigma );
}

TEST( Indicators, BollingerBands_FailsWhenInputNotLongerThanPeriod ) {
    TPriceSeries lMin;
    TPriceSeries lMean;
    TPriceSeries lMax;
    const TPriceSeries lSeries = SeriesOf( { 1.0, 2.0, 3.0 } );
    EXPECT_FALSE( _BollingerBands( lSeries, 3, 2.0, lMin, lMean, lMax ) );
    EXPECT_FALSE( _BollingerBands( lSeries, 5, 2.0, lMin, lMean, lMax ) );
}

//------------------------------------------------------------------------------------------
// _Stochastic
//------------------------------------------------------------------------------------------

TEST( Indicators, Stochastic_ReferenceAndZeroRange ) {
    const TBarSeries lBars{
        Bar( 0, 9.0, 10.0, 8.0, 9.0 ),
        Bar( 1, 10.0, 12.0, 9.0, 11.0 ),
        Bar( 2, 10.5, 11.0, 10.0, 10.5 ),
        Bar( 3, 11.0, 13.0, 11.0, 12.0 ) };

    const TPriceSeries lResult = _Stochastic( lBars, 3 );
    ASSERT_EQ( lResult.size(), lBars.size() );

    for( size_t i = 0; i < 3; ++i ) {
        ExpectBadTick( lResult[i] );
    }
    // Окно [1..3]: maxH=13, minL=9 -> %K = 100*(12-9)/4 = 75
    ExpectGoodTick( lResult[3], 75.0 );
}

TEST( Indicators, Stochastic_ZeroRangeGivesFifty ) {
    const TBarSeries lBars{
        Bar( 0, 10.0, 10.0, 10.0, 10.0 ),
        Bar( 1, 10.0, 10.0, 10.0, 10.0 ),
        Bar( 2, 10.0, 10.0, 10.0, 10.0 ),
        Bar( 3, 10.0, 10.0, 10.0, 10.0 ) };

    const TPriceSeries lResult = _Stochastic( lBars, 2 );
    ASSERT_EQ( lResult.size(), lBars.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    ExpectGoodTick( lResult[2], 50.0 );
    ExpectGoodTick( lResult[3], 50.0 );
}

TEST( Indicators, Stochastic_ShortInputGivesZeroTicks ) {
    const TBarSeries lBars{
        Bar( 0, 10.0, 10.0, 10.0, 10.0 ),
        Bar( 1, 10.0, 10.0, 10.0, 10.0 ) };

    const TPriceSeries lResult = _Stochastic( lBars, 3 );
    ASSERT_EQ( lResult.size(), lBars.size() );
    for( size_t i = 0; i < lResult.size(); ++i ) {
        EXPECT_TRUE( IsEqual( lResult[i].Price, 0.0 ) );
        EXPECT_TRUE( IsEqual( lResult[i].Volume, 0.0 ) );
    }
}

//------------------------------------------------------------------------------------------
// _ChannelSize
//------------------------------------------------------------------------------------------

TEST( Indicators, ChannelSize_Reference ) {
    const TBarSeries lBars{
        Bar( 0, 9.0, 10.0, 8.0, 9.0 ),
        Bar( 1, 10.0, 12.0, 9.0, 11.0 ),
        Bar( 2, 10.5, 11.0, 10.0, 10.5 ),
        Bar( 3, 11.5, 13.0, 11.0, 12.0 ) };

    const TPriceSeries lResult = _ChannelSize( lBars, 2 );
    ASSERT_EQ( lResult.size(), lBars.size() );
    ExpectBadTick( lResult[0] );
    ExpectBadTick( lResult[1] );
    // Окно [1..2]: maxH=12, minL=9
    ExpectGoodTick( lResult[2], 3.0 );
    // Окно [2..3]: maxH=13, minL=10
    ExpectGoodTick( lResult[3], 3.0 );
}

TEST( Indicators, ChannelSize_EmptyOnShortInput ) {
    EXPECT_TRUE( _ChannelSize( TBarSeries{ Bar( 0, 1.0, 2.0, 0.5, 1.5 ) }, 2 ).empty() );
    EXPECT_TRUE( _ChannelSize( TBarSeries(), 2 ).empty() );
}

//------------------------------------------------------------------------------------------
// _RollMinMax vs _RollMinMax_old
//------------------------------------------------------------------------------------------

namespace {

/// 30 детерминированных «пилообразных» баров
TBarSeries SawBars( const size_t aCount ) {
    TBarSeries lBars;
    for( size_t i = 0; i < aCount; ++i ) {
        const TPrice lHigh = 20.0 + static_cast< TPrice >( i % 7 );
        const TPrice lLow = 10.0 + static_cast< TPrice >( i % 5 );
        const TPrice lClose = lLow + ( lHigh - lLow ) / 2.0;
        lBars.push_back( Bar( i, lLow, lHigh, lLow, lClose ) );
    }
    return lBars;
}

} // namespace

TEST( Indicators, RollMinMax_MatchesOldImplementation ) {
    const TBarSeries lBars = SawBars( 30 );
    const int lPeriod = 5;

    for( int lTouch = 0; lTouch <= 1; ++lTouch ) {
        const bool lTouchMode = ( lTouch != 0 );
        TPriceSeries lMinNew;
        TPriceSeries lMaxNew;
        TPriceSeries lMinOld;
        TPriceSeries lMaxOld;

        ASSERT_TRUE( _RollMinMax( lBars, lPeriod, lMinNew, lMaxNew, lTouchMode ) );
        ASSERT_TRUE( _RollMinMax_old( lBars, lPeriod, lMinOld, lMaxOld, lTouchMode ) );

        ASSERT_EQ( lMinNew.size(), lBars.size() );
        ASSERT_EQ( lMaxNew.size(), lBars.size() );
        ASSERT_EQ( lMinOld.size(), lBars.size() );
        ASSERT_EQ( lMaxOld.size(), lBars.size() );

        for( size_t i = 0; i < static_cast< size_t >( lPeriod ); ++i ) {
            ExpectBadTick( lMinNew[i] );
            ExpectBadTick( lMaxNew[i] );
        }
        // min/max совпадают на общей серии (quantile у old не используется)
        for( size_t i = static_cast< size_t >( lPeriod ); i < lBars.size(); ++i ) {
            EXPECT_TRUE( IsEqual( lMinNew[i].Price, lMinOld[i].Price ) ) << "i = " << i;
            EXPECT_TRUE( IsEqual( lMaxNew[i].Price, lMaxOld[i].Price ) ) << "i = " << i;
            EXPECT_TRUE( IsEqual( lMinNew[i].DateTime, lMinOld[i].DateTime ) );
        }
    }
}

TEST( Indicators, RollMinMax_ReferenceValuesAndTouch ) {
    // High: 10,12,12,12,11; Low: 8,9,9,9,10 — экстремумы повторяются для aTouch
    const TBarSeries lBars{
        Bar( 0, 9.0, 10.0, 8.0, 9.5 ),
        Bar( 1, 10.0, 12.0, 9.0, 11.0 ),
        Bar( 2, 10.5, 12.0, 9.0, 10.5 ),
        Bar( 3, 11.0, 12.0, 9.0, 11.0 ),
        Bar( 4, 11.0, 11.0, 10.0, 10.5 ) };

    TPriceSeries lMin;
    TPriceSeries lMax;
    ASSERT_TRUE( _RollMinMax( lBars, 2, lMin, lMax ) );

    ExpectGoodTick( lMin[2], 9.0 );   // окно [1..2]: min Low
    ExpectGoodTick( lMax[2], 12.0 );  // окно [1..2]: max High
    ExpectGoodTick( lMin[3], 9.0 );
    ExpectGoodTick( lMax[3], 12.0 );
    ExpectGoodTick( lMin[4], 9.0 );   // окно [3..4]: min{9,10}
    ExpectGoodTick( lMax[4], 12.0 );  // окно [3..4]: max{12,11}
    // Без aTouch Volume = 1 у всех посчитанных точек
    EXPECT_TRUE( IsEqual( lMin[2].Volume, 1.0 ) );
    EXPECT_TRUE( IsEqual( lMax[3].Volume, 1.0 ) );

    // aTouch: аккумуляция касаний подряд идущих равных экстремумов; первая
    // посчитанная точка обнуляется аккумулятором (цена предыдущей — bad)
    TPriceSeries lMinTouch;
    TPriceSeries lMaxTouch;
    ASSERT_TRUE( _RollMinMax( lBars, 2, lMinTouch, lMaxTouch, true ) );
    EXPECT_TRUE( IsEqual( lMaxTouch[2].Price, 12.0 ) );
    EXPECT_TRUE( isZero( lMaxTouch[2].Volume ) );
    EXPECT_TRUE( IsEqual( lMaxTouch[3].Price, 12.0 ) );
    EXPECT_TRUE( IsEqual( lMaxTouch[3].Volume, 1.0 ) );  // High[3]=12, повтор
    EXPECT_TRUE( IsEqual( lMinTouch[3].Volume, 1.0 ) );  // Low[3]=9, повтор
    EXPECT_TRUE( isZero( lMaxTouch[4].Volume ) );        // High[4]=11 != max окна
    EXPECT_TRUE( isZero( lMinTouch[4].Volume ) );        // Low[4]=10 != min окна
}

TEST( Indicators, RollMinMax_FailsOnBadPeriodOrShortInput ) {
    TPriceSeries lMin;
    TPriceSeries lMax;
    EXPECT_FALSE( _RollMinMax( SawBars( 3 ), 5, lMin, lMax ) );
    EXPECT_FALSE( _RollMinMax( SawBars( 5 ), 0, lMin, lMax ) );
    EXPECT_FALSE( _RollMinMax_old( SawBars( 3 ), 5, lMin, lMax ) );
    EXPECT_FALSE( _RollMinMax_old( SawBars( 5 ), -1, lMin, lMax ) );
}

//------------------------------------------------------------------------------------------
// _SupportRessistance (smoke)
//------------------------------------------------------------------------------------------

TEST( Indicators, SupportRessistance_Smoke ) {
    const TBarSeries lBars{
        Bar( 0, 9.0, 10.0, 8.0, 9.5 ),
        Bar( 1, 10.0, 12.0, 9.0, 11.0 ),
        Bar( 2, 10.5, 11.0, 10.0, 10.5 ),
        Bar( 3, 11.0, 13.0, 11.0, 12.0 ) };

    TPriceSeries lMin;
    TPriceSeries lMax;
    ASSERT_TRUE( _SupportRessistance( lBars, 2, 1, 0.5, lMin, lMax ) );

    ASSERT_EQ( lMin.size(), lBars.size() );
    ASSERT_EQ( lMax.size(), lBars.size() );
    ExpectBadTick( lMin[0] );
    ExpectBadTick( lMax[0] );
    ExpectBadTick( lMin[1] );
    ExpectBadTick( lMax[1] );

    // Посчитанные точки — уровни окна с допуском: внутри размаха серии,
    // Volume = 1
    for( size_t i = 2; i < lBars.size(); ++i ) {
        EXPECT_FALSE( IsLess( lMin[i].Price, 8.0 ) );
        EXPECT_FALSE( IsLess( 13.0, lMin[i].Price ) );
        EXPECT_FALSE( IsLess( lMax[i].Price, 8.0 ) );
        EXPECT_FALSE( IsLess( 13.0, lMax[i].Price ) );
        EXPECT_TRUE( IsEqual( lMin[i].Volume, 1.0 ) );
        EXPECT_TRUE( IsEqual( lMax[i].Volume, 1.0 ) );
    }
    // Тolerance 0.5: min = первое значение >= (min окна + 0.5),
    // max = первое значение > (max окна - 0.5)
    EXPECT_TRUE( IsEqual( lMin[2].Price, 10.0 ) );   // Lows {9,10}: 10 >= 9.5
    EXPECT_TRUE( IsEqual( lMax[2].Price, 12.0 ) );   // Highs {12,11}: 12 > 11.5
    EXPECT_TRUE( IsEqual( lMin[3].Price, 11.0 ) );   // Lows {10,11}: 11 >= 10.5
    EXPECT_TRUE( IsEqual( lMax[3].Price, 13.0 ) );   // Highs {11,13}: 13 > 12.5
}

TEST( Indicators, SupportRessistance_FailsOnBadPeriodOrShortInput ) {
    TPriceSeries lMin;
    TPriceSeries lMax;
    EXPECT_FALSE( _SupportRessistance( SawBars( 2 ), 5, 1, 0.5, lMin, lMax ) );
    EXPECT_FALSE( _SupportRessistance( SawBars( 5 ), 0, 1, 0.5, lMin, lMax ) );
}

//------------------------------------------------------------------------------------------
// _ForwardMinMax
//------------------------------------------------------------------------------------------

TEST( Indicators, ForwardMinMax_LookAheadWindow ) {
    // H = 10+i, L = 5+i; TimeOf(i) = gStartingTime + 60*i
    const size_t lCount = 6;
    TBarSeries lBars;
    for( size_t i = 0; i < lCount; ++i ) {
        const TPrice lBase = static_cast< TPrice >( i );
        lBars.push_back( Bar( i, lBase + 7.0, lBase + 10.0, lBase + 5.0, lBase + 8.0 ) );
    }

    // Окно 60 c: точка i видит только бар i+1
    TPriceSeries lMin;
    TPriceSeries lMax;
    ASSERT_TRUE( _ForwardMinMax( lBars, 60, lMin, lMax ) );
    ASSERT_EQ( lMin.size(), lCount );
    ASSERT_EQ( lMax.size(), lCount );
    for( size_t i = 0; i + 1 < lCount; ++i ) {
        ExpectGoodTick( lMin[i], 5.0 + static_cast< TPrice >( i ) + 1.0 );
        ExpectGoodTick( lMax[i], 10.0 + static_cast< TPrice >( i ) + 1.0 );
    }
    ExpectBadTick( lMin[lCount - 1] );
    ExpectBadTick( lMax[lCount - 1] );

    // Окно 120 c: точка i видит бары i+1 и i+2
    TPriceSeries lMin2;
    TPriceSeries lMax2;
    ASSERT_TRUE( _ForwardMinMax( lBars, 120, lMin2, lMax2 ) );
    ExpectGoodTick( lMin2[0], 6.0 );   // min(6, 7)
    ExpectGoodTick( lMax2[0], 12.0 );  // max(11, 12)
    ExpectGoodTick( lMin2[lCount - 2], 10.0 );  // только бар 5
    ExpectGoodTick( lMax2[lCount - 2], 15.0 );
}

TEST( Indicators, ForwardMinMax_FailsOnShortInput ) {
    TPriceSeries lMin;
    TPriceSeries lMax;
    EXPECT_FALSE( _ForwardMinMax( SawBars( 4 ), 60, lMin, lMax ) );
    EXPECT_TRUE( _ForwardMinMax( SawBars( 5 ), 60, lMin, lMax ) );
}

//------------------------------------------------------------------------------------------
// BasisOfStrategy: BarsToPriceSeries / _CreateBars / ITime
//------------------------------------------------------------------------------------------

namespace {

/// Два бара с различными OHLCV: (10,12,9,11,V=5) и (11,13,10,12,V=6)
TBarSeries TwoDistinctBars() {
    TBarSeries lBars;
    lBars.push_back( Bar( 0, 10.0, 12.0, 9.0, 11.0, 5.0 ) );
    lBars.push_back( Bar( 1, 11.0, 13.0, 10.0, 12.0, 6.0 ) );
    return lBars;
}

} // namespace

TEST( Basis, BarsToPriceSeries_AllTypes ) {
    const TBarSeries lBars = TwoDistinctBars();

    const std::pair< TMAPoint, std::pair< TPrice, TPrice > > lExpected[] = {
        { TMAPoint::MAP_Open, { 10.0, 11.0 } },
        { TMAPoint::MAP_High, { 12.0, 13.0 } },
        { TMAPoint::MAP_Low, { 9.0, 10.0 } },
        { TMAPoint::MAP_Close, { 11.0, 12.0 } },
        { TMAPoint::MAP_Mid, { 10.5, 11.5 } },
        { TMAPoint::MAP_Triple, { 32.0 / 3.0, 35.0 / 3.0 } },
        { TMAPoint::MAP_Volume, { 5.0, 6.0 } } };

    for( const auto &lCase : lExpected ) {
        const TPriceSeries lResult = BarsToPriceSeries( lBars, lCase.first );
        ASSERT_EQ( lResult.size(), lBars.size() ) << "type = " << static_cast< int >( lCase.first );
        EXPECT_TRUE( IsEqual( lResult[0].Price, lCase.second.first ) );
        EXPECT_TRUE( IsEqual( lResult[1].Price, lCase.second.second ) );
        // Volume точки = Volume бара, DateTime сохраняется
        EXPECT_TRUE( IsEqual( lResult[0].Volume, 5.0 ) );
        EXPECT_TRUE( IsEqual( lResult[1].Volume, 6.0 ) );
        EXPECT_TRUE( IsEqual( lResult[0].DateTime, TimeOf( 0 ) ) );
        EXPECT_TRUE( IsEqual( lResult[1].DateTime, TimeOf( 1 ) ) );
    }
}

TEST( Basis, CreateBars_Resampling1mTo5m ) {
    // 12 одноминутных баров: O=10+i, H=10.5+i, L=9.5+i, C=10.2+i, V=i+1.
    // Вход начинается с gStartingTime+300 (бакет 1): по конвенции
    // _CreateBars бакет с индексом 0 отдельным баром не эмитит — сливается
    // со следующим бакетом (см. Doxygen), поэтому данные не должны
    // попадать в [gStartingTime, gStartingTime+period).
    TBarSeries lBars;
    for( size_t i = 0; i < 12; ++i ) {
        const TPrice lBase = static_cast< TPrice >( i );
        lBars.push_back( Bar( i + 5, 10.0 + lBase, 10.5 + lBase, 9.5 + lBase,
                              10.2 + lBase, static_cast< double >( i ) + 1.0 ) );
    }

    const TBarSeries lResult = _CreateBars( lBars, TBarPeriod::Minute5 );
    ASSERT_EQ( lResult.size(), 3u );

    // Группа 0: бары 0..4; DateTime = начало 5-минутного периода
    EXPECT_TRUE( IsEqual( lResult[0].DateTime, gStartingTime + 300.0 ) );
    EXPECT_TRUE( IsEqual( lResult[0].Open, 10.0 ) );
    EXPECT_TRUE( IsEqual( lResult[0].High, 14.5 ) );
    EXPECT_TRUE( IsEqual( lResult[0].Low, 9.5 ) );
    EXPECT_TRUE( IsEqual( lResult[0].Close, 14.2 ) );
    EXPECT_TRUE( IsEqual( lResult[0].Volume, 15.0 ) );

    // Группа 1: бары 5..9
    EXPECT_TRUE( IsEqual( lResult[1].DateTime, gStartingTime + 600.0 ) );
    EXPECT_TRUE( IsEqual( lResult[1].Open, 15.0 ) );
    EXPECT_TRUE( IsEqual( lResult[1].High, 19.5 ) );
    EXPECT_TRUE( IsEqual( lResult[1].Low, 14.5 ) );
    EXPECT_TRUE( IsEqual( lResult[1].Close, 19.2 ) );
    EXPECT_TRUE( IsEqual( lResult[1].Volume, 40.0 ) );

    // Группа 2 (последняя, неполная): бары 10..11
    EXPECT_TRUE( IsEqual( lResult[2].DateTime, gStartingTime + 900.0 ) );
    EXPECT_TRUE( IsEqual( lResult[2].Open, 20.0 ) );
    EXPECT_TRUE( IsEqual( lResult[2].High, 21.5 ) );
    EXPECT_TRUE( IsEqual( lResult[2].Low, 19.5 ) );
    EXPECT_TRUE( IsEqual( lResult[2].Close, 21.2 ) );
    EXPECT_TRUE( IsEqual( lResult[2].Volume, 23.0 ) );
}

TEST( Basis, ITime_FromDouble ) {
    // gStartingTime = 975888000 кратно 86400 -> время суток 0
    EXPECT_TRUE( IsEqual( ITime( gStartingTime ), 0.0 ) );
    EXPECT_TRUE( IsEqual( ITime( gStartingTime + 3661.0 ), 3661.0 ) );
    EXPECT_TRUE( IsEqual( ITime( gStartingTime + 2.0 * 86400.0 + 60.0 ), 60.0 ) );
}

TEST( Basis, ITime_FromString ) {
    EXPECT_TRUE( IsEqual( ITime( std::string( "01:02:03" ) ), 3723.0 ) );
    EXPECT_TRUE( IsEqual( ITime( std::string( "10:30" ) ), 37800.0 ) );
    EXPECT_TRUE( IsEqual( ITime( std::string( "00:00:00" ) ), 0.0 ) );
    // Строка без ':' (невалидная) -> 0.0
    EXPECT_TRUE( IsEqual( ITime( std::string( "100500" ) ), 0.0 ) );
    // Мусор на месте числа -> std::invalid_argument из std::stoi
    EXPECT_ANY_THROW( ITime( std::string( "ab:cd" ) ) );
    EXPECT_ANY_THROW( ITime( std::string( "12:30:xy" ) ) );
}

//------------------------------------------------------------------------------------------
// BasisOfStrategy: IsValidBar / operator+ / округления / DateToStr
//------------------------------------------------------------------------------------------

TEST( Basis, IsValidBar_VolumeAndPrices ) {
    EXPECT_TRUE( IsValidBar( Bar( 0, 10.0, 12.0, 9.0, 11.0 ) ) );
    // Volume = 0 допустим, отрицательный — нет
    EXPECT_TRUE( IsValidBar( Bar( 0, 10.0, 12.0, 9.0, 11.0, 0.0 ) ) );
    EXPECT_FALSE( IsValidBar( TSimpleBar{ TimeOf( 0 ), 10.0, 12.0, 9.0, 11.0, -1.0 } ) );
    EXPECT_FALSE( IsValidBar( TSimpleBar{ TimeOf( 0 ), 10.0, 12.0, 9.0, 11.0, -0.5 } ) );
    // Пустой бар и невалидные цены
    EXPECT_FALSE( IsValidBar( gEmptyBar ) );
    EXPECT_FALSE( IsValidBar( TSimpleBar{ TimeOf( 0 ), 0.0, 12.0, 9.0, 11.0, 1.0 } ) );
    EXPECT_FALSE( IsValidBar( TSimpleBar{ TimeOf( 0 ), gMaxPrice, gMaxPrice, 9.0, 11.0, 1.0 } ) );
}

TEST( Basis, BarMergeOperatorPlus ) {
    const TSimpleBar lFirst = Bar( 0, 10.0, 12.0, 9.0, 11.0, 5.0 );
    const TSimpleBar lSecond = Bar( 1, 11.0, 13.0, 8.5, 12.0, 6.0 );

    const TSimpleBar lMerged = lFirst + lSecond;
    EXPECT_TRUE( IsEqual( lMerged.DateTime, lFirst.DateTime ) );  // DateTime первого
    EXPECT_TRUE( IsEqual( lMerged.Open, 10.0 ) );                 // Open первого
    EXPECT_TRUE( IsEqual( lMerged.High, 13.0 ) );                 // экстремумы
    EXPECT_TRUE( IsEqual( lMerged.Low, 8.5 ) );
    EXPECT_TRUE( IsEqual( lMerged.Close, 12.0 ) );                // Close второго
    EXPECT_TRUE( IsEqual( lMerged.Volume, 11.0 ) );               // сумма Volume

    // Невалидный операнд заменяется другим
    const TSimpleBar lFromEnd = gEmptyBar + lSecond;
    EXPECT_TRUE( IsValidBar( lFromEnd ) );
    EXPECT_TRUE( IsEqual( lFromEnd.Open, 11.0 ) );
    const TSimpleBar lFromStart = lFirst + gEmptyBar;
    EXPECT_TRUE( IsValidBar( lFromStart ) );
    EXPECT_TRUE( IsEqual( lFromStart.Open, 10.0 ) );
    EXPECT_FALSE( IsValidBar( gEmptyBar + gEmptyBar ) );
}

TEST( Basis, RoundToTruncToCeilTo_Grid ) {
    const TPrice lStep = 0.25;
    // Округление к ближайшему
    EXPECT_TRUE( IsEqual( RoundTo( 10.3, lStep ), 10.25 ) );
    EXPECT_TRUE( IsEqual( RoundTo( 10.4, lStep ), 10.5 ) );
    EXPECT_TRUE( IsEqual( RoundTo( 10.25, lStep ), 10.25 ) );
    EXPECT_TRUE( IsEqual( RoundTo( -10.4, lStep ), -10.5 ) );
    // Вниз
    EXPECT_TRUE( IsEqual( TruncTo( 10.99, lStep ), 10.75 ) );
    EXPECT_TRUE( IsEqual( TruncTo( 10.5, lStep ), 10.5 ) );
    EXPECT_TRUE( IsEqual( TruncTo( -10.4, lStep ), -10.25 ) );
    // Вверх
    EXPECT_TRUE( IsEqual( CeilTo( 10.1, lStep ), 10.25 ) );
    EXPECT_TRUE( IsEqual( CeilTo( 10.0, lStep ), 10.0 ) );
    EXPECT_TRUE( IsEqual( CeilTo( -10.4, lStep ), -10.25 ) );
}

TEST( Basis, DateToStr_NonEmpty ) {
    const std::string lStr = DateToStr( 0.0 );
    EXPECT_FALSE( lStr.empty() );
    // ctime-формат, год стабилен вне зависимости от часового пояса
    EXPECT_NE( lStr.find( "1970" ), std::string::npos );
    const std::string lStr2 = DateToStr( gStartingTime );
    EXPECT_FALSE( lStr2.empty() );
    EXPECT_NE( lStr2.find( "2000" ), std::string::npos );
}

//------------------------------------------------------------------------------------------
// BarPeriod
//------------------------------------------------------------------------------------------

TEST( BarPeriod, LengthsForAllValues ) {
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Minute ), 60.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Minute5 ), 300.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Minute10 ), 600.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Minute15 ), 900.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Hour ), 3600.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Hour4 ), 14400.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Day ), 86400.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Week ), 604800.0 ) );
    EXPECT_TRUE( IsEqual( getBarPeriodLength( TBarPeriod::Second ), 1.0 ) );
}

TEST( BarPeriod, NamesForAllValues ) {
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Minute ), "1MIN" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Minute5 ), "5MIN" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Minute10 ), "10MIN" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Minute15 ), "15MIN" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Hour ), "1HOUR" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Hour4 ), "4HOURS" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Day ), "1DAY" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Week ), "1WEEK" );
    EXPECT_EQ( getBarPeriodName( TBarPeriod::Second ), "1SEC" );
}

TEST( BarPeriod, EnumBounds ) {
    // minValue/maxValue — служебные границы; таблица длин покрывает все слоты
    EXPECT_EQ( static_cast< int >( TBarPeriod::minValue ), 0 );
    EXPECT_EQ( static_cast< int >( TBarPeriod::maxValue ), 10 );
    EXPECT_EQ( sizeof( gOutBarPeriod ) / sizeof( gOutBarPeriod[0] ),
               static_cast< size_t >( TBarPeriod::maxValue ) );
}

#ifndef NDEBUG
TEST( BarPeriod, BoundsAssertDiesInDebug ) {
    // Выход за границы — assert в debug-сборке
    EXPECT_DEATH( getBarPeriodLength( TBarPeriod::minValue ), "" );
    EXPECT_DEATH( getBarPeriodLength( TBarPeriod::maxValue ), "" );
    EXPECT_DEATH( getBarPeriodName( TBarPeriod::maxValue ), "" );
}
#endif
