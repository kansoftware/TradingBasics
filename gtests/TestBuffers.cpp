#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "RubberBuffer.h"
#include "VolatilityBuffer.h"
#include "VolatilityBarBuffer.h"
#include "LiveSAR.h"
#include "Indicators.h"
#include "DelphisRound.h"
#include "BasisOfStrategy.h"

//------------------------------------------------------------------------------------------
TEST( RubberBuffer, FillAndStats ) {

    TRubberBuffer< double > x( 3 );
    EXPECT_FALSE( x.isFill() );
    EXPECT_FALSE( x.calcStats() );
    EXPECT_TRUE( std::isnan( x.getMean() ) );

    x.add( 1.0 );
    x.add( 2.0 );
    EXPECT_FALSE( x.isFill() );
    EXPECT_FALSE( x.calcStats() );

    x.add( 3.0 );
    EXPECT_TRUE( x.isFill() );
    EXPECT_TRUE( x.calcStats() );

    EXPECT_EQ( x.getMax(), 3.0 );
    EXPECT_EQ( x.getMin(), 1.0 );
    EXPECT_EQ( x.getMean(), 2.0 );
    // SD: sqrt( ((2-1)^2+(2-2)^2+(2-3)^2) / (3-1) ) = 1
    EXPECT_DOUBLE_EQ( x.getSD(), 1.0 );
    // err: sqrt( 2/(3-1.5) ) / sqrt(3) = 2/3
    EXPECT_NEAR( x.getErr(), 2.0 / 3.0, 1e-12 );

    // буфер растёт, статистика пересчитывается по всем накопленным значениям
    x.add( 4.0 );
    x.add( 5.0 );
    x.add( 6.0 );
    EXPECT_TRUE( x.calcStats() );
    EXPECT_EQ( x.getMean(), 3.5 );
    EXPECT_EQ( x.getMin(), 1.0 );
    EXPECT_EQ( x.getMax(), 6.0 );
    EXPECT_NEAR( x.getSD(), std::sqrt( 3.5 ), 1e-12 );
    EXPECT_NEAR( x.getErr(), std::sqrt( 17.5 / 4.5 ) / std::sqrt( 6.0 ), 1e-12 );
}

