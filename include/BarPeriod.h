/**
 * \file BarPeriod.h
 * \brief Модуль хранящий в себе типы периодов для баров и основные методы работы с ними
 * \author kan <kansoftware.ru>
 * \since 2016-12-19
 * \date 2016-12-20
 * Модуль используется неизменно в проектах TradeBot и [R] BackTester
 */

#ifndef BARPERIOD_H
#define BARPERIOD_H

#include <string>

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
    maxValue };

std::string getBarPeriodName( const TBarPeriod aID );
double getBarPeriodLength( const TBarPeriod aID );

#endif /* BARPERIOD_H */
