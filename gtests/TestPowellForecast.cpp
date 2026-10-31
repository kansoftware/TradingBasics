#include <gtest/gtest.h>

#include <cmath>

#include "PowellsMethod.h"
#include "CIndicatorES.h"
#include "Forecasting.h"
#include "Prices.h"
#include "DelphisRound.h"

//------------------------------------------------------------------------------------------
namespace {

/// Квадратичная цель x^2+y^2: минимум (0,0), сходится за 2-3 итерации Пауэлла
class TQuadFunction : public PowellsMethod {
    public:
        double func( const TDoubles &p ) override { return p[0] * p[0] + p[1] * p[1]; }
};

/// Доступ к защищённой CIndicatorES::func для проверки штрафа за диапазоны
class TTestableES : public CIndicatorES {
    public:
        double func( const TDoubles &p ) override { return CIndicatorES::func( p ); }
};

} // namespace

//------------------------------------------------------------------------------------------
TEST( Powell, MinimizeQuad ) {
    TQuadFunction lOpt;
    TDoubles lP = { 3.0, -4.0 };

    const int lRet = lOpt.Optimize( lP );

    EXPECT_GT( lRet, 0 );                                  // успех: число итераций
    EXPECT_NEAR( lOpt.GetFret(), 0.0, 1e-6 );              // |fret| < 1e-6
    EXPECT_NEAR( lRet, lOpt.GetIter(), 0.0 );              // GetIter == код возврата
    EXPECT_LT( std::fabs( lP[0] ), 1e-3 );                 // |x| < 1e-3
    EXPECT_LT( std::fabs( lP[1] ), 1e-3 );                 // |y| < 1e-3
}

//------------------------------------------------------------------------------------------
TEST( Powell, BadDimensionReturnsZero ) {
    TQuadFunction lOpt;

    TDoubles lEmpty;
    EXPECT_EQ( lOpt.Optimize( lEmpty ), 0 );               // N == 0

    TDoubles lTwo = { 1.0, 2.0 };
    EXPECT_EQ( lOpt.Optimize( lTwo, 3 ), 0 );              // N > p.size()

    TDoubles lOne = { 1.0 };
    EXPECT_EQ( lOpt.Optimize( lOne, 1 ), 2 );              // n=1: одномерная минимизация, 2 итерации
    EXPECT_NEAR( lOne[0], 0.0, 1e-6 );
    EXPECT_NEAR( lOpt.GetFret(), 0.0, 1e-6 );
    EXPECT_GT( lOpt.Optimize( lOne ), 0 );                 // повторный запуск из найденного минимума
    EXPECT_NEAR( lOne[0], 0.0, 1e-6 );
}

//------------------------------------------------------------------------------------------
TEST( Powell, MaxIterationsReturnsMinusOne ) {
    TQuadFunction lOpt;
    lOpt.SetItMaxPowell( 1 );                              // жёсткий лимит: 1 итерация
    TDoubles lP = { 3.0, -4.0 };

    const int lRet = lOpt.Optimize( lP );

    EXPECT_EQ( lRet, -1 );                                 // исчерпан лимит итераций
    EXPECT_EQ( lOpt.GetIter(), 1 );
}

//------------------------------------------------------------------------------------------
TEST( CIndicatorES, CalcParLinearTrend ) {
    // Линейный тренд 100+2i с малым детерминированным шумом
    TDoubles lData( 30 );
    for( size_t i = 0; i < lData.size(); ++i ) {
        lData[i] = 100.0 + 2.0 * ToDouble(i) + ( ( i % 3 ) ? 0.01 : -0.01 );
    }

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );

    TTestableES lEs;
    EXPECT_TRUE( lEs.CalcPar( lData, lParams ) );          // оптимум найден

    // Параметры в допустимых диапазонах модели (с допуском на сходимость FtolPowell)
    EXPECT_GE( lEs.GetPar(2), 0.05 - 1e-3 ); EXPECT_LE( lEs.GetPar(2), 0.95 + 1e-3 ); // Alpha
    EXPECT_GE( lEs.GetPar(3), 0.05 - 1e-3 ); EXPECT_LE( lEs.GetPar(3), 0.95 + 1e-3 ); // Gamma
    EXPECT_GE( lEs.GetPar(4), 0.05 - 1e-3 ); EXPECT_LE( lEs.GetPar(4), 1.0 + 1e-3 );  // Phi
    // Тренд восстановлен: t ~ 2 (шаг ряда)
    EXPECT_NEAR( lEs.GetPar(1), 2.0, 0.2 );
}

//------------------------------------------------------------------------------------------
TEST( CIndicatorES, CalcParClampsExtremeInitialParams ) {
    TDoubles lData( 20 );
    for( size_t i = 0; i < lData.size(); ++i ) {
        lData[i] = 10.0 + ( ( i % 2 ) ? 0.1 : -0.1 );
    }

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );
    lParams.Alpha = 5.0;                                   // заведомо вне диапазона
    lParams.Gamma = -2.0;

    TTestableES lEs;
    EXPECT_TRUE( lEs.CalcPar( lData, lParams ) );
    EXPECT_GE( lEs.GetPar(2), 0.05 - 1e-3 ); EXPECT_LE( lEs.GetPar(2), 0.95 + 1e-3 );
    EXPECT_GE( lEs.GetPar(3), 0.05 - 1e-3 ); EXPECT_LE( lEs.GetPar(3), 0.95 + 1e-3 );
}

