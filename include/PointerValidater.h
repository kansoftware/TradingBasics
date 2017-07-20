/**
 * \file PointerValidater.h
 * \brief Модуль с шаблонами для работы с указателями
 * \author Андрей Карбанов <kan@kansoftware.ru>
 * \since 2014-04-22
 * \date 2016-05-24
 * Об атомарности исполнения функций должен заботится программист
 */

#ifndef CHECKPTRVALIDATE_H_INCLUDED
#define CHECKPTRVALIDATE_H_INCLUDED

#include <memory>

/**
 *  \fn CheckPtrValidate
 *  \brief Проверка валидности указателя
 */
template<typename T> bool isValidPointer( const T* aPointer ) {
    #ifdef NDEBUG
        return aPointer != nullptr;

    #else
        if ( aPointer == nullptr ) { return false; }

        bool result = false;
        try {
            if( nullptr != dynamic_cast< const T* >( aPointer ) ) { result = true; }
        } catch (...) {
        }

        return result;
    #endif
}

inline bool isValidPointer( const void* aPointer ) {
    return aPointer != nullptr;
}

template<typename T> bool isValidPointer( const std::shared_ptr< T > & aPointer ) {
    #ifdef NDEBUG
        return aPointer.operator bool();
    #else
        return isValidPointer( aPointer.get() );
    #endif
}

/**
 *  \fn FreeAndNil
 *  \brief Зачистка памяти
 */
template<typename T> void FreeAndNil( T* &aPointer ) {
    if( isValidPointer( aPointer ) ) {
        delete aPointer;
    }

    aPointer = nullptr;
}

#endif // CHECKPTRVALIDATE_H_INCLUDED
