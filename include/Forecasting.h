/**
 * \file Forecasting.h
 * \author kan <kansoftware.ru>
 * \since 2017-01-23
 */

#ifndef FORECASTING_H
#define FORECASTING_H

#include "CIndicatorES.h"

/**
 *  \struct TForecastResult
 *  \brief Результат прогноза: граница/точечная оценка/граница доверительного интервала
 */
struct TForecastResult{
    double lower;       /*!< Нижняя граница доверительного интервала */
    double forecast;    /*!< Точечный прогноз на шаг вперёд */
    double upper;       /*!< Верхняя граница доверительного интервала */
} ;

// To calc confidence intervals in R (err=5%,n=100) do :  qt( 1.0 - 0.05 / 2.0, 100 )
// Confidence intervals( 90% ) = 1.65
// Confidence intervals( 80% ) = 1.290075

/**
 *  \brief Прогноз следующего значения ряда моделью Хольта с демпфированным трендом
 *
 *  Параметры модели подбираются CIndicatorES::CalcPar от стартовой точки
 *  aioParams (обычно aioParams.SetDefault(aData[0])). После успешного
 *  фитирования настроенные параметры (уровень S, тренд t, Alpha, Gamma, Phi)
 *  записываются обратно в aioParams — повторный вызов продолжает с них.
 *
 *  Точечный прогноз — Fore = S + Phi*t (уровень плюс демпфированный тренд на
 *  шаг вперёд); доверительный интервал — Fore ± aConfidenceIntervals*sqrt(var),
 *  где var — средний квадрат ошибок прогноза по истории (сумма e^2 / (NDat-1)).
 *
 *  \param aData Входной ряд (не менее 2 точек)
 *  \param aioParams Стартовые параметры; на выходе — настроенные параметры модели
 *  \param aConfidenceIntervals Квантиль Стьюдента для интервала (80% -> 1.290075, 90% -> 1.65)
 *  \return lower <= forecast <= upper; при неудаче фитирования или NDat < 2 —
 *          все три поля = GetBadPrice() (bad-конвенция модуля Prices)
 */
TForecastResult Forecast( const TDoubles &aData, TForecastModelParams & aioParams, const double aConfidenceIntervals = 1.290075 );

#endif /* FORECASTING_H */
