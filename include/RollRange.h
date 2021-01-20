/**
 * \file RollRange.h
 * \brief Модуль реализующий перекатывающиеся индикатор Min-Max Range
 * \since 2016-10-10
 * \date 2021-01-20
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
#include <iterator>

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
        const bool backSearch;
        const int pN;
        std::queue< double > window;
        std::multiset< double > windowSorted;

        std::vector< double > minHistory;
        std::vector< double > maxHistory;
        std::vector< double > quantileHistory;
        
        const double fBadValue;
        const size_t fArrayReserve = 1000; 

    public:

        RollRange( const size_t aN, const double aP = 1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            backSearch(aP >= 0.5),
            pN( (backSearch ? Trunc((aP - 1.0) * ToDouble(n) ) : Trunc(aP * ToDouble(n) ) ) ),
            fBadValue( aBadValue ),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (aP < 0.0 or aP > 1.0) throw std::invalid_argument("p must be in [0,1]");
            

            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
            quantileHistory.reserve( fArrayReserve );
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

            if( IsFormed() ){
                auto it = backSearch ? windowSorted.cend() : windowSorted.cbegin();
                std::advance(it, pN);
                range.quantile = *it;

                minHistory.push_back( range.min );
                maxHistory.push_back( range.max );
                quantileHistory.push_back( range.quantile );
            } else {
                minHistory.push_back( fBadValue );
                maxHistory.push_back( fBadValue );
                quantileHistory.push_back( fBadValue );
            }
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

            minHistory.clear();
            maxHistory.clear();
            quantileHistory.clear();

            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
            quantileHistory.reserve( fArrayReserve );
        }

        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (lFirstValue!=0.0) ? (window.back() / lFirstValue - 1.0) : NAN;
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

class RollSeesaw {
    private:
        const size_t n;
        std::queue< double > window;

    public:
        RollSeesaw( const size_t aN ) :
            n( aN ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
        }

        void Add( const double value ) {
            window.push(value);
            if (window.size() > n) {
                window.pop();
            }
        }

        bool IsFormed() const {
            return window.size() == n;
        }

        void Reset() {
            window={};
        }

        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (lFirstValue!=0.0) ? (window.back() / lFirstValue - 1.0) : NAN;
        }
};

class RollRangeLazy {
    
    private:
        Range range;
        const size_t n;
        const bool backSearch;
        const int pN;
        std::queue< double > window;
        std::multiset< double > windowSorted;

        std::vector< double > minHistory;
        std::vector< double > maxHistory;
                
        const double fBadValue;
        const size_t fArrayReserve = 1000; 

    public:
        RollRangeLazy( const size_t aN, const double aP = 1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            backSearch(aP >= 0.5),
            pN( (backSearch ? Trunc((aP - 1.0) * ToDouble(n) ) : Trunc(aP * ToDouble(n) ) ) ),
            fBadValue( aBadValue ),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (aP < 0.0 or aP > 1.0) throw std::invalid_argument("p must be in [0,1]");
            
            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
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

            if( IsFormed() ){
                minHistory.push_back( range.min );
                maxHistory.push_back( range.max );
            } else {
                minHistory.push_back( fBadValue );
                maxHistory.push_back( fBadValue );
            }
        }

        bool IsFormed() const {
            return window.size() == n;
        }

        Range GetValue() {
            auto it = backSearch ? windowSorted.cend() : windowSorted.cbegin();
            std::advance(it, pN);
            range.quantile = *it;

            return range;
        }

        std::vector< double > GetMinHistory() const {
            return minHistory;
        }

        std::vector< double > GetMaxHistory() const {
            return maxHistory;
        }

        void Reset() {
            window={};
            windowSorted.clear();

            minHistory.clear();
            maxHistory.clear();

            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
        }

        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (lFirstValue!=0.0) ? (window.back() / lFirstValue - 1.0) : NAN;
        }
};

#endif //ROLLRANGE_H
