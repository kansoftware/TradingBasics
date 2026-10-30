#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "RollRange.h"
#include "RollRangeBar.h"
#include "RollSMA.h"
#include "BasisOfStrategy.h"

//------------------------------------------------------------------------------------------
// Регресс №1: при p=1.0 (дефолт) pN клампится к -1, quantile == max окна.
// До фикса pN == 0 -> advance( cend(), 0 ) -> разыменование cend() (UB, мусорное значение).
//------------------------------------------------------------------------------------------
TEST( RollRange, QuantileP1IsMax ) {

    RollRange x( 3, 1.0 );
    x.Add( 1.0 );
    x.Add( 2.0 );
    x.Add( 3.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 3.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 3.0 );

    // выталкивание старых значений: quantile остаётся max скользящего окна
    x.Add( 0.0 );   // окно {2, 3, 0}
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 3.0 );
    x.Add( 5.0 );   // окно {3, 0, 5}
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 5.0 );

    // дефолтный aP == 1.0
    RollRange y( 3 );
    y.Add( 7.0 );
    y.Add( 6.0 );
    y.Add( 8.0 );
    EXPECT_DOUBLE_EQ( y.GetValue().quantile, 8.0 );

    // окно из одного элемента
    RollRange z( 1, 1.0 );
    z.Add( 7.0 );
    EXPECT_TRUE( z.IsFormed() );
    EXPECT_DOUBLE_EQ( z.GetValue().quantile, 7.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, QuantileP0IsMin ) {

    RollRange x( 3, 0.0 );
    x.Add( 5.0 );
    x.Add( 1.0 );
    x.Add( 3.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 1.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().min, 1.0 );

    x.Add( 0.5 );   // окно {1, 3, 0.5}
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 0.5 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, QuantileMedianEvenOdd ) {

    // нечётное окно: p=0.5 -> точная медиана
    RollRange odd( 5, 0.5 );
    for( double v : {1.0, 2.0, 3.0, 4.0, 5.0} ) {
        odd.Add( v );
    }
    EXPECT_DOUBLE_EQ( odd.GetValue().quantile, 3.0 );

    // чётное окно: p=0.5 -> верхняя медиана (третья из четырёх)
    RollRange even( 4, 0.5 );
    for( double v : {1.0, 2.0, 3.0, 4.0} ) {
        even.Add( v );
    }
    EXPECT_DOUBLE_EQ( even.GetValue().quantile, 3.0 );

    // p=0.85, n=10: pN = Trunc(-1.5) = -2 -> второй с конца (девятый из десяти)
    RollRange x( 10, 0.85 );
    for( double v : {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0} ) {
        x.Add( v );
    }
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 9.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, MinMaxSlideDuplicates ) {

    // дубликаты: multiset::erase( find ) удаляет ровно один экземпляр
    RollRange x( 3 );
    x.Add( 5.0 );
    x.Add( 5.0 );
    x.Add( 5.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().min, 5.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 5.0 );

    x.Add( 1.0 );   // окно {5, 5, 1}
    EXPECT_DOUBLE_EQ( x.GetValue().min, 1.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 5.0 );

    x.Add( 9.0 );   // окно {5, 1, 9}: вытолкнут только один экземпляр "5"
    EXPECT_DOUBLE_EQ( x.GetValue().min, 1.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 9.0 );

    // выталкивание значения, все экземпляры которого в окне
    RollRange y( 2 );
    y.Add( 4.0 );
    y.Add( 4.0 );
    EXPECT_DOUBLE_EQ( y.GetValue().min, 4.0 );
    EXPECT_DOUBLE_EQ( y.GetValue().max, 4.0 );

    y.Add( 4.0 );   // окно {4, 4}: удалили один "4", второй остался
    EXPECT_DOUBLE_EQ( y.GetValue().min, 4.0 );
    EXPECT_DOUBLE_EQ( y.GetValue().max, 4.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, History ) {

    const double lBad = -333333.33;
    RollRange x( 3 );

    x.Add( 5.0 );
    ASSERT_EQ( x.GetMinHistory().size(), 1 );
    ASSERT_EQ( x.GetMaxHistory().size(), 1 );
    ASSERT_EQ( x.GetQuantileHistory().size(), 1 );
    EXPECT_DOUBLE_EQ( x.GetMinHistory().front(), lBad );
    EXPECT_DOUBLE_EQ( x.GetMaxHistory().front(), lBad );
    EXPECT_DOUBLE_EQ( x.GetQuantileHistory().front(), lBad );

    x.Add( 1.0 );
    ASSERT_EQ( x.GetQuantileHistory().size(), 2 );
    EXPECT_DOUBLE_EQ( x.GetQuantileHistory().back(), lBad );

    x.Add( 3.0 );   // окно сформировано
    ASSERT_EQ( x.GetMinHistory().size(), 3 );
    ASSERT_EQ( x.GetMaxHistory().size(), 3 );
    ASSERT_EQ( x.GetQuantileHistory().size(), 3 );
    EXPECT_DOUBLE_EQ( x.GetMinHistory().back(), 1.0 );
    EXPECT_DOUBLE_EQ( x.GetMaxHistory().back(), 5.0 );
    EXPECT_DOUBLE_EQ( x.GetQuantileHistory().back(), 5.0 ); // дефолт p=1.0 -> max
}

//------------------------------------------------------------------------------------------
TEST( RollRange, Reset ) {

    RollRange x( 2 );
    x.Add( 1.0 );
    x.Add( 2.0 );
    EXPECT_TRUE( x.IsFormed() );
    ASSERT_EQ( x.GetMinHistory().size(), 2 );

    x.Reset();
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( x.GetMinHistory().empty() );
    EXPECT_TRUE( x.GetMaxHistory().empty() );
    EXPECT_TRUE( x.GetQuantileHistory().empty() );

    // после Reset индикатор работает заново
    x.Add( 10.0 );
    x.Add( 20.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().min, 10.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 20.0 );
    EXPECT_EQ( x.GetMinHistory().size(), 2 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, GetDelta ) {

    RollRange x( 3 );
    x.Add( 10.0 );
    x.Add( 20.0 );
    x.Add( 30.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetDelta(), 2.0 );

    // первое значение окна ~0 -> NAN
    RollRange y( 3 );
    y.Add( 0.0 );
    y.Add( 1.0 );
    y.Add( 2.0 );
    EXPECT_TRUE( std::isnan( y.GetDelta() ) );

    // отрицательное первое значение
    RollRange z( 3 );
    z.Add( -10.0 );
    z.Add( -20.0 );
    z.Add( -30.0 );
    EXPECT_DOUBLE_EQ( z.GetDelta(), 2.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, IsFormed ) {

    RollRange x( 2 );
    EXPECT_FALSE( x.IsFormed() );
    x.Add( 1.0 );
    EXPECT_FALSE( x.IsFormed() );
    x.Add( 2.0 );
    EXPECT_TRUE( x.IsFormed() );
    x.Add( 3.0 );   // окно скользит, состояние "сформирован" сохраняется
    EXPECT_TRUE( x.IsFormed() );
}

//------------------------------------------------------------------------------------------
TEST( RollRange, CtorThrows ) {

    EXPECT_THROW( RollRange( 0 ), std::invalid_argument );
    EXPECT_THROW( RollRange( 3, -0.1 ), std::invalid_argument );
    EXPECT_THROW( RollRange( 3, 1.1 ), std::invalid_argument );
}

//------------------------------------------------------------------------------------------
// Регресс №3: дефолт aTollerance == 0.0 (старый дефолт -1.0 гарантированно
// бросал invalid_argument из ctor'а).
//------------------------------------------------------------------------------------------
TEST( RollRange_with_tollerance, DefaultCtorDoesNotThrow ) {

    ASSERT_NO_THROW( RollRange_with_tollerance x( 10 ) );

    RollRange_with_tollerance x( 10 );
    EXPECT_NO_THROW( x.Add( 100.0 ) );
    EXPECT_FALSE( x.IsFormed() );

    // с дефолтными aMinTouch=1 и aTollerance=0.0 касаний "ниже min+tol" нет
    // (0 < 1), поэтому обе границы — bad-значения
    EXPECT_DOUBLE_EQ( x.GetValue().min, -333333.33 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, -333333.33 );
}

//------------------------------------------------------------------------------------------
TEST( RollRange_with_tollerance, MinMaxToleranceSemantics ) {

    const double lBad = -333333.33;

    // достаточно касаний снизу и сверху (minTouch=2): min = первое значение
    // >= (min окна + tolerance), max = первое значение > (max окна - tolerance)
    RollRange_with_tollerance x( 6, 2, 0.5 );
    for( double v : {1.0, 1.0, 3.0, 3.0, 5.0, 5.0} ) {
        x.Add( v );
    }
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().min, 3.0 );  // ниже 1.5 лежат 2 касания
    EXPECT_DOUBLE_EQ( x.GetValue().max, 5.0 );  // выше 4.5 лежат 2 касания

    // недостаточно касаний -> bad-значения с обеих сторон
    RollRange_with_tollerance y( 4, 3, 0.1 );
    for( double v : {1.0, 1.05, 2.0, 2.05} ) {
        y.Add( v );
    }
    EXPECT_DOUBLE_EQ( y.GetValue().min, lBad );
    EXPECT_DOUBLE_EQ( y.GetValue().max, lBad );

    // кастомный bad-value
    RollRange_with_tollerance z( 4, 3, 0.1, -777.0 );
    for( double v : {1.0, 1.05, 2.0, 2.05} ) {
        z.Add( v );
    }
    EXPECT_DOUBLE_EQ( z.GetValue().min, -777.0 );
    EXPECT_DOUBLE_EQ( z.GetValue().max, -777.0 );

    // окно скользит: min/max пересчитываются по текущему окну
    RollRange_with_tollerance w( 2, 1, 0.5 );
    w.Add( 1.0 );
    w.Add( 10.0 );  // окно {1, 10}
    EXPECT_DOUBLE_EQ( w.GetValue().min, 10.0 );
    EXPECT_DOUBLE_EQ( w.GetValue().max, 10.0 );

    w.Add( 20.0 );  // окно {10, 20}
    EXPECT_DOUBLE_EQ( w.GetValue().min, 20.0 );
    EXPECT_DOUBLE_EQ( w.GetValue().max, 20.0 );

    w.Reset();
    EXPECT_FALSE( w.IsFormed() );
}

//------------------------------------------------------------------------------------------
// Регресс №2: при p=1.0 (дефолт) GetValue() == max окна (до фикса —
// разыменование cend()).
//------------------------------------------------------------------------------------------
TEST( RollRangeLazy, QuantileP1IsMax ) {

    RollRangeLazy x( 3 );
    x.Add( 1.0 );
    x.Add( 2.0 );
    x.Add( 3.0 );
    EXPECT_TRUE( x.IsFormed() );

    const Range lRange = x.GetValue();
    EXPECT_DOUBLE_EQ( lRange.quantile, 3.0 );
    EXPECT_DOUBLE_EQ( lRange.max, 3.0 );
    EXPECT_DOUBLE_EQ( lRange.min, 1.0 );

    x.Add( 0.0 );   // окно {2, 3, 0}
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 3.0 );

    x.Add( 5.0 );   // окно {3, 0, 5}
    EXPECT_DOUBLE_EQ( x.GetValue().quantile, 5.0 );

    RollRangeLazy y( 1, 1.0 );
    y.Add( 7.0 );
    EXPECT_DOUBLE_EQ( y.GetValue().quantile, 7.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollRangeLazy, HistoryAndReset ) {

    const double lBad = -333333.33;
    RollRangeLazy x( 2 );

    x.Add( 5.0 );
    ASSERT_EQ( x.GetMinHistory().size(), 1 );
    EXPECT_DOUBLE_EQ( x.GetMinHistory().front(), lBad );
    EXPECT_DOUBLE_EQ( x.GetMaxHistory().front(), lBad );

    x.Add( 1.0 );
    ASSERT_EQ( x.GetMinHistory().size(), 2 );
    EXPECT_DOUBLE_EQ( x.GetMinHistory().back(), 1.0 );
    EXPECT_DOUBLE_EQ( x.GetMaxHistory().back(), 5.0 );

    x.Reset();
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( x.GetMinHistory().empty() );
    EXPECT_TRUE( x.GetMaxHistory().empty() );

    x.Add( 4.0 );
    x.Add( 6.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().min, 4.0 );
    EXPECT_DOUBLE_EQ( x.GetValue().max, 6.0 );
}

//------------------------------------------------------------------------------------------
TEST( RollSeesaw, Main ) {

    RollSeesaw x( 2 );
    EXPECT_FALSE( x.IsFormed() );

    x.Add( 10.0 );
    EXPECT_FALSE( x.IsFormed() );

    x.Add( 20.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetDelta(), 1.0 );

    x.Add( 40.0 );  // окно {20, 40}
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetDelta(), 1.0 );

    // первое значение окна ~0 -> NAN
    RollSeesaw y( 2 );
    y.Add( 0.0 );
    y.Add( 5.0 );
    EXPECT_TRUE( std::isnan( y.GetDelta() ) );

    y.Reset();
    EXPECT_FALSE( y.IsFormed() );
    y.Add( 1.0 );
    y.Add( 4.0 );
    EXPECT_TRUE( y.IsFormed() );
    EXPECT_DOUBLE_EQ( y.GetDelta(), 3.0 );

    EXPECT_THROW( RollSeesaw( 0 ), std::invalid_argument );
}

//------------------------------------------------------------------------------------------
TEST( RollSMA, Main ) {

    RollSMA x( 3 );
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( std::isnan( x.GetValue() ) );

    x.Add( 1.0 );
    x.Add( 2.0 );
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( std::isnan( x.GetValue() ) );

    x.Add( 3.0 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue(), 2.0 );

    // совпадение с ручным средним на скользящем окне
    x.Add( 4.0 );   // окно {2, 3, 4}
    EXPECT_DOUBLE_EQ( x.GetValue(), 3.0 );

    x.Add( 5.0 );   // окно {3, 4, 5}
    EXPECT_DOUBLE_EQ( x.GetValue(), 4.0 );

    x.Add( 10.0 );  // окно {4, 5, 10}
    EXPECT_DOUBLE_EQ( x.GetValue(), 19.0 / 3.0 );

    x.Add( -3.0 );  // окно {5, 10, -3}
    EXPECT_DOUBLE_EQ( x.GetValue(), 4.0 );

    x.Reset();
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( std::isnan( x.GetValue() ) );

    // сумма обнулена при Reset
    x.Add( 1.0 );
    x.Add( 2.0 );
    x.Add( 3.0 );
    EXPECT_DOUBLE_EQ( x.GetValue(), 2.0 );

    EXPECT_THROW( RollSMA( 0 ), std::invalid_argument );
}

//------------------------------------------------------------------------------------------
TEST( TRollRangeBar, Main ) {

    const double lBad = -333333.33;
    const TSimpleBar lBar1{gStartingTime,        1.0, 2.0, 1.0, 1.5, 1.0};
    const TSimpleBar lBar2{gStartingTime + 60.0, 1.5, 3.0, 1.5, 2.5, 1.0};
    const TSimpleBar lBar3{gStartingTime + 120.0, 2.0, 2.5, 0.5, 1.0, 1.0};

    TRollRangeBar x( 2 );
    EXPECT_THROW( TRollRangeBar( 0 ), std::invalid_argument );

    x.Add( lBar1 );
    EXPECT_FALSE( x.IsFormed() );
    ASSERT_EQ( x.GetHistory().size(), 1 );
    EXPECT_DOUBLE_EQ( x.GetHistory().front().first, lBad );
    EXPECT_DOUBLE_EQ( x.GetHistory().front().second, lBad );
    EXPECT_TRUE( std::isnan( x.getMean() ) );
    EXPECT_TRUE( std::isnan( x.getErr() ) );

    x.Add( lBar2 );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().first, 1.0 );    // min по Low
    EXPECT_DOUBLE_EQ( x.GetValue().second, 3.0 );   // max по High

    // история: [bad, {min, max}]
    ASSERT_EQ( x.GetHistory().size(), 2 );
    EXPECT_DOUBLE_EQ( x.GetHistory().back().first, 1.0 );
    EXPECT_DOUBLE_EQ( x.GetHistory().back().second, 3.0 );

    // mean по валидным записям истории (диапазон окна min..max):
    // валидная запись одна {1, 3} -> 2.0
    EXPECT_DOUBLE_EQ( x.getMean(), 2.0 );
    // валидных записей < 2 -> err = NAN (регресс №5)
    EXPECT_TRUE( std::isnan( x.getErr() ) );

    x.Add( lBar3 ); // окно {lBar2, lBar3} -> запись {0.5, 3}
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.GetValue().first, 0.5 );
    EXPECT_DOUBLE_EQ( x.GetValue().second, 3.0 );
    ASSERT_EQ( x.GetHistory().size(), 3 );

    // две валидные записи: (2.0 + 2.5) / 2 = 2.25
    EXPECT_DOUBLE_EQ( x.getMean(), 2.25 );
    // err: sqrt( ((2.25-2)^2 + (2.25-2.5)^2) / 0.5 ) / sqrt(2) = sqrt(2)/4
    EXPECT_NEAR( x.getErr(), 0.35355339059327373, 1e-12 );

    x.Reset();
    EXPECT_FALSE( x.IsFormed() );
    EXPECT_TRUE( x.GetHistory().empty() );
}

//------------------------------------------------------------------------------------------
// Регресс №5: при n < 2 getErr == NAN (mean остаётся честным).
//------------------------------------------------------------------------------------------
TEST( TRollRangeBar, ErrIsNanWhenSingleBar ) {

    const TSimpleBar lBar{gStartingTime, 1.0, 2.0, 1.0, 1.5, 1.0};

    TRollRangeBar x( 1 );
    x.Add( lBar );
    EXPECT_TRUE( x.IsFormed() );
    EXPECT_DOUBLE_EQ( x.getMean(), 1.0 );   // диапазон бара High-Low
    EXPECT_TRUE( std::isnan( x.getErr() ) );
}
