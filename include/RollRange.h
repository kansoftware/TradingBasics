/**
 * \file RollRange.h
 * \brief Модуль реализующий перекатывающиеся индикатор Min-Max Range
 * \since 2016-10-10
 * \date 2026-10-30
 * Модуль заимствован с https://bitbucket.org/quanttools/quanttools
 * (+) RollRange_with_tollerance [kan@kansoftware.ru]
 * \note Все классы модуля не потокобезопасны (non thread-safe)
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

/**
 *  \struct Range
 *  \brief Снимок состояния индикаторов range: min/max окна и p-квантиль
 */
struct Range {
    double min;         /*!< Минимум окна */
    double max;         /*!< Максимум окна */
    double quantile;    /*!< p-квантиль окна (не все индикаторы модуля её заполняют) */
};

/**
 *  \class RollRange
 *  \brief Скользящий по окну размера n индикатор min/max и p-квантиль
 *
 *  Окно на std::queue + отсортированная копия на std::multiset: Add выполняется
 *  за O(log n), значения вытесняются строго по одному (корректно при дубликатах).
 *  \note Non thread-safe
 */
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

        /**
         *  \brief Конструктор
         *  \param aN Размер окна (>= 1)
         *  \param aP Порядок квантили из [0, 1], по умолчанию 1.0
         *  \param aBadValue Значение, записываемое в историю до формирования окна
         *  \throw std::invalid_argument При aN == 0 или aP вне [0, 1]
         *  \note При aP == 1.0 (дефолт) квантиль равна максимуму окна: pN
         *  клампится к -1 (advance от cend()), что согласуется с пределом p -> 1
         *  и с поведением при p = 0.85/0.9
         */
        RollRange( const size_t aN, const double aP = 1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            backSearch(aP >= 0.5),
            pN( (backSearch ? std::min(Trunc((aP - 1.0) * ToDouble(n) ), -1) : Trunc(aP * ToDouble(n) ) ) ),
            fBadValue( aBadValue ),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (aP < 0.0 or aP > 1.0) throw std::invalid_argument("p must be in [0,1]");
            

            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
            quantileHistory.reserve( fArrayReserve );
        }

        /**
         *  \brief Добавляет значение в окно и пересчитывает min/max/квантиль
         *  \param value Новое значение
         *  \note До формирования окна (IsFormed) в историю пишется aBadValue,
         *  min/max при этом всегда соответствуют текущему (неполному) окну
         */
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

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n значений
         */
        bool IsFormed() const {
            return window.size() == n;
        }

        /**
         *  \brief Текущее состояние индикатора
         *  \return Структура Range с min/max окна и p-квантилью
         */
        Range GetValue() const {
            return range;
        }

        /**
         *  \brief История минимумов окна (по одному значению на каждый Add)
         *  \return Вектор; до формирования окна элементы равны aBadValue
         */
        std::vector< double > GetMinHistory() const {
            return minHistory;
        }

        /**
         *  \brief История максимумов окна (по одному значению на каждый Add)
         *  \return Вектор; до формирования окна элементы равны aBadValue
         */
        std::vector< double > GetMaxHistory() const {
            return maxHistory;
        }

        /**
         *  \brief История p-квантилей окна (по одному значению на каждый Add)
         *  \return Вектор; до формирования окна элементы равны aBadValue
         */
        std::vector< double > GetQuantileHistory() const {
            return quantileHistory;
        }

        /**
         *  \brief Очищает окно и всю историю (индикатор начинает формироваться заново)
         */
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
        
        /**
         *  \brief Относительное изменение значения в окне: последнее / первое - 1
         *  \return Дельта; NAN, если |первое значение| <= gEpsilon
         *  \note Требуется сформированное окно (assert)
         */
        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (std::fabs(lFirstValue) > gEpsilon) ? (window.back() / lFirstValue - 1.0) : NAN;
        }
};


/**
 *  \class RollRange_with_tollerance
 *  \brief Скользящий min/max с допуском (tollerance) и минимальным числом касаний уровня
 *
 *  Значения внутри допуска от края окна считаются "шумом":
 *  - min: первое значение >= (min окна + aTollerance), если значений ниже
 *    (min окна + aTollerance) не меньше aMinTouch, иначе aBadValue;
 *  - max: первое значение > (max окна - aTollerance), если значений выше
 *    не меньше aMinTouch, иначе aBadValue.
 *  \note Non thread-safe. Поле Range::quantile этим классом не заполняется.
 *  \note При aTollerance == 0 и aMinTouch >= 1 значений "ниже min + 0" нет,
 *  поэтому min и max всегда равны aBadValue — задавайте ненулевой допуск.
 */
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

        /**
         *  \brief Конструктор
         *  \param aN Размер окна (>= 1)
         *  \param aMinTouch Минимальное число значений за границей допуска (>= 1)
         *  \param aTollerance Допуск "шума" от края окна (>= 0), по умолчанию 0.0
         *  \param aBadValue Значение при недостатке касаний
         *  \throw std::invalid_argument При aN == 0 или aTollerance < 0
         */
        RollRange_with_tollerance( const size_t aN, const size_t aMinTouch=1, const double aTollerance = 0.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            fMinTouch(aMinTouch),
            fTollerance( aTollerance ),
            fBadValue( aBadValue ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (fTollerance < 0.0 ) throw std::invalid_argument("fTollerance must be >= 0.0");
        }

        /**
         *  \brief Добавляет значение и пересчитывает min/max с допуском
         *  \param value Новое значение
         */
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

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n значений
         */
        bool IsFormed() const {
            return window.size() == n;
        }

        /**
         *  \brief Текущие min/max с допуском
         *  \return Структура Range; quantile не заполняется
         */
        Range GetValue() const {
            return range;
        }

        /**
         *  \brief Очищает окно
         */
        void Reset() {
            window={};
            windowSorted.clear();
        }
};


