/***
 * \file RubberBuffer.h
 * \author kan <kansoftware.ru>
 * \since 2020-09-16
 * \date 2026-10-30
 * \brief По безразмерному буферу рассчитывает min/max/Mean/err(TR)
 * \note Класс не потокобезопасен (non thread-safe)
 */

#ifndef RUBBERBUFFER_H
#define RUBBERBUFFER_H

#include <deque>
#include <stdexcept>
#include <cmath>
#include <cassert>

#include "Comparers.h"

/**
 *  \class TRubberBuffer
 *  \brief Накапливающий буфер без ограничения размера со статистикой min/max/mean/SD/err
 *
 *  Буфер считается заполненным (isFill) после aMinSize значений, но продолжает
 *  накапливать; статистика calcStats() считается по всем накопленным значениям.
 *  T — тип min/max/mean/SD/err (обычно double), Tin — тип добавляемых значений.
 *  \note Non thread-safe
 */
template <class T, class Tin=T> class TRubberBuffer {
    public:
        using TBufferType = std::deque<T>;

        /**
         *  \brief Конструктор
         *  \param aMinSize Число значений, после которого буфер считается заполненным
         *  \note При aMinSize == 0 сработает assert; в Release (NDEBUG) assert
         *  исчезает и isFill() всегда true — ноль заполняйте значением >= 1
         */
        explicit TRubberBuffer( const size_t aMinSize ):
            fMinSize(aMinSize),fBuffer() {
                assert( fMinSize > 0 );
            }

        /**
         *  \brief Буфер набрал минимальный размер
         *  \return true, если значений не меньше aMinSize
         */
        bool isFill() const {return fBuffer.size() >= fMinSize;}

        /**
         *  \brief Очищает буфер и сбрасывает всю статистику в NAN
         */
        void reset(){
            fBuffer.clear();
            fMax = NAN;
            fMin = NAN;
            fMean = NAN;
            fErr = NAN;
            fSD = NAN;
        }

        /**
         *  \brief Добавляет значение и обновляет min/max/сумму для mean
         *  \param aValue Новое значение
         */
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

        /**
         *  \brief Рассчитывает SD и err по всем накопленным значениям
         *  \return true, если буфер заполнен (статистика рассчитана); иначе false
         *  \note SD — несмещённая оценка СКО по ГОСТ Р 8.736-2011 (деление на
         *  n-1); err — sqrt(сум.кв.откл / (n-1.5)) / sqrt(n). При n < 2 обе
         *  величины равны NAN; mean остаётся честным (см. getMean)
         */
        bool calcStats(){
            if(isFill()){
                const T lc = static_cast<T>(fBuffer.size());
                const T lMean = fMean / lc;
                if( lc < 2.0 ){
                    fSD = NAN;
                    fErr = NAN;
                    return true;
                }
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

        /** \brief Максимум накопленных значений (NAN до первого add) */
        T getMax()const{return fMax;}
        /** \brief Минимум накопленных значений (NAN до первого add) */
        T getMin()const{return fMin;}
        /**
         *  \brief Среднее значение
         *  \return Сумма / число значений; NAN, если буфер пуст
         */
        T getMean()const{
            return isFill() ? fMean/static_cast<T>(fBuffer.size()) : NAN;
        }
        /** \brief Стандартная ошибка среднего (NAN до calcStats или при n < 2) */
        T getErr()const{return fErr;}
        /** \brief Несмещённое СКО по ГОСТ Р 8.736-2011 (NAN до calcStats или при n < 2) */
        T getSD()const{return fSD;}

    private:
        const size_t fMinSize;
        TBufferType fBuffer;
        T fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN, fSD=NAN;
};

#endif //RUBBERBUFFER_H
