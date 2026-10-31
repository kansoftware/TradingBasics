/*
* \file FileManipulator.h
* \brief Модуль шаблон для работы с типизированными файлами
* \author https://habrahabr.ru/post/134788/
* \since 2016-09-29
* \date 2016-11-03
*
* ВНИМАНИЕ: чтение/запись — СЫРОЙ бинарный IO через reinterpret_cast
* (sizeof(T) байт внутреннего поля). Формат НЕ портируем: зависит от
* порядка байт (endianness) платформы и упаковки/паддинга структуры.
* Гарантируется только roundtrip на той же платформе, тем же компилятором
* и той же структурой T. Для переносимых форматов данных не использовать.
*/

#ifndef KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
#define KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
#include <iostream>

using std::istream;
using std::ostream;

/**
 *  \class TWrapFileData
 *  \brief Обёртка над значением T для сырой бинарной записи/чтения потоком.
 *
 *  Неявно преобразуется к T& / T const&, поэтому результат чтения из потока
 *  можно присвоить переменной типа T напрямую.
 */
template<typename T>
class TWrapFileData {
    T t;

public:
    TWrapFileData() : t() {}

    TWrapFileData(T const & aValue) : t(aValue) {} //explicit использовать нельзя, иначе не будет работать copy(...)

    operator T&() {
        return t;
    }

    operator T const &() const {
        return t;
    }
};

/**
 *  \brief Читает sizeof(T) сырых байт из потока во внутреннее поле обёртки.
 *  \param is Входной поток (рекомендуется открытый в binary-режиме).
 *  \param wt Обёртка-приёмник.
 *  \return Поток is; при нехватке байт выставляет failbit (проверяйте good()).
 */
template<typename T>
istream & operator>>(istream & is, TWrapFileData<T> & wt) {
    is.read(reinterpret_cast<char *>(&static_cast<T &>(wt)), sizeof(T));
    return is;
}

/**
 *  \brief Пишет sizeof(T) сырых байт внутреннего поля обёртки в поток.
 *  \param os Выходной поток (рекомендуется открытый в binary-режиме).
 *  \param wt Обёртка-источник.
 *  \return Поток os.
 */
template<typename T>
ostream & operator<<(ostream & os, TWrapFileData<T> const & wt) {
    os.write(
        reinterpret_cast<char const *>(&static_cast<T const &>(wt)),
        sizeof(T));
    return os;
}

#endif //KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
