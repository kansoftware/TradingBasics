/***
 * \file VolatilityBuffer.h
 * \author kan <kansoftware.ru>
 * \since 2020-06-22
 * \date 2026-10-30
 * \brief По заранее заполненному буферу рассчитывает min/max/Mean/err(TR)
 * \note Класс не потокобезопасен (non thread-safe)
 */

#ifndef VOLATILITYBUFFER_H
#define VOLATILITYBUFFER_H

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cassert>

#include "Comparers.h"

/**
 *  \class TVolatilityBuffer
 *  \brief Буфер фиксированного размера aCount со статистикой min/max/mean/err
 *
 *  После заполнения add() новые значения не принимает (возвращает false),
 *  статистика не меняется. T — тип min/max/mean/err (обычно double),
 *  Tin — тип добавляемых значений.
 *  \note Non thread-safe
 */
template <class T, class Tin=T> class TVolatilityBuffer {
    public:
        using TBufferType = std::vector<T>;

        /**
         *  \brief Конструктор
         *  \param aCount Размер буфера (>= 1)
         *  \note При aCount == 0 сработает assert
         */
        explicit TVolatilityBuffer( const size_t aCount ):
            fCount(aCount),fBuffer() {
                assert( fCount > 0 );
                fBuffer.reserve(fCount);
            }

        /**
         *  \brief Буфер заполнен
         *  \return true, если значений ровно aCount
         */
        bool isFill() const {return fBuffer.size() == fCount;}

        /**
         *  \brief Очищает буфер и сбрасывает всю статистику в NAN
         */
        void reset(){
            fBuffer.clear();
            fMax = NAN;
            fMin = NAN;
            fMean = NAN;
            fErr = NAN;
        }

        /**
         *  \brief Добавляет значение; при заполнении буфера рассчитывает err
         *  \param aValue Новое значение
         *  \return true, если буфер только что заполнился; false, если был
         *  заполнен ранее (значение отброшено) или ещё не заполнился
         *  \note err — sqrt(сум.кв.откл / (n-1.5)) / sqrt(n); при n < 2 err = NAN
         */
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
                    if( lc < 2.0 ){
                        fErr = NAN;
                    } else {
                        double lsd = 0.0;
                        for (auto& lv : fBuffer) {
                            lsd += pow( lMean - lv, 2.0 );
                        }

                        lsd /= (lc-1.5);
                        fErr = sqrt(lsd) / sqrt(lc);
                    }
                }

                return lisfill;
            }

            return false;
        }

        /** \brief Максимум добавленных значений (NAN до первого add) */
        T getMax()const{return fMax;}
        /** \brief Минимум добавленных значений (NAN до первого add) */
        T getMin()const{return fMin;}
        /**
         *  \brief Среднее значение
         *  \return Сумма значений / aCount; NAN, пока буфер пуст. До заполнения
         *  буфера сумма неполная — среднее занижено
         */
        T getMean()const{return fMean/static_cast<T>(fCount);}
        /** \brief Стандартная ошибка среднего (NAN до заполнения или при n < 2) */
        T getErr()const{return fErr;}

    private:
        const size_t fCount;
        TBufferType fBuffer;
        T fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN;
};

#endif //VOLATILITYBUFFER_H
