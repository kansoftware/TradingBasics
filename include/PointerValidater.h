/**
 * \file PointerValidater.h
 * \brief Модуль с шаблонами для работы с указателями
 * \author kan <kansoftware.ru>
 * \since 2014-04-22
 * \date 2026-10-31
 * \note Об атомарности исполнения функций должен заботиться программист
 */

#ifndef CHECKPTRVALIDATE_H_INCLUDED
#define CHECKPTRVALIDATE_H_INCLUDED

#include <memory>

/**
 *  \fn isValidPointer
 *  \brief Проверка валидности указателя: true, если указатель не нулевой.
 *
 *  Единственная реализация для всех режимов сборки (ранняя debug-ветка с
 *  dynamic_cast к тому же типу не имела наблюдаемых эффектов: dynamic_cast
 *  по указателю никогда не бросает и для ненулевого указателя всегда успешен).
 *  Работает и для неклассных типов T.
 */
template<typename T> constexpr bool isValidPointer( const T* aPointer ) {
    return aPointer != nullptr;
}

/**
 *  \brief Перегрузка для const void* (в том числе для литерала nullptr,
 *  который не выводит T в шаблонной перегрузке).
 */
constexpr bool isValidPointer( const void* aPointer ) {
    return aPointer != nullptr;
}

/**
 *  \brief Перегрузка для std::shared_ptr: true, если указатель владеет
 *  объектом (эквивалентно static_cast<bool>( aPointer )).
 */
template<typename T> constexpr bool isValidPointer( const std::shared_ptr< T > & aPointer ) {
    return aPointer.operator bool();
}

/**
 *  \fn FreeAndNil
 *  \brief Зачистка памяти: удаляет объект и обнуляет указатель.
 *
 *  Для нулевого указателя удаление не выполняется (только обнуление);
 *  после вызова aPointer гарантированно равен nullptr.
 */
template<typename T> void FreeAndNil( T* &aPointer ) {
    if( aPointer != nullptr ) {
        delete aPointer;
    }

    aPointer = nullptr;
}

#endif // CHECKPTRVALIDATE_H_INCLUDED
