// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Comparers.h"

#include "Prices.h"

const TPrice gSpreadValueLimit = 99999.9 ;

//------------------------------------------------------------------------------------------
bool IsValidPrice( const TPrice aPrice ) {
    return //IsNotEqual( aPrice, gBadPrice ) and
            IsLess( gMinPrice, aPrice ) and IsLess( aPrice, gMaxPrice ) ;
}

//------------------------------------------------------------------------------------------
bool isValidSpreadValue( const TPrice aValue ) {
    return IsLess( -gSpreadValueLimit, aValue ) and IsLess( aValue, gSpreadValueLimit );
}

//------------------------------------------------------------------------------------------
TPrice GetBadPrice() {
    return -gMaxPrice;
}

//------------------------------------------------------------------------------------------