//------------------------------------------------------------------------------------------
TEST( CIndicatorES, GetParOutOfRange ) {
    TTestableES lEs;

    EXPECT_EQ( lEs.GetPar( 5 ), 0.0 );                     // выше числа параметров
    EXPECT_EQ( lEs.GetPar( -1 ), 0.0 );                    // отрицательный индекс
}

//------------------------------------------------------------------------------------------
TEST( CIndicatorES, FuncPenaltyForExtremeParams ) {
    // Ряд 10 +- 0.1: путь ES почти не зависит от Alpha/Gamma -> виден чистый штраф
    TDoubles lData( 20 );
    for( size_t i = 0; i < lData.size(); ++i ) {
        lData[i] = 10.0 + ( ( i % 2 ) ? 0.1 : -0.1 );
    }

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );
    TTestableES lEs;
    ASSERT_TRUE( lEs.CalcPar( lData, lParams ) );          // заполняет Dat/Dlen

    const double lNorm = lEs.func( { 10.0, 0.0, 0.5, 0.5, 0.5 } );
    const double lAlp  = lEs.func( { 10.0, 0.0, 2.0, 0.5, 0.5 } );  // Alpha > 0.95
    const double lGam  = lEs.func( { 10.0, 0.0, 0.5, -3.0, 0.5 } ); // Gamma < 0.05
    const double lPhi  = lEs.func( { 10.0, 0.0, 0.5, 0.5, 5.0 } );  // Phi > 1.0

    EXPECT_GT( lAlp, lNorm );                              // штраф 200x внутри log
    EXPECT_GT( lGam, lNorm );
    EXPECT_GT( lPhi, lNorm );
}

//------------------------------------------------------------------------------------------
TEST( Forecast, LinearTrendNextValueAndParams ) {
    TDoubles lData( 30 );
    for( size_t i = 0; i < lData.size(); ++i ) {
        lData[i] = 100.0 + 2.0 * ToDouble(i) + ( ( i % 3 ) ? 0.01 : -0.01 );
    }

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );

    const TForecastResult lRes = Forecast( lData, lParams );

    EXPECT_NE( lRes.forecast, GetBadPrice() );             // фитирование успешно
    EXPECT_NEAR( lRes.forecast, 160.0, 0.1 );              // следующая точка тренда 100+2*30
    EXPECT_LE( lRes.lower, lRes.forecast );
    EXPECT_LE( lRes.forecast, lRes.upper );
    // Настроенные параметры возвращаются в aioParams
    EXPECT_NEAR( lParams.S, 158.0, 1.0 );                  // последний уровень
    EXPECT_NEAR( lParams.t, 2.0, 0.2 );                    // восстановленный тренд
    EXPECT_GE( lParams.Alpha, 0.05 - 1e-3 ); EXPECT_LE( lParams.Alpha, 0.95 + 1e-3 );
    EXPECT_GE( lParams.Gamma, 0.05 - 1e-3 ); EXPECT_LE( lParams.Gamma, 0.95 + 1e-3 );
    EXPECT_GE( lParams.Phi, 0.05 - 1e-3 ); EXPECT_LE( lParams.Phi, 1.0 + 1e-3 );
}

//------------------------------------------------------------------------------------------
TEST( Forecast, NearConstantSeriesNarrowInterval ) {
    // Строго постоянный ряд даёт sse==0 -> log(0)=-inf и срыв фитирования,
    // поэтому берём детерминированный околопостоянный ряд 7 +- 1e-6
    TDoubles lData( 30 );
    for( size_t i = 0; i < lData.size(); ++i ) {
        lData[i] = 7.0 + ( ( i % 2 ) ? 1e-6 : -1e-6 );
    }

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );

    const TForecastResult lRes = Forecast( lData, lParams );

    EXPECT_NE( lRes.forecast, GetBadPrice() );
    EXPECT_NEAR( lRes.forecast, 7.0, 1e-5 );               // прогноз ~ константе
    EXPECT_LE( lRes.upper - lRes.lower, 1e-3 );            // интервал узкий
}

//------------------------------------------------------------------------------------------
TEST( Forecast, SinglePointSeriesReturnsBad ) {
    // Регресс: NDat < 2 -> деление на (NDat-1) давало NaN-варианс
    TDoubles lData = { 5.0 };

    TForecastModelParams lParams;
    lParams.SetDefault( lData[0] );

    const TForecastResult lRes = Forecast( lData, lParams );

    EXPECT_EQ( lRes.forecast, GetBadPrice() );
    EXPECT_EQ( lRes.lower, GetBadPrice() );
    EXPECT_EQ( lRes.upper, GetBadPrice() );
}
