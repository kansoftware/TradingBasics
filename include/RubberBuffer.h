/*** 
 * \file RubberBuffer.h
 * \author kan <kansoftware.ru>
 * \since 2020-09-16
 * \date 2020-09-16
 * \brief По безразмерному буферу рассчитывает min/max/Mean/err(TR)
 */

#ifndef RUBBERBUFFER_H
#define RUBBERBUFFER_H

#include <deque>
#include <stdexcept>
#include <cmath>
#include <cassert>

template <class T, class Tin=T> class TRubberBuffer {
    public:
        using TBufferType = std::deque<T>;
        explicit TRubberBuffer( const size_t aMinSize ):
            fMinSize(aMinSize),fBuffer() {
                assert( fMinSize > 0 );
            }

        bool isFill() const {return fBuffer.size() >= fMinSize;}
        
        void reset(){
            fBuffer.clear();
            fMax = NAN;
            fMin = NAN;
            fMean = NAN;
            fErr = NAN;
            fSD = NAN;
        }
        
        void add(const Tin& aValue){
            fBuffer.push_back(aValue);
            if(not std::isnan(fMax)){
                fMax = IsGreat(aValue, fMax) ? aValue : fMax;
                fMin = IsLess(aValue, fMin) ? aValue : fMin;
                fMean += aValue;
            }else{
                fMax = aValue;
                fMin = aValue;
                fMean = aValue;
            }
        }

        bool calcStats(){
            if(isFill()){
                const T lc = static_cast<T>(fBuffer.size());
                const T lMean = fMean / lc;
                double lsd = 0.0;
                for (auto& lv : fBuffer) {
                    lsd += pow( lMean - lv, 2 );
                }
                fSD = sqrt( lsd / (lc-1) ); // ГОСТ Р 8.736-2011
                lsd /= (lc-1.5);
                fErr = sqrt(lsd) / sqrt(lc);
                
                return true;
            }

            return false;
        }

        T getMax()const{return fMax;}
        T getMin()const{return fMin;}
        T getMean()const{
            return isFill() ? fMean/static_cast<T>(fBuffer.size()) : NAN;
        }
        T getErr()const{return fErr;}
        T getSD()const{return fSD;}

    private:
        const size_t fMinSize;
        TBufferType fBuffer;
        T fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN, fSD=NAN;
};

#endif //RUBBERBUFFER_H
