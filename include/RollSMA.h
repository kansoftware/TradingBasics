/***
 * File:   RollSMA.h
 * Author: kan
 *
 * Created on November 13, 2019, 12:55 PM
 * \date 2026-10-30
 * The idea is from https://bitbucket.org/quanttools/quanttools/src/master/inst/include/Indicators/Sma.h
 * \note Класс не потокобезопасен (non thread-safe)
 */

#ifndef ROLLSMA_H
#define ROLLSMA_H

#include <queue>
#include <stdexcept>
#include <cmath>

/**
 *  \class RollSMA
 *  \brief Скользящее среднее (SMA) по окну из n значений, пересчёт за O(1) на Add
 */
class RollSMA {
    private:
        double fSum;
        const std::size_t fN;
        std::queue< double > fWindow;

    public:
        /**
         *  \brief Конструктор
         *  \param n Размер окна (>= 1)
         *  \throw std::invalid_argument При n < 1
         */
        RollSMA( const int n ) :
            fSum( 0.0 ),
            fN( static_cast<size_t>(n) ) {
                if (n < 1) throw std::invalid_argument("n must be greater than 0");
            }

        /**
         *  \brief Добавляет значение в окно (лишнее вытесняется, сумма корректируется)
         *  \param value Новое значение
         */
        void Add( double value ) {

            fSum += value;
            fWindow.push(value);

            if (fWindow.size() > fN) {
                fSum -= fWindow.front();
                fWindow.pop();
            }
        }

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n значений
         */
        bool IsFormed() const {
            return fWindow.size() == fN;
        }

        /**
         *  \brief Текущее скользящее среднее
         *  \return Среднее по окну; NAN, если окно ещё не сформировано
         */
        double GetValue() const {
            return ((fWindow.size() == fN) ? (fSum / static_cast<double>(fN)) : NAN);
        }

        /**
         *  \brief Очищает окно и накопленную сумму (индикатор начинает формироваться заново)
         */
        void Reset() {
            fSum = 0.0;
            fWindow = {};
        }
};

#endif /* ROLLSMA_H */
