/**
 * \file Indicators.h
 * \brief Модуль реализующий индикаторы для одноногих стратегий
 * \author kan <kansoftware.ru>
 * \since 2015-11-10
 * \date 2017-07-21
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

TPriceSeries _SimpleMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _ExponentMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _SmoothedMA( const TPriceSeries & aPrices, const int aPeriod, const size_t aLag = 0 );
TPriceSeries _KAMA( const TPriceSeries & aPrices, const int aPeriod, const double aCoeff = 2.0, const double aFastPeriod = 2.0, const double aSlowPeriod = 30.0, const size_t aLag = 0 );

const size_t gSARStabilizationDelay = 200 ;
TPriceSeries _ParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );
TPriceSeries _IntradayParabolicSar( const TBarSeries & aBars, const double aAf, const double aMaxAf );
TPriceSeries _MACD( const TPriceSeries & aPrices, const int aFastPeriod, const int aSlowPeriod, const int aSmoothPeriod );
TPriceSeries _ZigZag( const TBarSeries & aBars, const double aGap );

bool _Forecasting( const TPriceSeries & aPrices, const size_t aForecastPeriod, const double aConfidenceIntervals, 
                    TPriceSeries & aoUpperBorder, 
                    TPriceSeries & aoForecast, 
                    TPriceSeries & aoLowerBorder );

bool _RollMinMax( const TBarSeries & aBars, const int aPeriod, 
            TPriceSeries & aoMin, 
            TPriceSeries & aoMax );

#endif //BACKTESTER_INDICATORS_H