//------------------------------------------------------------------------------------------
// Регресс №5: при n < 2 calcStats() возвращает true, но getSD/getErr = NAN,
// mean остаётся честным.
//------------------------------------------------------------------------------------------
TEST( RubberBuffer, StatsAreNanWhenSingleElement ) {

    TRubberBuffer< double > x( 1 );
    x.add( 5.0 );
    EXPECT_TRUE( x.isFill() );
    EXPECT_TRUE( x.calcStats() );
    EXPECT_DOUBLE_EQ( x.getMean(), 5.0 );
    EXPECT_TRUE( std::isnan( x.getSD() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
TEST( RubberBuffer, Reset ) {

    TRubberBuffer< double > x( 2 );
    x.add( 1.0 );
    x.add( 2.0 );
    EXPECT_TRUE( x.calcStats() );
    EXPECT_DOUBLE_EQ( x.getMax(), 2.0 );

    x.reset();
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getMin() ) );
    EXPECT_TRUE( std::isnan( x.getMean() ) );
    EXPECT_TRUE( std::isnan( x.getSD() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
    EXPECT_FALSE( x.calcStats() );

    x.add( 10.0 );
    x.add( 20.0 );
    EXPECT_TRUE( x.isFill() );
    EXPECT_TRUE( x.calcStats() );
    EXPECT_DOUBLE_EQ( x.getMean(), 15.0 );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBuffer, FillAndStats ) {

    TVolatilityBuffer< double > x( 3 );
    EXPECT_FALSE( x.isFill() );

    x.add( 1.0 );
    x.add( 2.0 );
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getErr() ) );

    EXPECT_TRUE( x.add( 3.0 ) );
    EXPECT_TRUE( x.isFill() );
    EXPECT_EQ( x.getMax(), 3.0 );
    EXPECT_EQ( x.getMin(), 1.0 );
    EXPECT_EQ( x.getMean(), 2.0 );
    // err: sqrt( 2/(3-1.5) ) / sqrt(3) = 2/3
    EXPECT_NEAR( x.getErr(), 2.0 / 3.0, 1e-12 );

    // заполненный буфер новые значения не принимает
    EXPECT_FALSE( x.add( 100.0 ) );
    EXPECT_EQ( x.getMax(), 3.0 );
    EXPECT_EQ( x.getMean(), 2.0 );

    x.reset();
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getMin() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
// Регресс №5: при fCount < 2 (единичный буфер) err = NAN, mean честный.
//------------------------------------------------------------------------------------------
TEST( VolatilityBuffer, ErrIsNanWhenSingleElement ) {

    TVolatilityBuffer< double > x( 1 );
    EXPECT_TRUE( x.add( 7.0 ) );
    EXPECT_TRUE( x.isFill() );
    EXPECT_DOUBLE_EQ( x.getMean(), 7.0 );
    EXPECT_DOUBLE_EQ( x.getMax(), 7.0 );
    EXPECT_DOUBLE_EQ( x.getMin(), 7.0 );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBarBuffer, FillAndStats ) {

    const TSimpleBar lBar1{gStartingTime,        1.0, 2.0, 1.0, 1.5, 1.0};
    const TSimpleBar lBar2{gStartingTime + 60.0, 1.5, 3.0, 1.0, 2.5, 1.0};

    TVolatilityBarBuffer x( 2 );
    EXPECT_FALSE( x.isFill() );

    EXPECT_FALSE( x.add( lBar1 ) );
    EXPECT_TRUE( x.add( lBar2 ) );
    EXPECT_TRUE( x.isFill() );

    // min по Low, max по High
    EXPECT_DOUBLE_EQ( x.getMin(), 1.0 );
    EXPECT_DOUBLE_EQ( x.getMax(), 3.0 );
    // mean по диапазонам High-Low: (1 + 2) / 2 = 1.5
    EXPECT_DOUBLE_EQ( x.getMean(), 1.5 );
    // err: sqrt( ((1.5-1)^2+(1.5-2)^2) / 0.5 ) / sqrt(2) = 1/sqrt(2)
    EXPECT_NEAR( x.getErr(), 1.0 / std::sqrt( 2.0 ), 1e-12 );

    x.reset();
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getMin() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
// Регресс №5: при fCount < 2 err = NAN, mean честный.
//------------------------------------------------------------------------------------------
TEST( VolatilityBarBuffer, ErrIsNanWhenSingleElement ) {

    const TSimpleBar lBar{gStartingTime, 1.0, 2.0, 1.0, 1.5, 1.0};

    TVolatilityBarBuffer x( 1 );
    EXPECT_TRUE( x.add( lBar ) );
    EXPECT_TRUE( x.isFill() );
    EXPECT_DOUBLE_EQ( x.getMean(), 1.0 );   // High-Low
    EXPECT_DOUBLE_EQ( x.getMax(), 2.0 );
    EXPECT_DOUBLE_EQ( x.getMin(), 1.0 );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
// Регресс №4: getMax/getMin пустого буфера = NAN (до фикса — разыменование
// begin/rbegin пустого multiset).
//------------------------------------------------------------------------------------------
TEST( VolatilityBarRollBuffer, EmptyMinMaxIsNan ) {

    TVolatilityBarRollBuffer x( 2 );
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getMin() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );

    x.reset();
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getMin() ) );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBarRollBuffer, FillSlideDuplicates ) {

    const TSimpleBar lBar1{gStartingTime,        1.0, 2.0, 1.0, 1.5, 1.0};    // диапазон 1
    const TSimpleBar lBar2{gStartingTime + 60.0, 1.0, 3.0, 1.0, 2.0, 1.0};    // диапазон 2
    const TSimpleBar lBar3{gStartingTime + 120.0, 2.0, 5.0, 2.0, 4.0, 1.0};   // диапазон 3

    TVolatilityBarRollBuffer x( 2 );
    EXPECT_FALSE( x.add( lBar1 ) );
    EXPECT_TRUE( x.add( lBar2 ) );
    EXPECT_TRUE( x.isFill() );

    EXPECT_DOUBLE_EQ( x.getMax(), 2.0 );
    EXPECT_DOUBLE_EQ( x.getMin(), 1.0 );
    EXPECT_DOUBLE_EQ( x.getMean(), 1.5 );
    // err: sqrt( ((1.5-1)^2+(1.5-2)^2) / 0.5 ) / sqrt(2) = 1/sqrt(2)
    EXPECT_NEAR( x.getErr(), 1.0 / std::sqrt( 2.0 ), 1e-12 );

    // скольжение: диапазон 3 вытесняет диапазон 1
    EXPECT_TRUE( x.add( lBar3 ) );
    EXPECT_DOUBLE_EQ( x.getMax(), 3.0 );
    EXPECT_DOUBLE_EQ( x.getMin(), 2.0 );
    EXPECT_DOUBLE_EQ( x.getMean(), 2.5 );

    // дубликаты диапазонов: erase( find ) удаляет ровно один экземпляр
    TVolatilityBarRollBuffer y( 2 );
    y.add( lBar1 );
    y.add( lBar1 );
    y.add( lBar1 ); // вытолкнут один из двух одинаковых
    EXPECT_TRUE( y.isFill() );
    EXPECT_DOUBLE_EQ( y.getMax(), 1.0 );
    EXPECT_DOUBLE_EQ( y.getMin(), 1.0 );
    EXPECT_DOUBLE_EQ( y.getMean(), 1.0 );

    x.reset();
    EXPECT_FALSE( x.isFill() );
    EXPECT_TRUE( std::isnan( x.getMax() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
// Регресс №5: при fCount < 2 err = NAN, mean честный.
//------------------------------------------------------------------------------------------
TEST( VolatilityBarRollBuffer, ErrIsNanWhenSingleElement ) {

    const TSimpleBar lBar1{gStartingTime,        1.0, 2.0, 1.0, 1.5, 1.0};    // диапазон 1
    const TSimpleBar lBar2{gStartingTime + 60.0, 0.5, 1.5, 0.5, 1.0, 1.0};    // диапазон 1

    TVolatilityBarRollBuffer x( 1 );
    EXPECT_TRUE( x.add( lBar1 ) );
    EXPECT_TRUE( x.isFill() );
    EXPECT_DOUBLE_EQ( x.getMean(), 1.0 );
    EXPECT_DOUBLE_EQ( x.getMax(), 1.0 );
    EXPECT_DOUBLE_EQ( x.getMin(), 1.0 );
    EXPECT_TRUE( std::isnan( x.getErr() ) );

    EXPECT_TRUE( x.add( lBar2 ) );
    EXPECT_TRUE( x.isFill() );
    EXPECT_DOUBLE_EQ( x.getMean(), 1.0 );
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}

//------------------------------------------------------------------------------------------
// Первый Add — warm-up: SAR ещё не рассчитан, возвращается Low первого бара
// (согласуется с _ParabolicSar: result[0] == bars[0].Low).
//------------------------------------------------------------------------------------------
TEST( LiveSAR, FirstAddIsWarmup ) {

    TLiveSAR x;
    const TSimpleBar lBar{gStartingTime, 101.0, 102.0, 100.0, 101.0, 1.0};
    EXPECT_DOUBLE_EQ( x.Add( lBar ), 100.0 );
}

//------------------------------------------------------------------------------------------
// Монотонно растущая серия: SAR ниже цены, AF растёт до maxAf и упирается в
// него. Эталонные значения (aAf=0.02, aMaxAf=0.2) получены эталонной
// реализацией _ParabolicSar (бар-в-бар сходимость с ней проверяется ниже);
// значения баров 10-12 соответствуют AF, зафиксированному на 0.2.
//------------------------------------------------------------------------------------------
TEST( LiveSAR, RisingSeries ) {

    const double lAf = 0.02;
    const double lMaxAf = 0.2;

    const double lExpected[] = {
        100.0,                  // бар 1 (SAR не опускается ниже priorSar)
        100.08,                 // бар 2
        100.2552,               // бар 3
        100.554784,             // бар 4
        100.9993056,            // бар 5
        101.599388928,          // бар 6
        102.35547447808,        // бар 7
        103.25859856158719,     // бар 8
        104.2920508205015,      // бар 9  (AF достиг 0.2)
        105.4336406564012,      // бар 10 (AF зафиксирован на 0.2)
        106.54691252512096,     // бар 11
        107.63753002009676      // бар 12 (высокий High=162, AF остался 0.2)
    };
    const size_t lCount = sizeof( lExpected ) / sizeof( lExpected[0] );

    TLiveSAR x( lAf, lMaxAf );

    for( size_t i = 0; i <= lCount; ++i ) {
        const TSimpleBar lBar{
            gStartingTime + ToDouble( i ) * 60.0,
            100.0 + ToDouble( i ) + 0.5,
            101.0 + ToDouble( i ) + ( i == lCount ? 49.0 : 0.0 ),   // последний бар — большой High
            100.0 + ToDouble( i ),
            100.0 + ToDouble( i ) + 0.5,
            1.0 };

        const TPrice lSar = x.Add( lBar );

        if( i == 0 ) {
            EXPECT_DOUBLE_EQ( lSar, 100.0 );    // warm-up: Low первого бара
            continue;
        }

        EXPECT_NEAR( lSar, lExpected[i - 1], 1e-9 );
        EXPECT_TRUE( lSar < lBar.Low );         // SAR ниже цены в растущем тренде
    }
}

//------------------------------------------------------------------------------------------
// Сходимость с _ParabolicSar на детерминированной серии баров с подъёмами,
// спадами и разворотами тренда. Обе реализации должны совпадать бар в бар.
//------------------------------------------------------------------------------------------
TEST( LiveSAR, ConvergenceWithParabolicSar ) {

    // захардкоженная детерминированная серия close-цен: подъёмы, спады, развороты
    const double lCloses[] = {
        100.0, 101.2, 102.5, 103.1, 104.4, 103.2, 102.0, 100.5,  99.2,  98.5,
        100.8, 102.3, 104.0, 105.5, 104.8, 103.9, 102.6, 101.0,  99.8,  98.0,
         97.2,  99.9, 101.2, 103.3, 105.8, 107.1, 108.4, 107.2, 106.0, 104.7,
        103.5, 102.2, 104.0, 105.8, 107.1, 108.4, 109.9, 108.4, 106.0, 104.0 };
    const size_t lCount = sizeof( lCloses ) / sizeof( lCloses[0] );

    TBarSeries lBars;
    lBars.reserve( lCount );
    for( size_t i = 0; i < lCount; ++i ) {
        TSimpleBar lBar;
        lBar.DateTime = gStartingTime + ToDouble( i ) * 60.0;
        lBar.Open = ( i == 0 ? lCloses[0] - 0.5 : lCloses[i - 1] );
        lBar.Close = lCloses[i];
        lBar.High = std::max( lBar.Open, lBar.Close ) + 0.6;
        lBar.Low = std::min( lBar.Open, lBar.Close ) - 0.6;
        lBar.Volume = 1.0;
        lBars.push_back( lBar );
    }

    const TPriceSeries lRef = _ParabolicSar( lBars, 0.02, 0.2 );
    ASSERT_EQ( lRef.size(), lCount );

    TLiveSAR x( 0.02, 0.2 );
    size_t lCompared = 0;
    for( size_t i = 0; i < lCount; ++i ) {
        const TPrice lSar = x.Add( lBars[i] );
        if( std::isnan( lSar ) && std::isnan( lRef[i].Price ) ) {
            continue;   // warm-up с обеих сторон (в данной серии не встречается)
        }
        EXPECT_NEAR( lSar, lRef[i].Price, 1e-9 ) << "бар " << i;
        lCompared++;
    }
    EXPECT_EQ( lCompared, lCount ); // расхождений warm-up нет: сравнены все бары
}
