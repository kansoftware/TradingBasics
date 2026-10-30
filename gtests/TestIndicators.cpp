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
