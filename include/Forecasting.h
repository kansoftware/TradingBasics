/* 
 * File:   Forecasting.h
 * Author: kan <kansoftware.ru>
 *
 * Created on January 23, 2017, 1:58 PM
 */

#ifndef FORECASTING_H
#define FORECASTING_H

#include "CIndicatorES.h"

struct TForecastResult{
    double lower;
    double forecast;
    double upper;
} ;

// To calc confidence intervals in R (err=5%,n=100) do :  qt( 1.0 - 0.05 / 2.0, 100 )
// Confidence intervals( 90% ) = 1.65
// Confidence intervals( 80% ) = 1.290075
// TForecastModelParams aioParams;
// aioParams.SetDefault( aData[ 0 ] );
TForecastResult Forecast( const TDoubles &aData, TForecastModelParams & aioParams, const double aConfidenceIntervals = 1.290075 );

#endif /* FORECASTING_H */
