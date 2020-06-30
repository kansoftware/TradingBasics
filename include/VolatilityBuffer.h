/* 
 * File:   VolatilityBuffer.h
 * Author: kan
 *
 * Created on 2020-06-22
 * \date 2020-06-22
 * По заранее заполненному буферу рассчитывает min/max/ATR/err(TR)
 */

#ifndef VOLATILITYBUFFER_H
#define VOLATILITYBUFFER_H

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cassert>

template <class T, class Tin=T> class TVolatilityBuffer {
    public:
        using TBufferType = std::vector<T>;
        explicit TVolatilityBuffer( const size_t aCount ):
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
        virtual bool add(const Tin& aValue){
            if( not isFill() ){
                fBuffer.emplace_back(aValue);
                if(not std::isnan(fMax)){
                    fMax = IsGreat(aValue, fMax) ? aValue : fMax;
                    fMin = IsLess(aValue, fMin) ? aValue : fMin;
                    fMean += aValue;
                }else{
                    fMax = aValue;
                    fMin = aValue;
                    fMean = aValue;
                }

                const bool lisfill = isFill();
                if(lisfill){
                    const T lc = static_cast<T>(fCount);
                    const T lMean = fMean / lc;
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

        T getMax()const{return fMax;}
        T getMin()const{return fMin;}
        T getMean()const{return fMean/static_cast<T>(fCount);}
        T getErr()const{return fErr;}

    private:
        const size_t fCount;
        TBufferType fBuffer;
        T fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN;
};

#endif //VOLATILITYBUFFER_H
