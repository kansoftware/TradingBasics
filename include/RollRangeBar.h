/**
 * \file RollRangeBar.h
 * \brief Модуль реализующий перекатывающиеся индикатор Min-Max Range для баров
 * \author kan <kansoftware.ru>
 * \since 2020-10-21
 * \date 2020-12-08
 * \note Non thread-safe
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

class TRollRangeBar {
    public:
        using TRange = std::pair<double, double>;//min max

        TRollRangeBar( const size_t aN, const double aBadValue = -333333.33 ) :
            n( aN ),
            fBadValue( std::make_pair(aBadValue,aBadValue) ),
            range( fBadValue),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");

            History.reserve( fArrayReserve );
        }

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

        bool IsFormed() const {
            return window.size() == n;
        }

        TRange GetValue() const {
            return range;
        }

        std::vector< TRange > GetHistory() const {
            return History;
        }

        void Reset() {
            window={};
            windowSortedMax.clear();
            windowSortedMin.clear();

            History.clear();
            History.reserve( fArrayReserve );

            fCached_m = false;
            fCached_e = false;
        }

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
            lsd /= (lc-1.5);
            
            fErr = sqrt(lsd) / sqrt(lc);
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