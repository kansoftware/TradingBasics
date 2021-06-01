/** 
 * File:   BarPeriod.cpp
 * Author: kan
 * 
 * Created on December 19, 2016, 5:37 PM
 */

#include "BarPeriod.h"

const char *gBarPeriodNames[ static_cast<size_t>( TBarPeriod::maxValue ) ] = { "0MIN", "1MIN", "5MIN", "10MIN", "15MIN", "1HOUR", "4HOURS", "1DAY", "1WEEK", "1SEC" };

//-----------------------------------------------------------------------------
std::string getBarPeriodName( const TBarPeriod aID ) {
    assert( aID > TBarPeriod::minValue );
    assert( aID < TBarPeriod::maxValue );
    return std::string( gBarPeriodNames[ static_cast<size_t>( aID ) ] );
}

//-----------------------------------------------------------------------------
