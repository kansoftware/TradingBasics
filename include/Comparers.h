/**
 * \file Comparers.h
 * \brief Модуль содержащий методы для сравнения чисел с заданной точностью
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2017-04-24
 */

#ifndef COMPARERS_H
#define	COMPARERS_H

const double gAbsoluteZero = 0.000001;/*!< Числовное представление "маленького" значения */

/**
 *  \brief Проверяемое число, чуток, но больше чем 0
 */
template< typename TData > inline bool isPositiveValue( const TData A ) {
    return ( A > gAbsoluteZero );
}

/**
 *  \brief A > B
 */
template< typename TData > inline bool IsGreat( const TData A, const TData B ) {
    return ( A - B > gAbsoluteZero );
}

/**
 *  \brief A < B
 */
template< typename TData > inline bool IsLess( const TData A, const TData B ) {
    return ( B - A > gAbsoluteZero );
}

/**
 *  \brief Проверяемое число 0 или типа того
 */
template< typename TData > inline bool isZero( const TData A ) {
    return ( ( -gAbsoluteZero < A ) and ( A < gAbsoluteZero ) );
}

/**
 *  \brief A != B
 */
template< typename TData > inline bool IsNotEqual( const TData A, const TData B ) {
    return ( not isZero( A - B ) );
    //return ( IsGreat( A, B ) or IsLess( A, B ) );
}

/**
 *  \brief A == B
 */
template< typename TData > inline bool IsEqual( const TData A, const TData B ) {
    return ( isZero( A - B ) );
    //return ( A - B < gAbsoluteZero ) and ( B - A < gAbsoluteZero ) ;
    //return ( not IsNotEqual( A, B ) );
}

#endif	/* COMPARERS_H */
