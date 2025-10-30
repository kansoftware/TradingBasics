#include "StringHasher.h"
#include "DelphisRound.h"

/**
 *  \var gSymbols
 *  \brief Символы в строке для создания уникальных идентификаторов
 */
const std::string gSymbols( "0123456789qwertyuiopasdfghjklzxcvbnm" ); /// QWERTYUIOPASDFGHJKLZXCVBNM (пришлось удалить после проблемы на binance)

/**
 *  \var gSymbols_count
 *  \brief Число символов в строке для создания уникальных идентификаторов
 */
const size_t gSymbols_count = gSymbols.size();// 36 было 62;

//------------------------------------------------------------------------------------------
std::string IntToCode( const long aValue ) {
    if( aValue <= 0 ) return std::string( "0" );

    long lDiv = aValue;
    std::string lresult;

    while( lDiv > 0 ) {
        const size_t lSymbolCode = ToSize_t( ToInt( lDiv ) % ToInt( gSymbols_count ) );
        lresult = gSymbols[ lSymbolCode ] + lresult;
        lDiv /= static_cast< long >( gSymbols_count );
    }

    return lresult;
}

//------------------------------------------------------------------------------------------
size_t GetSymbolsCount() {
    return gSymbols_count;
}

//------------------------------------------------------------------------------------------
 char GetSymbolByIdx( const size_t aIdx ) {
    if( aIdx >= gSymbols_count ) {
        return '_';
    
    } else {
        return gSymbols[ aIdx ] ;
    }
}

//------------------------------------------------------------------------------------------