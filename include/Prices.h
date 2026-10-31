/**
 * \file Prices.h
 * \brief Модуль основные определения для работы с ценами
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2019-09-23
 * \note Дополнительные функции см. в модуле PricesUtils.h
 */

#ifndef PRICES_H
#define	PRICES_H

#include "Comparers.h"

using TPrice = double ;

///\todo можно ввести таблицу с максимальными ценами для каждого инструмента
/**
 *  \var gMaxPrice
 *  \brief Верхняя граница валидной цены (само значение границы — невалидно, интервал открытый).
 */
constexpr TPrice gMaxPrice = 333333.33;
/**
 *  \var gMinPrice
 *  \brief Нижняя граница валидной цены (само значение границы — невалидно, интервал открытый).
 */
constexpr TPrice gMinPrice = 0.00000001;//0.01
/**
 *  \var gDefaultPrice
 *  \brief Цена по умолчанию (нулевая; невалидна с точки зрения IsValidPrice).
 */
constexpr TPrice gDefaultPrice = 0.00;
/**
 *  \var gSpreadValueLimit
 *  \brief Граница допустимого спреда (открытый интервал; сама граница — невалидна).
 */
constexpr TPrice gSpreadValueLimit = 99999.9 ;

/**
 *  \brief Проверка цены на валидность.
 *  \param aPrice Проверяемая цена.
 *  \return true, если цена в интервале (gMinPrice, gMaxPrice) с учётом
 *  эпсилона: сравнения идут через IsLess (B - A > gAbsoluteZero), поэтому
 *  эффективный интервал ОТКРЫТЫЙ — (gMinPrice + gAbsoluteZero,
 *  gMaxPrice - gAbsoluteZero). Сами границы (как и 0, отрицательные цены
 *  и sentinel GetBadPrice()) невалидны.
 */
constexpr bool IsValidPrice( const TPrice aPrice ) {
    return IsLess( gMinPrice, aPrice ) and IsLess( aPrice, gMaxPrice ) ;
}

/**
 *  \brief Получить плохую цену — sentinel «цена не установлена».
 *  \return -gMaxPrice; всегда невалиден с точки зрения IsValidPrice().
 */
constexpr TPrice GetBadPrice() {
    return -gMaxPrice;
}

/**
 *  \brief Проверка валидности значения переданного спреда.
 *  \param aValue Проверяемое значение спреда (допустимы и отрицательные).
 *  \return true, если -gSpreadValueLimit < aValue < gSpreadValueLimit
 *  (интервал открытый; 0 — валидный спред).
 */
constexpr bool isValidSpreadValue( const TPrice aValue ) {
    return IsLess( -gSpreadValueLimit, aValue ) and IsLess( aValue, gSpreadValueLimit );
}

#endif	/* PRICES_H */
