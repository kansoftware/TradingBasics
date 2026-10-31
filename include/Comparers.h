/**
 * \file Comparers.h
 * \brief Модуль содержащий методы для сравнения чисел с заданной точностью
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2025-06-21
 */

#ifndef COMPARERS_H
#define	COMPARERS_H

/**
 *  \var gAbsoluteZero
 *  \brief Абсолютная погрешность ("маленькое" число) по умолчанию для всего семейства сравнения.
 *
 *  Семантика эпсилона: значения в ОТКРЫТОМ интервале (-gAbsoluteZero; gAbsoluteZero)
 *  считаются нулём; сами границы нулём не считаются (неравенства строгие).
 *  Для сравнения величин A и B проверяется их разность тем же порогом.
 */
constexpr double gAbsoluteZero = 1.0e-9;/*!< Числовное представление "маленького" значения , мы обычно используем numeric 18.8*/

/**
 *  \brief Проверяемое число чуток, но больше чем 0.
 *  \param A Проверяемое число.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если A > aAbsoluteZero (строго; значение ровно на границе — не "положительное").
 */
template< typename TData > constexpr bool isPositiveValue( const TData A, const double aAbsoluteZero = gAbsoluteZero ) {
    return ( A > aAbsoluteZero );
}

/**
 *  \brief A > B с учётом погрешности.
 *  \param A Левый операнд.
 *  \param B Правый операнд.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если A - B > aAbsoluteZero (разность ровно на границе — ещё не "больше").
 */
template< typename TData > constexpr bool IsGreat( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( A - B > aAbsoluteZero );
}

/**
 *  \brief A < B с учётом погрешности.
 *  \param A Левый операнд.
 *  \param B Правый операнд.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если B - A > aAbsoluteZero (симметрично IsGreat).
 */
template< typename TData > constexpr bool IsLess( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( B - A > aAbsoluteZero );
}

/**
 *  \brief Проверяемое число 0 или типа того.
 *  \param A Проверяемое число.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если -aAbsoluteZero < A < aAbsoluteZero (открытый интервал; сами границы — не ноль).
 */
template< typename TData > constexpr bool isZero( const TData A, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( ( -aAbsoluteZero < A ) and ( A < aAbsoluteZero ) );
}

/**
 *  \brief A != B с учётом погрешности.
 *  \param A Левый операнд.
 *  \param B Правый операнд.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если разность A - B вне эпсилон-зоны (отрицание isZero).
 */
template< typename TData > constexpr bool IsNotEqual( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero  ) {
    return ( not isZero( A - B, aAbsoluteZero ) );
}

/**
 *  \brief A == B с учётом погрешности.
 *  \param A Левый операнд.
 *  \param B Правый операнд.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return true, если разность A - B попадает в открытый интервал (-aAbsoluteZero; aAbsoluteZero).
 */
template< typename TData > constexpr bool IsEqual( const TData A, const TData B, const double aAbsoluteZero = gAbsoluteZero ) {
    return ( isZero( A - B, aAbsoluteZero ) );
}


/**
 *  \brief Получить знак числа с учётом погрешности.
 *  \param x Проверяемое число.
 *  \param aAbsoluteZero Порог (по умолчанию gAbsoluteZero).
 *  \return 1, если x > aAbsoluteZero; -1, если x < -aAbsoluteZero; 0 в эпсилон-зоне.
 */
template< typename TData > constexpr int Sign( TData x, const double aAbsoluteZero = gAbsoluteZero ){
  if( x > aAbsoluteZero ) return 1;
  if( x < -aAbsoluteZero ) return -1;
  return 0;
}

#endif	/* COMPARERS_H */
