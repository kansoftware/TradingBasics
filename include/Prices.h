/**
 * \file Prices.h
 * \brief Модуль основные определения для работы с ценами
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2016-04-12
 * Дополнительные функции см. в модуле PricesUtils.h
 */

#ifndef PRICES_H
#define	PRICES_H

typedef double TPrice;

///\todo можно ввести таблицу с максимальными ценами для каждого инструмента
const TPrice gMaxPrice = 333333.33;
const TPrice gMinPrice = 0.01;
const TPrice gDefaultPrice = 0.00;

/**
 *  \brief проверка цены
 */
bool IsValidPrice( const TPrice aPrice );

/**
 *  \brief Получить плохую цену
 */
TPrice GetBadPrice();

/**
 *  \brief Проверка валидности значения переданного спреда
 */
bool isValidSpreadValue( const TPrice aValue );

#endif	/* PRICES_H */
