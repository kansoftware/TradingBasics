// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/** 
 * File:   BarPeriod.cpp
 * Author: kan
 * 
 * Created on December 19, 2016, 5:37 PM
 */

#include <cassert>

#include "BarPeriod.h"

const char *gBarPeriodNames[ static_cast<size_t>( TBarPeriod::maxValue ) ] = { "0MIN", "1MIN", "5MIN", "10MIN", "15MIN", "1HOUR", "4HOURS", "1DAY", "1WEEK" };
const double gOutBarPeriod[ static_cast<size_t>( TBarPeriod::maxValue ) ] = { 0.0, 60.0, 60.0*5, 60.0*10, 60.0*15, 60.0*60, 60.0*60*4, 60.0*60*24, 60.0*60*24*7 };

//-----------------------------------------------------------------------------
std::string getBarPeriodName( const TBarPeriod aID ) {
    assert( aID > TBarPeriod::minValue );
    assert( aID < TBarPeriod::maxValue );
    return gBarPeriodNames[ static_cast<size_t>( aID ) ];
}

//-----------------------------------------------------------------------------
double getBarPeriodLength( const TBarPeriod aID ) {
    assert( aID > TBarPeriod::minValue );
    assert( aID < TBarPeriod::maxValue );
    return gOutBarPeriod[ static_cast<size_t>( aID ) ];    
}

//-----------------------------------------------------------------------------
