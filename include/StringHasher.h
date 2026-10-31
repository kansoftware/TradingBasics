/**
 * \file StringHasher.h
 * \brief Модуль содержащий функции для перевода чисел в хеш-строку
 * \author kan <kansoftware.ru>
 * \since 2015-08-25
 * \date 2015-08-25
 */

#ifndef STRINGHASHER_H
#define	STRINGHASHER_H

#include <string>

/**
 *  \fn IntToCode
 *  \brief Преобразование числа в сжатую строку по алфавиту gSymbols (base-36).
 *
 *  Алфавит — [0-9a-z] из 36 символов (uppercase-буквы удалены после проблем
 *  на binance; служебный '_' вне алфавита выдаёт только GetSymbolByIdx вне
 *  диапазона); обратное преобразование (CodeToInt) не существует. Числа <= 0
 *  вырождаются в строку "0". Внимание: внутри используется ToInt(lDiv), поэтому
 *  значения больше INT_MAX недопустимы — в debug-сборке assert, в release UB
 *  (см. DelphisRound.h). Результат используется как компонент orderLinkId Bybit
 *  (ограничение длины идентификатора — 36 символов).
 *  \param aValue Преобразуемое число (0 < aValue <= INT_MAX).
 *  \return Строка из символов алфавита, старший разряд первым; "0" для aValue <= 0.
 */
std::string IntToCode( const long aValue );

/**
 *  \fn GetSymbolByIdx
 *  \brief Получить символ по его индексу в алфавите.
 *  \param aIdx Индекс в диапазоне [0; GetSymbolsCount()).
 *  \return Символ алфавита; вне диапазона — служебный '_'.
 */
char GetSymbolByIdx( const size_t aIdx );

/**
 *  \fn GetSymbolsCount
 *  \brief Получить число символов в наборе.
 *  \return Размер алфавита (36).
 */
size_t GetSymbolsCount();

#endif	/* STRINGHASHER_H */
