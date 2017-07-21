/**
 * \file StringHasher.h
 * \brief Модуль содержащий функции для перевода чисел в хеш-строку
 * \author Андрей Карбанов <kan@kansoftware.ru>
 * \since 2015-08-25
 * \date 2015-08-25
 */

#ifndef STRINGHASHER_H
#define	STRINGHASHER_H

#include <string>

/**
 *  \fn IntToCode
 *  \brief преобразование числа в сжатую gSymbols строку
 */
std::string IntToCode( const long aValue );

/**
 *  \fn GetSymbolByIdx
 *  \brief Получить символ по его коду
 */
char GetSymbolByIdx( const size_t aIdx );

/**
 *  \fn GetSymbolsCount
 *  \brief Получить число символов в наборе
 */
size_t GetSymbolsCount();

#endif	/* STRINGHASHER_H */

