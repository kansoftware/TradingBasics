/**
 * \file Comparers.h
 * \brief Модуль содержащий методы для сравнения чисел с заданной точностью
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2019-09-23
 */

#ifndef COMPARERS_H
#define	COMPARERS_H

constexpr double gAbsoluteZero = 0.000000001;/*!< 0.000001 Числовное представление "маленького" значения */

/**
 *  \brief Проверяемое число, чуток, но больше чем 0
 */
template< typename TData > constexpr bool isPositiveValue( const TData A, const double aAbsoluteZero = gAbsoluteZero ) {
    return ( A > aAbsoluteZero );
}

/**
 *  \brief A > B
 */
template< typename TData > constexpr bool IsGreat( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( A - B > aAbsoluteZero );
}

/**
 *  \brief A < B
 */
template< typename TData > constexpr bool IsLess( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( B - A > aAbsoluteZero );
}

/**
 *  \brief Проверяемое число 0 или типа того
 */
template< typename TData > constexpr bool isZero( const TData A, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( ( -aAbsoluteZero < A ) and ( A < aAbsoluteZero ) );
}

/**
 *  \brief A != B
 */
template< typename TData > constexpr bool IsNotEqual( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( not isZero( A - B, aAbsoluteZero ) );
}

/**
 *  \brief A == B
 */
template< typename TData > constexpr bool IsEqual( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero ) {
    return ( isZero( A - B, aAbsoluteZero ) );
}

#endif	/* COMPARERS_H */
