/**
 * \file DelphisRound.h
 * \brief Модуль содержащий методы для округления с приведением типа, как в Дельфи
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2025-06-21
 */

#ifndef DELPHISROUND_H
#define	DELPHISROUND_H

#include <cmath>
#include <limits>
#include <stdexcept>
#include <cassert>

#include "Comparers.h"


/**
 *  \var gMaxInteger
 *  \brief Верхняя граница int в double — порог assert'ов Int-функций в debug-сборке.
 */
static const double gMaxInteger = static_cast< double >( std::numeric_limits<int>::max() );
/**
 *  \var gMinInteger
 *  \brief Нижняя граница int в double — порог assert'ов Int-функций в debug-сборке.
 */
static const double gMinInteger = static_cast< double >( std::numeric_limits<int>::min() );
/**
 *  \var gMaxSizet
 *  \brief Верхняя граница size_t в double — порог assert'ов size_t-функций.
 */
static const double gMaxSizet = static_cast< double >( std::numeric_limits<size_t>::max() );
/**
 *  \var gEpsilon
 *  \brief Машинная погрешность сравнений модуля (на данный момент вне функций не используется).
 */
static const double gEpsilon = 1e-9;

/**
 *  \brief ВАЖНО о NDEBUG-макросах.
 *
 *  Часть функций модуля собрана парами: в release (NDEBUG) — это МАКРОСЫ
 *  (простое static_cast, без проверок), в debug — ШАБЛОНЫ с тем же телом плюс
 *  assert'ы на диапазон. Наблюдаемая семантика для значений в допустимом
 *  диапазоне одинакова в обоих режимах; при выходе за диапазон debug-сборка
 *  падает по assert, release даёт неопределённое поведение усечения.
 *  Тесты не должны проверять assert-поведение — только общие семантики.
 */

/**
 *  \fn ToDouble
 *  \brief Приведение к double.
 *  \param aValue Приводимое значение.
 *  \return Значение, приведённое к double (static_cast).
 */
#ifdef NDEBUG
    #define ToDouble( aValue ) static_cast< double >( (aValue) )
#else
    template< typename T > double ToDouble( const T aValue ) {
        return static_cast< double >( aValue );
    }
#endif

/**
 *  \fn ToSize_t
 *  \brief Приведение к size_t; отрицательные (и эпсилон-нулевые) значения заменяются на 0.
 *  \param aValue Приводимое значение.
 *  \return static_cast<size_t>(aValue) для aValue > gAbsoluteZero, иначе 0.
 */
template< typename T > constexpr size_t ToSize_t( const T aValue ) {
    assert( ToDouble( aValue ) < gMaxSizet  );

    return isPositiveValue( ToDouble(aValue) ) ? static_cast< size_t >( aValue ) : 0UL ;
}

/**
 *  \fn ToInt
 *  \brief Приведение к int усечением к нулю (static_cast); вне диапазона int —
 *  в debug assert, в release UB (см. ВАЖНО о NDEBUG-макросах).
 *  \param aValue Приводимое значение.
 *  \return Целая часть aValue, усечённая к нулю: ToInt(5.7)==5, ToInt(-5.7)==-5.
 */
#ifdef NDEBUG
    #define ToInt( aValue ) static_cast< int >( (aValue) )
#else
    template< typename T > int ToInt( const T aValue ) {
        assert( gMinInteger <= ToDouble( aValue ) );
        assert( ToDouble( aValue ) <= gMaxInteger );

        return static_cast< int >( aValue );
    }
#endif

/**
 *  \fn Round
 *  \brief Округление к ближайшему целому из double в int.
 *
 *  std::round округляет половины ОТ НУЛЯ (2.5 -> 3, -2.5 -> -3) — в отличие от
 *  банковского округления Delphi. Вне диапазона int — в debug assert, в release UB.
 *  \param aValue Округляемое значение.
 *  \return Ближайшее целое как int.
 *  \todo всё с округлением вынести в отдельный модуль
 */
#ifdef NDEBUG
    #define Round( aValue ) static_cast< int >( std::round( aValue ) )
#else
    template< typename T > int Round( const T aValue ) {
        assert( gMinInteger <= ToDouble( aValue ) );
        assert( ToDouble( aValue ) <= gMaxInteger );

        return static_cast< int >( std::round( aValue ) );
    }
#endif

/**
 *  \fn RoundToSize_t
 *  \brief Округление к ближайшему целому в size_t; отрицательные значения заменяются на 0.
 *  \param aValue Округляемое значение.
 *  \return std::round(aValue) для aValue > gAbsoluteZero, иначе 0 (RoundToSize_t(-0.5)==0).
 */
template< typename T > constexpr size_t RoundToSize_t( const T aValue ) {
    assert( ToDouble( aValue ) < gMaxSizet );

    return isPositiveValue( aValue ) ? static_cast< size_t >( std::round( aValue ) ) : 0UL ;
}

/**
 *  \fn Ceil
 *  \brief Округление вверх из double в int.
 *  Вне диапазона int — в debug assert, в release UB (см. ВАЖНО о NDEBUG-макросах).
 *  \param aValue Округляемое значение.
 *  \return Наименьшее целое >= aValue (Ceil(-5.4)==-5).
 */
#ifdef NDEBUG
    #define Ceil( aValue ) static_cast< int >( std::ceil( aValue ) )
#else
    template< typename T > int Ceil( const T aValue ) {
        assert( gMinInteger <= ToDouble( aValue ) );
        assert( ToDouble( aValue ) <= gMaxInteger );

        return static_cast< int >( std::ceil( aValue ) );
    }
#endif

/**
 *  \fn CeilToSize_t
 *  \brief Округление вверх в size_t; отрицательные значения заменяются на 0.
 *  \param aValue Округляемое значение.
 *  \return std::ceil(aValue) для aValue > gAbsoluteZero, иначе 0.
 */
template< typename T > constexpr size_t CeilToSize_t( const T aValue ) {
    assert( ToDouble( aValue ) <  gMaxSizet );

    return isPositiveValue( aValue ) ? static_cast< size_t >( std::ceil( aValue ) ) : 0UL ;
}

/**
 *  \fn Trunc
 *  \brief Округление вниз (усечение) из double в int.
 *  Вне диапазона int — в debug assert, в release UB (см. ВАЖНО о NDEBUG-макросах).
 *  \param aValue Округляемое значение.
 *  \return Наибольшее целое <= aValue (Trunc(-5.4)==-6).
 */
#ifdef NDEBUG
    #define Trunc( aValue ) static_cast< int >( std::floor( aValue ) )
#else
    template< typename T > int Trunc( const T aValue ) {
        assert( gMinInteger <= ToDouble( aValue ) );
        assert( ToDouble( aValue ) <= gMaxInteger );

        return static_cast< int >( std::floor( aValue ) );
    }
#endif

/**
 *  \fn TruncToSize_t
 *  \brief Округление вниз (усечение) в size_t; отрицательные значения заменяются на 0.
 *  \param aValue Округляемое значение.
 *  \return std::floor(aValue) для aValue > gAbsoluteZero, иначе 0.
 */
template< typename T > constexpr size_t TruncToSize_t( const T aValue ) {
    assert( ToDouble( aValue ) < gMaxSizet );
    
    return isPositiveValue( aValue ) ? static_cast< size_t >( std::floor( aValue ) ) : 0UL ;
}

#endif	/* DELPHISROUND_H */
