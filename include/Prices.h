/**
 * \file Prices.h
 * \brief Модуль основные определения для работы с ценами
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2019-09-23
 * Дополнительные функции см. в модуле PricesUtils.h
 */

#ifndef PRICES_H
#define	PRICES_H

#include "Comparers.h"

using TPrice = double ;

///\todo можно ввести таблицу с максимальными ценами для каждого инструмента
constexpr TPrice gMaxPrice = 333333.33;
constexpr TPrice gMinPrice = 0.00000001;//0.01
constexpr TPrice gDefaultPrice = 0.00;
constexpr TPrice gSpreadValueLimit = 99999.9 ;

/**
 *  \brief проверка цены
 */
constexpr bool IsValidPrice( const TPrice aPrice ) {
    return IsLess( gMinPrice, aPrice ) and IsLess( aPrice, gMaxPrice ) ;
}

/**
 *  \brief Получить плохую цену
 */
constexpr TPrice GetBadPrice() {
    return -gMaxPrice;
}

/**
 *  \brief Проверка валидности значения переданного спреда
 */
constexpr bool isValidSpreadValue( const TPrice aValue ) {
    return IsLess( -gSpreadValueLimit, aValue ) and IsLess( aValue, gSpreadValueLimit );
}

#endif	/* PRICES_H */
