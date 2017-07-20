/*
* \file FileManipulator.h
* \brief Модуль шаблон для работы с типизированными файлами
* \author https://habrahabr.ru/post/134788/
* \since 2016-09-29
* \date 2016-11-03
*/

#ifndef KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
#define KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
#include <iostream>

using std::istream;
using std::ostream;

template<typename T>
class TWrapFileData {
    T t;

public:
    TWrapFileData() : t() {}

    TWrapFileData(T const & t) : t(t) {} //explicit использывать нельзя, иначе не будет работать copy(...)

    operator T&() {
        return t;
    }

    operator T const &() const {
        return t;
    }
};

template<typename T>
istream & operator>>(istream & is, TWrapFileData<T> & wt) {
    is.read(reinterpret_cast<char *>(&static_cast<T &>(wt)), sizeof(T));
    return is;
}

template<typename T>
ostream & operator<<(ostream & os, TWrapFileData<T> const & wt) {
    os.write(
        reinterpret_cast<char const *>(&static_cast<T const &>(wt)),
        sizeof(T));
    return os;
}

#endif //KELTHNERCUDABACKTESTER_FILEMANIPULATOR_H
