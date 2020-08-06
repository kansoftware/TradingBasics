/* 
 * File:   VolatilityBarBuffer.h
 * Author: kan
 *
 * Created on 2020-06-23
 * \date 2020-06-23
 * По заранее заполненному буферу рассчитывает min/max/ATR/err(TR), полный аналог VolatilityBuffer.h
 */

#ifndef VOLATILITYBARBUFFER_H
#define VOLATILITYBARBUFFER_H

#include <cmath>
#include <vector>
#include <set>
#include <deque>

#include "BasisOfStrategy.h"

class TVolatilityBarBuffer {
    public:
        using TBufferType = std::vector<double>;
        explicit TVolatilityBarBuffer( const size_t aCount ):
            fCount(aCount),fBuffer() {
                assert( fCount > 0 );
                fBuffer.reserve(fCount);
            }

        bool isFill() const {return fBuffer.size() == fCount;}
        void reset(){
            fBuffer.clear();
            fMax = NAN;
            fMin = NAN;
            fMean = NAN;
            fErr = NAN;
        }
        bool add(const TSimpleBar& aValue) {
            if( not isFill() ){
                const double lValue = aValue.High - aValue.Low;
                fBuffer.emplace_back(lValue);
                if(not std::isnan(fMax)){
                    fMax = IsGreat(aValue.High, fMax) ? aValue.High : fMax;
                    fMin = IsLess(aValue.Low, fMin) ? aValue.Low : fMin;
                    fMean += lValue;
                }else{
                    fMax = aValue.High;
                    fMin = aValue.Low;
                    fMean = lValue;
                }

                const bool lisfill = isFill();
                if(lisfill){
                    const double lc = static_cast<double>(fCount);
                    const double lMean = fMean / lc;
                    double lsd = 0.0;
                    for (auto& lv : fBuffer) {
                        lsd += pow( lMean - lv, 2 );
                    }
                    
                    lsd /= (lc-1.5);
                    fErr = sqrt(lsd) / sqrt(lc);
                }

                return lisfill;
            }

            return false;
        }

        double getMax()const{return fMax;}
        double getMin()const{return fMin;}
        double getMean()const{return fMean/static_cast<double>(fCount);}
        double getErr()const{return fErr;}

    private:
        const size_t fCount;
        TBufferType fBuffer;
        double fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN;
};

class TVolatilityBarRollBuffer {
    public:
        using TBufferType = std::deque<double>;
        explicit TVolatilityBarRollBuffer( const size_t aCount ):
            fCount(aCount){
            ;
        }
        bool isFill() const {return fBuffer.size() == fCount;}
        void reset(){
            fBuffer.clear();
            fMMBuffer.clear();

            fMean = 0;
            fErr = NAN;
        }
        double getMax()const{return *fMMBuffer.rbegin();}
        double getMin()const{return *fMMBuffer.begin();}
        double getMean()const{return fMean/static_cast<double>(fCount);}
        double getErr()const{return fErr;}

        bool add(const TSimpleBar& aValue) {
            const double lValue = aValue.High - aValue.Low;

            bool lisfill = isFill();
            if( lisfill ){
                const double lOldValue = fBuffer.front();
                fBuffer.pop_front();
                fBuffer.push_back(lValue);
                fMean += (lValue-lOldValue);
                fMMBuffer.erase( fMMBuffer.find( lOldValue ) );

            }else{
                fBuffer.push_back(lValue);
                if(not fBuffer.empty()){
                    fMean += lValue;
                }else{
                    fMean = lValue;
                }
                lisfill = isFill();
            }
            fMMBuffer.emplace(lValue);
            
            if(lisfill){
                const double lc = static_cast<double>(fCount);
                const double lMean = fMean / lc;
                double lsd = 0.0;
                for (auto& lv : fBuffer) {
                    lsd += pow( lMean - lv, 2 );
                }
                
                lsd /= (lc-1.5);
                fErr = sqrt(lsd) / sqrt(lc);
            }

            return lisfill;
        }

    private:
        TBufferType fBuffer;
        std::multiset<double> fMMBuffer;
        const size_t fCount;
        double fMean=0, fErr=NAN;
};

#endif // VOLATILITYBARBUFFER_H