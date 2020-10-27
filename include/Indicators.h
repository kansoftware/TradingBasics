/**
 * \file Indicators.h
 * \brief Модуль реализующий индикаторы для одноногих стратегий
 * \author kan <kansoftware.ru>
 * \since 2015-11-10
 * \date 2019-07-31
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */

#ifndef BACKTESTER_INDICATORS_H
#define BACKTESTER_INDICATORS_H

#include "BasisOfStrategy.h"
#include "BarPeriod.h"

const size_t gUsingCPU = 4;

TPriceSeries _TrueRange( const TBarSeries & aBars );
TPriceSeries _AverageTrueRange( const TBarSeries & aBars, const int aPeriod );

bool _DM( const TBarSeries & aBars, 
            TPriceSeries & aoDMp, 
            TPriceSeries & aoDMn );
bool _DI( const TBarSeries & aBars, const int aPeriod, 
            TPriceSeries & aoDMIp, 
            TPriceSeries & aoDMIn );

TPriceSeries _ADX( const TBarSeries & aBars, const int aPeriod );

TPriceSeries _SimpleMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _ExponentMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _SmoothedMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _KAMA( const TPriceSeries & aPrices, const int aPeriod, const double aCoeff = 2.0, const double aFastPeriod = 2.0, const double aSlowPeriod = 30.0, const size_t aLag = 0 );

const size_t gSARStabilizationDelay = 200 ;
TPriceSeries _ParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );
TPriceSeries _IntradayParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );
TPriceSeries _AbsoluteZigZag( const TBarSeries & aBars, const double aGap );
TPriceSeries _MACD( const TPriceSeries & aPrices, const int aFastPeriod, const int aSlowPeriod, const int aSmoothPeriod );
TPriceSeries _ZigZag( const TBarSeries & aBars, const double aGap );

bool _Forecasting( const TPriceSeries & aPrices, const size_t aForecastPeriod, const double aConfidenceIntervals, 
                    TPriceSeries & aoUpperBorder, 
                    TPriceSeries & aoForecast, 
                    TPriceSeries & aoLowerBorder );

bool _RollMinMax( const TBarSeries & aBars, const int aPeriod, 
            TPriceSeries & aoMin, 
            TPriceSeries & aoMax,
            const bool aTouch = false );

bool _RollMinMax_old( const TBarSeries & aBars, const int aPeriod, 
            TPriceSeries & aoMin, 
            TPriceSeries & aoMax,
            const bool aTouch = false );

bool _SupportRessistance( 
    const TBarSeries & aBars, 
    const int aPeriod,
    const size_t aMinTouch, 
    const double aTollerance,
    TPriceSeries & aoMin, 
    TPriceSeries & aoMax );

bool _ForwardMinMax( const TBarSeries & aBars, const size_t aTimeDelta,
            TPriceSeries & aoMin, 
            TPriceSeries & aoMax );

bool _BollingerBands( const TPriceSeries & aPrices, const int aPeriod, const double aSigma,
    TPriceSeries & aoMin,
    TPriceSeries & aoMean,
    TPriceSeries & aoMax );

TPriceSeries _ChannelSize( const TBarSeries & aBars, const int aPeriod );

TPriceSeries _Stochastic( const TBarSeries & aBars, const int aPeriod );

#endif //BACKTESTER_INDICATORS_H
