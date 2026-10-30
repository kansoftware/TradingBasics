/***
 * \file LiveSAR.h
 * \brief SAR расчитываемый шаг за шагом
 * \author kan <kansoftware.ru>
 * \since 2020-08-26
 * \date 2026-10-30
 * \note Класс не потокобезопасен (non thread-safe)
 */

#ifndef LIVESAR_H
#define LIVESAR_H

#include <queue>
#include <stdexcept>
#include <cmath>

#include "BasisOfStrategy.h"

/**
 *  \class TLiveSAR
 *  \brief Parabolic SAR (Wilder), рассчитываемый пошагово по мере прихода баров
 *
 *  Пошаговый аналог потоковой реализации _ParabolicSar (src/Indicators.cpp):
 *  на одинаковой серии баров значения совпадают бар-в-бар. Ускорительный
 *  фактор AF стартует с aAf и растёт на aAf при новом экстремуме тренда до
 *  aMaxAf; при развороте тренда SAR перескакивает на противоположный
 *  экстремум, AF сбрасывается в aAf.
 *  \note Non thread-safe
 */
class TLiveSAR {
    public:
        /**
         *  \brief Конструктор
         *  \param aStartAf Начальный шаг AF и шаг его роста (по умолчанию 0.02)
         *  \param aMaxAfLimit Максимальный AF (по умолчанию 0.2)
         */
        TLiveSAR(const double aStartAf=0.02, const double aMaxAfLimit=0.2):
            aAf(aStartAf),
            aMaxAf(aMaxAfLimit),
            lInitaAfValue(aStartAf),
            lAf(aStartAf){
            ;
        }

        /**
         *  \brief Добавляет бар и возвращает текущее значение SAR
         *  \param aBar Новый бар
         *  \return Значение SAR для этого бара
         *  \note Первый Add — warm-up: SAR ещё не рассчитан, возвращается
         *  Low первого бара (согласуется с _ParabolicSar: result[0] = bars[0].Low)
         */
        TPrice Add( const TSimpleBar& aBar ) {
            
            if( std::isnan(lSar) ){
                lSar = aBar.Low;
                lFuturesSar = aBar.High;
                lPrevBar = aBar;

                return lSar;
            }

            const TPrice lHigh = aBar.High;
            const TPrice lLow = aBar.Low;
            const double lPriorSar = lSar;

            lSar = (lFuturesSar - lPriorSar) * lAf + lPriorSar;
            
            if( lCurrentSide == TDealSide::Buy ) {

                if( IsGreat( lHigh, lFuturesSar ) ) {
                    lFuturesSar = lHigh;
                    if( IsLess(lAf, aMaxAf) ) {
                        lAf += aAf;
                    }
                }
                    
                const TPrice lPriorLow = lPrevBar.Low;
                const TPrice lLocalLow = std::min( lLow, lPriorLow );
                const double lCondidateSAR = std::min( lLocalLow, lSar );
                lSar = std::max( lPriorSar, lCondidateSAR );
                
                if( IsLess( lLow , lSar ) ) {
                    lAf = lInitaAfValue;
                    lCurrentSide = TDealSide::Sell;
                    lSar = lFuturesSar;
                    lFuturesSar = lLow;
                }
                    
                lPrevBar = aBar;
                return lSar;
            }

            if( lCurrentSide == TDealSide::Sell ) {
                    
                if( IsLess(lLow, lFuturesSar) ) {
                    lFuturesSar = lLow;
                    if( IsLess( lAf, aMaxAf) ) {
                        lAf += aAf;
                    }
                }

                const TPrice lPriorHigh = lPrevBar.High;
                const TPrice lLocalHigh= std::max( lHigh, lPriorHigh );
                const double lCondidateSAR = std::max( lLocalHigh, lSar );
                lSar = std::min( lPriorSar, lCondidateSAR );
                
                if( IsGreat(lHigh, lSar) ) {
                    lAf = lInitaAfValue;
                    lCurrentSide = TDealSide::Buy;
                    lSar = lFuturesSar;
                    lFuturesSar = lHigh;
                }
            }

            lPrevBar = aBar;

            return lSar;
        }

    private:
        TPrice lSar = NAN;
        TPrice lFuturesSar = NAN;
        const double aAf;
        const double aMaxAf;
        const double lInitaAfValue;
        double lAf;
        TDealSide lCurrentSide = TDealSide::Buy;
        TSimpleBar lPrevBar;
};

#endif //LIVESAR_H