/**
 *  \class RollSeesaw
 *  \brief Скользящее окно размера n для расчёта относительного изменения ("качели")
 *  \note Non thread-safe
 */
class RollSeesaw {
    private:
        const size_t n;
        std::queue< double > window;

    public:
        /**
         *  \brief Конструктор
         *  \param aN Размер окна (>= 1)
         *  \throw std::invalid_argument При aN == 0
         */
        RollSeesaw( const size_t aN ) :
            n( aN ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
        }

        /**
         *  \brief Добавляет значение в окно (лишнее вытесняется)
         *  \param value Новое значение
         */
        void Add( const double value ) {
            window.push(value);
            if (window.size() > n) {
                window.pop();
            }
        }

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n значений
         */
        bool IsFormed() const {
            return window.size() == n;
        }

        /**
         *  \brief Очищает окно
         */
        void Reset() {
            window={};
        }

        /**
         *  \brief Относительное изменение значения в окне: последнее / первое - 1
         *  \return Дельта; NAN, если |первое значение| <= gEpsilon
         *  \note Требуется сформированное окно (assert)
         */
        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (std::fabs(lFirstValue) > gEpsilon) ? (window.back() / lFirstValue - 1.0) : NAN;
        }
};


/**
 *  \class RollRangeLazy
 *  \brief Скользящий индикатор min/max как RollRange, но с ленивым расчётом p-квантили
 *
 *  История ведётся только для min/max; квантиль вычисляется по требованию
 *  в GetValue(). Add выполняется за O(log n).
 *  \note Non thread-safe
 */
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
        /**
         *  \brief Конструктор
         *  \param aN Размер окна (>= 1)
         *  \param aP Порядок квантили из [0, 1], по умолчанию 1.0
         *  \param aBadValue Значение, записываемое в историю до формирования окна
         *  \throw std::invalid_argument При aN == 0 или aP вне [0, 1]
         *  \note При aP == 1.0 (дефолт) квантиль равна максимуму окна: pN
         *  клампится к -1 (advance от cend())
         */
        RollRangeLazy( const size_t aN, const double aP = 1.0, const double aBadValue = -333333.33 ) :
            n( aN ),
            backSearch(aP >= 0.5),
            pN( (backSearch ? std::min(Trunc((aP - 1.0) * ToDouble(n) ), -1) : Trunc(aP * ToDouble(n) ) ) ),
            fBadValue( aBadValue ),
            fArrayReserve( std::max(aN, 1000UL) ) {
            if (n < 1) throw std::invalid_argument("n must be greater than 0");
            if (aP < 0.0 or aP > 1.0) throw std::invalid_argument("p must be in [0,1]");
            
            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
        }

        /**
         *  \brief Добавляет значение в окно и пересчитывает min/max
         *  \param value Новое значение
         *  \note До формирования окна (IsFormed) в историю пишется aBadValue
         */
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

        /**
         *  \brief Окно набрало заданный размер
         *  \return true, если в окне ровно n значений
         */
        bool IsFormed() const {
            return window.size() == n;
        }

        /**
         *  \brief Текущее состояние: min/max окна и p-квантиль (ленивый расчёт)
         *  \return Структура Range с заполненной квантилью
         *  \note Метод неконстантный: рассчитанная квантиль кэшируется в range
         */
        Range GetValue() {
            auto it = backSearch ? windowSorted.cend() : windowSorted.cbegin();
            std::advance(it, pN);
            range.quantile = *it;

            return range;
        }

        /**
         *  \brief История минимумов окна (по одному значению на каждый Add)
         *  \return Вектор; до формирования окна элементы равны aBadValue
         */
        std::vector< double > GetMinHistory() const {
            return minHistory;
        }

        /**
         *  \brief История максимумов окна (по одному значению на каждый Add)
         *  \return Вектор; до формирования окна элементы равны aBadValue
         */
        std::vector< double > GetMaxHistory() const {
            return maxHistory;
        }

        /**
         *  \brief Очищает окно и всю историю (индикатор начинает формироваться заново)
         */
        void Reset() {
            window={};
            windowSorted.clear();

            minHistory.clear();
            maxHistory.clear();

            minHistory.reserve( fArrayReserve );
            maxHistory.reserve( fArrayReserve );
        }

        /**
         *  \brief Относительное изменение значения в окне: последнее / первое - 1
         *  \return Дельта; NAN, если |первое значение| <= gEpsilon
         *  \note Требуется сформированное окно (assert)
         */
        double GetDelta(){
            assert( IsFormed() );
            const double lFirstValue = window.front();
            return (std::fabs(lFirstValue) > gEpsilon) ? (window.back() / lFirstValue - 1.0) : NAN;
        }
};

#endif //ROLLRANGE_H
