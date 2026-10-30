/**
 * \file VolatilityBarBuffer.h
 * \author kan <kansoftware.ru>
 * \since 2020-06-23
 * \date 2026-10-30
 * \brief По заранее заполненному буферу рассчитывает min/max/ATR/err(TR), полный аналог VolatilityBuffer.h
 * \note Классы не потокобезопасны (non thread-safe)
 */

#ifndef VOLATILITYBARBUFFER_H
#define VOLATILITYBARBUFFER_H

#include <cmath>
#include <vector>
#include <set>
#include <deque>

#include "BasisOfStrategy.h"

/**
 *  \class TVolatilityBarBuffer
 *  \brief Буфер фиксированного размера aCount для баров: диапазоны High-Low,
 *  min по Low, max по High, mean/err по диапазонам
 *
 *  После заполнения add() новые бары не принимает (возвращает false),
 *  статистика не меняется.
 *  \note Non thread-safe
 */
class TVolatilityBarBuffer {
    public:
        using TBufferType = std::vector<double>;

        /**
         *  \brief Конструктор
         *  \param aCount Размер буфера в барах (>= 1)
         *  \note При aCount == 0 сработает assert
         */
        explicit TVolatilityBarBuffer( const size_t aCount ):
            fCount(aCount),fBuffer() {
                assert( fCount > 0 );
                fBuffer.reserve(fCount);
            }

        /**
         *  \brief Буфер заполнен
         *  \return true, если баров ровно aCount
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
         *  \brief Добавляет бар; при заполнении буфера рассчитывает err
         *  \param aValue Новый бар
         *  \return true, если буфер только что заполнился; false, если был
         *  заполнен ранее (бар отброшен) или ещё не заполнился
         *  \note err — sqrt(сум.кв.откл / (n-1.5)) / sqrt(n) по диапазонам
         *  High-Low; при n < 2 err = NAN
         */
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
                    if( lc < 2.0 ){
                        fErr = NAN;
                    } else {
                        double lsd = 0.0;
                        for (auto& lv : fBuffer) {
                            lsd += pow( lMean - lv, 2 );
                        }

                        lsd /= (lc-1.5);
                        fErr = sqrt(lsd) / sqrt(lc);
                    }
                }

                return lisfill;
            }

            return false;
        }

        /** \brief Максимум High добавленных баров (NAN до первого add) */
        double getMax()const{return fMax;}
        /** \brief Минимум Low добавленных баров (NAN до первого add) */
        double getMin()const{return fMin;}
        /**
         *  \brief Средний диапазон High-Low
         *  \return Сумма диапазонов / aCount; NAN, пока буфер пуст. До заполнения
         *  буфера сумма неполная — среднее занижено
         */
        double getMean()const{return fMean/static_cast<double>(fCount);}
        /** \brief Стандартная ошибка среднего (NAN до заполнения или при n < 2) */
        double getErr()const{return fErr;}

    private:
        const size_t fCount;
        TBufferType fBuffer;
        double fMax=NAN, fMin=NAN, fMean=NAN, fErr=NAN;
};

/**
 *  \class TVolatilityBarRollBuffer
 *  \brief Скользящий буфер баров размера aCount: диапазоны High-Low с вытеснением,
 *  min/max по диапазонам (multiset), mean/err по диапазонам окна
 *
 *  После заполнения каждый add() вытесняет самый старый диапазон; mean
 *  пересчитывается инкрементально, min/max поддерживаются multiset'ом.
 *  \note Non thread-safe
 */
class TVolatilityBarRollBuffer {
    public:
        using TBufferType = std::deque<double>;

        /**
         *  \brief Конструктор
         *  \param aCount Размер окна в барах (>= 1)
         */
        explicit TVolatilityBarRollBuffer( const size_t aCount ):
            fCount(aCount){
            ;
        }

        /**
         *  \brief Окно заполнено
         *  \return true, если диапазонов ровно aCount
         */
        bool isFill() const {return fBuffer.size() == fCount;}

        /**
         *  \brief Очищает окно и сбрасывает статистику
         */
        void reset(){
            fBuffer.clear();
            fMMBuffer.clear();

            fMean = 0;
            fErr = NAN;
        }

        /** \brief Максимум диапазонов окна (NAN на пустом окне) */
        double getMax()const{return fMMBuffer.empty() ? NAN : *fMMBuffer.rbegin();}
        /** \brief Минимум диапазонов окна (NAN на пустом окне) */
        double getMin()const{return fMMBuffer.empty() ? NAN : *fMMBuffer.begin();}

        /**
         *  \brief Средний диапазон окна
         *  \return Сумма диапазонов / aCount; 0 на пустом/сброшенном окне
         */
        double getMean()const{return fMean/static_cast<double>(fCount);}
        /** \brief Стандартная ошибка среднего (NAN до заполнения или при n < 2) */
        double getErr()const{return fErr;}

        /**
         *  \brief Добавляет бар (при заполненном окне вытесняет самый старый)
         *  \param aValue Новый бар; диапазон High-Low добавляется в окно
         *  \return true, если окно заполнено (после добавления)
         *  \note err — sqrt(сум.кв.откл / (n-1.5)) / sqrt(n); при n < 2 err = NAN
         */
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
                if( fBuffer.empty() ){
                    fMean = lValue;
                }else{
                    fMean += lValue;
                }
                fBuffer.push_back(lValue);
                lisfill = isFill();
            }
            fMMBuffer.emplace(lValue);

            if(lisfill){
                const double lc = static_cast<double>(fCount);
                const double lMean = fMean / lc;
                if( lc < 2.0 ){
                    fErr = NAN;
                } else {
                    double lsd = 0.0;
                    for (auto& lv : fBuffer) {
                        lsd += pow( lMean - lv, 2 );
                    }

                    lsd /= (lc-1.5);
                    fErr = sqrt(lsd) / sqrt(lc);
                }
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