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

enum class TBarPeriod { 
    minValue = 0,
    Minute = 1,
    Minute5 = 2,
    Minute10 = 3,
    Minute15 = 4,
    Hour = 5,
    Hour4 = 6,
    Day = 7,
    Week = 8,
    Second = 9,
    maxValue };
    
constexpr double gOutBarPeriod[ static_cast<size_t>( TBarPeriod::maxValue ) ] = { 0.0, 60.0, 60.0*5, 60.0*10, 60.0*15, 60.0*60, 60.0*60*4, 60.0*60*24, 60.0*60*24*7, 1 };

constexpr double getBarPeriodLength( const TBarPeriod aID ) {
    assert( aID > TBarPeriod::minValue );
    assert( aID < TBarPeriod::maxValue );
    return gOutBarPeriod[ static_cast<size_t>( aID ) ];    
}

std::string getBarPeriodName( const TBarPeriod aID );

#endif /* BARPERIOD_H */
