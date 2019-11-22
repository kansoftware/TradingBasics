/**
 * \file RollRange.h
 * \brief Модуль реализующий перекатывающиеся индикатор Min-Max Range
 * \since 2016-10-10
 * \date 2019-11-13
 * Модуль заимствован с https://bitbucket.org/quanttools/quanttools
 * (+) RollRange_with_tollerance [kan@kansoftware.ru]
 */

#ifndef ROLLRANGE_H
#define ROLLRANGE_H

#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <algorithm>

#include "DelphisRound.h"

struct Range {
    double min;
    double max;
    double quantile;
};

class RollRange {
    private:
        Range range;
        const size_t n;
        const double p;
        std::queue< double > window;
        std::multiset< double > windowSorted;

        std::vector< double > minHistory;
        std::vector< double > maxHistory;
        std::vector< double > quantileHistory;
        
        const double fBadValue;

    public:

        RollRange( const size_t aN, const double aP = 1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            p( aP ),
            fBadValue( aBadValue ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (p < 0.0 or p > 1.0) throw std::invalid_argument("p must be in [0,1]");
        }

        void Add( const double value ) {

            window.push(value);
            windowSorted.insert(value);

            if (window.size() > n) {

                windowSorted.erase(windowSorted.find(window.front()));
                window.pop();

            }
            range.min = *windowSorted.begin();
            range.max = *std::prev(windowSorted.end());
            if( IsFormed() ) range.quantile = ( p >= 0.5 ) ?
                    *std::next( windowSorted.rbegin(), Trunc((1.0 - p) * ToDouble(n) )  ) 
                    :
                    *std::next( windowSorted.begin(), Trunc(p * ToDouble(n) ) );

            IsFormed() ? minHistory.push_back(range.min) : minHistory.push_back( fBadValue );
            IsFormed() ? maxHistory.push_back(range.max) : maxHistory.push_back( fBadValue );
            IsFormed() ? quantileHistory.push_back(range.quantile) : quantileHistory.push_back( fBadValue );

        }

        bool IsFormed() const {
            return window.size() == n;
        }

        Range GetValue() const {
            return range;
        }

        std::vector< double > GetMinHistory() const {
            return minHistory;
        }

        std::vector< double > GetMaxHistory() const {
            return maxHistory;
        }

        std::vector< double > GetQuantileHistory() const {
            return quantileHistory;
        }

        void Reset() {
            window={};
            windowSorted.clear();
        }

};

class RollRange_with_tollerance {
    private:
        Range range;
        const size_t n;
        const size_t fMinTouch;
        const double fTollerance;
        std::queue< double > window;
        std::multiset< double > windowSorted;
        const double fBadValue;

    public:

        RollRange_with_tollerance( const size_t aN, const size_t aMinTouch=1, const double aTollerance = -1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            fMinTouch(aMinTouch),
            fTollerance( aTollerance ),
            fBadValue( aBadValue ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (fTollerance < 0.0 ) throw std::invalid_argument("fTollerance must be >= 0.0");
        }

        void Add( const double value ) {

            window.push(value);
            windowSorted.insert(value);

            if (window.size() > n) {
                windowSorted.erase(windowSorted.find(window.front()));
                window.pop();
            }
            
            auto it = std::lower_bound(windowSorted.begin(), windowSorted.end(), *windowSorted.begin()+fTollerance );
            range.min = ( static_cast<size_t>(std::distance( windowSorted.begin(),it)) >= fMinTouch ) ? *it : fBadValue;
            
            auto itr = std::upper_bound(windowSorted.begin(), windowSorted.end(), *windowSorted.rbegin()-fTollerance );
            range.max = ( static_cast<size_t>(std::distance(itr, windowSorted.end())) >= fMinTouch ) ? *itr : fBadValue;
        }

        bool IsFormed() const {
            return window.size() == n;
        }

        Range GetValue() const {
            return range;
        }

        void Reset() {
            window={};
            windowSorted.clear();
        }
};

#endif //ROLLRANGE_H
