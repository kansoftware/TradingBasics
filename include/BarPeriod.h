/**
 * \file BarPeriod.h
 * \brief Модуль хранящий в себе типы периодов для баров и основные методы работы с ними
 * \author kan <kansoftware.ru>
 * \since 2016-12-19
 * \date 2021-05-31
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */

#ifndef BARPERIOD_H
#define BARPERIOD_H

#include <string>
#include <cassert>

/**
 *  \enum TBarPeriod
 *  \brief Типы периодов баров
 *
 *  Значения enum — порядковые номера, а не длины; длины в секундах даёт
 *  getBarPeriodLength. minValue/maxValue — служебные границы (не периоды).
 */
enum class TBarPeriod {
    minValue = 0,   /*!< Служебная нижняя граница (не период) */
    Minute = 1,     /*!< 1 минута */
    Minute5 = 2,    /*!< 5 минут */
    Minute10 = 3,   /*!< 10 минут */
    Minute15 = 4,   /*!< 15 минут */
    Hour = 5,       /*!< 1 час */
    Hour4 = 6,      /*!< 4 часа */
    Day = 7,        /*!< 1 сутки */
    Week = 8,       /*!< 1 неделя */
    Second = 9,     /*!< 1 секунда */
    maxValue };     /*!< Служебная верхняя граница (не период) */

/**
 *  \brief Длины периодов TBarPeriod в секундах (индекс — значение enum)
 *
 *  Слот minValue = 0.0 служебный: getBarPeriodLength не допускает это значение.
 */
constexpr double gOutBarPeriod[ static_cast<size_t>( TBarPeriod::maxValue ) ] = { 0.0, 60.0, 60.0*5, 60.0*10, 60.0*15, 60.0*60, 60.0*60*4, 60.0*60*24, 60.0*60*24*7, 1 };

/**
 *  \brief Длина периода в секундах
 *  \param aID Идентификатор периода (строго между minValue и maxValue)
 *  \return Длина периода в секундах (Minute=60, ..., Week=604800, Second=1);
 *          выход за границы — assert (в debug-сборке)
 */
constexpr double getBarPeriodLength( const TBarPeriod aID ) {
    assert( aID > TBarPeriod::minValue );
    assert( aID < TBarPeriod::maxValue );
    return gOutBarPeriod[ static_cast<size_t>( aID ) ];
}

/**
 *  \brief Короткое имя периода
 *  \param aID Идентификатор периода (строго между minValue и maxValue)
 *  \return Имя периода: "1SEC", "1MIN", "5MIN", "10MIN", "15MIN", "1HOUR",
 *          "4HOURS", "1DAY", "1WEEK"; выход за границы — assert (в debug-сборке)
 */
std::string getBarPeriodName( const TBarPeriod aID );

#endif /* BARPERIOD_H */
