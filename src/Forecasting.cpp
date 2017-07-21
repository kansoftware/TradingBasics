// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   Forecasting.cpp
 * Author: victorg(mq5), kan(c++)
 * 
 * Created on January 23, 2017, 1:58 PM
 */

#include <cmath>
#include <cstddef>

#include "Prices.h"
#include "DelphisRound.h"

#include "Forecasting.h"

TForecastResult Forecast(const TDoubles &aData, TForecastModelParams & aioParams, const double aConfidenceIntervals) {
    const size_t NDat = aData.size();

    CIndicatorES Es;
    if (not Es.CalcPar(aData, aioParams)) {
        return TForecastResult{ GetBadPrice(), GetBadPrice(), GetBadPrice()};
    }

    double s = Es.GetPar(0);
    double t = Es.GetPar(1);
    const double alp = Es.GetPar(2);
    const double gam = Es.GetPar(3);
    const double phi = Es.GetPar(4);
    double f = (s + phi * t);
    double var = 0.0;

    for (size_t i = 0; i < NDat; i++) { // History
        const double e = aData[i] - f;
        var += e*e;
        const double a1 = alp*e;
        const double a2 = phi*t;
        s = s + a2 + a1;
        t = a2 + gam*a1;
        f = (s + phi * t);
        //        Hist[start + i] = f;
    }
    var /= ToDouble(NDat - 1);
    double a1 = 1.0;
    double a2 = 0.0;
    double a3 = 1.0;

    a1 = a1*phi;
    a2 += a1;

    const double Fore = s + a2*t; // Forecast

    const double ci = aConfidenceIntervals * sqrt(var * a3);
    const double Conf1 = Fore + ci;
    const double Conf2 = Fore - ci;

    //    a = alp * (1 + a2 * gam);
    //    a3 += a*a;
    TForecastResult lResult;
    lResult.upper = Conf1;
    lResult.forecast = Fore;
    lResult.lower = Conf2;

    aioParams.S = s;
    aioParams.t = t;
    aioParams.Alpha = alp;
    aioParams.Gamma = gam;
    aioParams.Phi = phi;

    return lResult;
}
