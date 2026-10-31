/**
 * \file RollRangeBar.h
 * \brief Модуль реализующий перекатывающиеся индикатор Min-Max Range для баров
 * \author kan <kansoftware.ru>
 * \since 2020-10-21
 * \date 2026-10-30
 * \note Класс не потокобезопасен (non thread-safe)
 */

#ifndef ROLLRANGEBAR_H
#define ROLLRANGEBAR_H

#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <algorithm>
#include <utility>

#include "DelphisRound.h"
#include "BasisOfStrategy.h"

/**
 *  \class TRollRangeBar
 *  \brief Скользящий по окну из aN баров индикатор min/max: минимум по Low, максимум по High
 *
 *  В историю пишется по одной записи {min окна, max окна} на каждый Add
 *  (до формирования окна — {aBadValue, aBadValue}); история не скользит и
 *  накапливается. getMean/getErr считают статистику по валидным записям
 *  истории (диапазон окна: second - first).
 *  \note Non thread-safe
 */
class TRollRangeBar {
    public:
        using TRange = std::pair<double, double>;//min max

        /**
         *  \brief Конструктор
         *  \param aN Размер окна в барах (>= 1)
         *  \param aBadValue Значение, записываемое в историю до формирования окна
         *  \throw std::invalid_argument При aN == 0
         */
        TRollRangeBar( const size_t aN, const double aBadValue = -333333.33 ) :
            n( aN ),
            fBadValue( std::make_pair(aBadValue,aBadValue) ),
            range( fBadValue),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");

            History.reserve( fArrayReserve );
        }

        /**
         *  \brief Добавляет бар в окно и пересчитывает min/max
         *  \param value Новый бар
         *  \note До формирования окна (IsFormed) в историю пишется {aBadValue, aBadValue}
         */
        void Add( const TSimpleBar& value ) {
            window.push( std::make_pair(value.Low, value.High) );
            windowSortedMin.insert(value.Low);
            windowSortedMax.insert(value.High);

            if( window.size() > n ){
                windowSortedMin.erase( windowSortedMin.find(window.front().first) );
                windowSortedMax.erase( windowSortedMax.find(window.front().second) );

                window.pop();
            }

            range = std::make_pair( *windowSortedMin.begin(), *windowSortedMax.rbegin() );

            if( IsFormed() ){
                History.push_back( range );
            } else {
                History.push_back( fBadValue );
            }

            fCached_m = false;
            fCached_e = false;
        }

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n баров
         */
        bool IsFormed() const {
            return window.size() == n;
        }

        /**
         *  \brief Текущие min/max окна
         *  \return Пара {минимум Low, максимум High}
         */
        TRange GetValue() const {
            return range;
        }

        /**
         *  \brief История пар {min окна, max окна} (по одной на каждый Add)
         *  \return Вектор; до формирования окна элементы равны {aBadValue, aBadValue}
         */
        std::vector< TRange > GetHistory() const {
            return History;
        }

        /**
         *  \brief Очищает окно и историю (индикатор начинает формироваться заново)
         */
        void Reset() {
            window={};
            windowSortedMax.clear();
            windowSortedMin.clear();

            History.clear();
            History.reserve( fArrayReserve );

            fCached_m = false;
            fCached_e = false;
        }

        /**
         *  \brief Средний диапазон окна по валидным записям истории
         *  \return Среднее (max - min) по записям, отличным от bad-value; NAN,
         *  если окно ещё не сформировано
         */
        double getMean() const {
            if( not IsFormed() ){
                return NAN;
            }
            
            if(fCached_m){
                return fMean;
            }

            double lmv = 0.0;
            size_t ln = 0;
            for( const auto& lv : History ){
                if( lv == fBadValue) continue;
                lmv += (lv.second - lv.first);
                ln++;
            }

            fMean = lmv / static_cast<double>(ln);
            fCached_m = true;
            return fMean;
        }

        /**
         *  \brief Стандартная ошибка среднего диапазона окна по истории
         *  \return sqrt(сум.кв.откл / (ln - 1.5)) / sqrt(ln) по валидным записям;
         *  NAN, если окно не сформировано или валидных записей меньше 2
         */
        double getErr() const {
            const double lMean = getMean();
            if( std::isnan(lMean) ){
                return NAN;
            }

            if(fCached_e){
                return fErr;
            }

            double lsd = 0.0;
            size_t ln = 0;
            for( const auto& lv : History ) {
                if( lv == fBadValue) continue;
                const double ld = (lv.second - lv.first);
                lsd += pow( lMean - ld, 2 );
                ln++;
            }
            const double lc = static_cast<double>(ln);
            if( lc < 2.0 ){
                fErr = NAN;
            } else {
                lsd /= (lc-1.5);

                fErr = sqrt(lsd) / sqrt(lc);
            }
            fCached_e = true;
            return fErr;
        }

    private:
        const size_t n;

        std::queue< TRange > window;
        std::multiset< double > windowSortedMin;
        std::multiset< double > windowSortedMax;

        std::vector< TRange > History;
        
        const TRange fBadValue;
        TRange range;
        const size_t fArrayReserve = 1000;
        
        mutable bool fCached_m = false;
        mutable bool fCached_e = false;
        mutable double fMean = NAN;
        mutable double fErr = NAN;
};

#endif //ROLLRANGEBAR_H