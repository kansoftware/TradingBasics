/*
 * File:   CIndicatorES.h
 * Author: https://www.mql5.com/ru/articles/318
 *
 * Created on January 20, 2017, 4:14 PM
 */

#ifndef CINDICATORES_H
#define CINDICATORES_H

#include "PowellsMethod.h"

/**
 *  \struct TForecastModelParams
 *  \brief Параметры модели Хольта с демпфированным трендом (экспоненциальное сглаживание)
 *
 *  Рекуррентные соотношения модели (см. CIndicatorES::func):
 *    e = Dat[i] - (S + Phi*t);  S' = S + Phi*t + Alpha*e;  t' = Phi*t + Gamma*e.
 */
struct TForecastModelParams{
    double S;       /*!< Уровень (сглаженное значение ряда) */
    double t;       /*!< Тренд (темп прироста уровня за шаг) */
    double Alpha;   /*!< Коэффициент сглаживания уровня [0.05..0.95] */
    double Gamma;   /*!< Коэффициент сглаживания тренда [0.05..0.95] */
    double Phi;     /*!< Коэффициент демпфирования тренда [0.05..1.0] */

    void SetDefault( const double aS ) {
        S = aS;
        t = 0.0;
        Alpha = 0.5;
        Gamma = 0.5;
        Phi = 0.5;
    }
};

//-----------------------------------------------------------------------------------
// Class CIndicatorES
//-----------------------------------------------------------------------------------
/**
 *  \class CIndicatorES
 *  \brief Подбор параметров модели Хольта (демпфированный тренд) методом Пауэлла
 *
 *  CalcPar() запускает Optimize() от стартовых параметров aParams и ищет
 *  минимум целевой функции func() — суммы квадратов ошибок модели с логарифмическим
 *  штрафом за выход Alpha/Gamma/Phi за допустимые диапазоны (штраф 200x за
 *  единицу выхода: k += (отклонение)*200 внутри log — см. func). Уровень S и
 *  тренд t не ограничиваются.
 *
 *  Диапазоны параметров: Alpha, Gamma in [0.05, 0.95], Phi in [0.05, 1.0].
 *  Внутри func() вышедшие за диапазон значения подрезаются до границы,
 *  а в целевую функцию добавляется штраф — поэтому итоговые параметры могут
 *  быть на границе диапазона (в пределах точности сходимости FtolPowell).
 */
class CIndicatorES : public PowellsMethod {
    protected:
        TDoubles Dat;                                              /*!< Копия входного ряда */
        size_t Dlen;                                               /*!< Длина входного ряда */
        TDoubles Par;                                              /*!< Параметры [S, t, Alpha, Gamma, Phi] */
    public:
        CIndicatorES(): PowellsMethod(), Par( 5.0 ) { }
        bool CalcPar( const TDoubles &dat, const TForecastModelParams &aParams );   /*!< \brief Подобрать параметры ряда dat от старта aParams */
        double GetPar( const int n );                              /*!< \brief Параметр по индексу: 0=S, 1=t, 2=Alpha, 3=Gamma, 4=Phi; вне [0..4] -> 0.0 */

    protected:
        /// \brief Целевая функция: Dlen*log(k1*k2*k3*sse), где sse — сумма
        /// квадратов ошибок модели, k1/k2/k3 >= 1 — штрафы 200x за выход
        /// Alpha/Gamma/Phi за диапазоны. Protected для прямого гтеста штрафа.
        double func( const TDoubles &p ) override;
};

#endif /* CINDICATORES_H */
