// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/** 
 * File:   BarPeriod.cpp
 * Author: kan
 * 
 * Created on 11.11.2015.
 */

#include <cmath>
#include <time.h>
#include <stddef.h>
#include <cassert>
#include <sstream>

#include "Comparers.h"
#include "DelphisRound.h"
#include "BarPeriod.h"

#include "BasisOfStrategy.h"

//------------------------------------------------------------------------------------------
const std::string whitespaces (" \t\f\v\n\r");
std::string trim( const std::string& str ) {
    const size_t first = str.find_first_not_of( whitespaces );
    if( std::string::npos == first ) {
        return str;
    }
    const size_t last = str.find_last_not_of( whitespaces );
    return str.substr(first, (last - first + 1));
}

//------------------------------------------------------------------------------------------
bool IsOneDay( const TInnerDate aLeft, const TInnerDate aRight) {
    return Trunc( aLeft / gOneDay ) == Trunc( aRight / gOneDay );
}

//------------------------------------------------------------------------------------------
bool IsOneHour( const TInnerDate aLeft, const TInnerDate aRight, const double aDuration ) {
    assert( isPositiveValue( aDuration ) );
    return IsLess( std::fabs( aLeft - aRight ), gOneHour*aDuration + 1.0 );
}

//------------------------------------------------------------------------------------------
bool IsValidBar( const TSimpleBar & aBar ) {
    return IsGreat( aBar.Volume, -gAbsoluteZero )
           and IsValidPrice( aBar.Open )
           and IsValidPrice( aBar.High )
           and IsValidPrice( aBar.Low )
           and IsValidPrice( aBar.Close );
}

//------------------------------------------------------------------------------------------
#define FAST_DECODER( aField ) \
    for( std::size_t lRowNum = 0; lRowNum < aBar.size(); ++lRowNum ) { \
        TSimpleBar lBar = aBar[ lRowNum ]; \
        TPrice lPrice = lBar.aField; \
        TSimpleTick lTick { lBar.DateTime, lPrice, lBar.Volume }; \
        lResult[ lRowNum ] = lTick; \
    } \
// end of FAST_DECODER

TPriceSeries BarsToPriceSeries( const TBarSeries &aBar, const TMAPoint aType ) {

    TPriceSeries lResult( aBar.size() );

    switch ( aType ) {
        /////////
        case TMAPoint::MAP_Close : {
            FAST_DECODER( Close );
        } break ;
        /////////
        case TMAPoint::MAP_Mid : {
            for( std::size_t lRowNum = 0; lRowNum < aBar.size(); ++lRowNum ) {
                TSimpleBar lBar = aBar[ lRowNum ];
                TPrice lPrice = ( lBar.High + lBar.Low ) / 2.0 ;
                TSimpleTick lTick { lBar.DateTime, lPrice, lBar.Volume };
                lResult[ lRowNum ] = lTick;
            }
        } break ;
        /////////
        case TMAPoint::MAP_Triple : {
            for( std::size_t lRowNum = 0; lRowNum < aBar.size(); ++lRowNum ) {
                TSimpleBar lBar = aBar[ lRowNum ];
                TPrice lPrice = ( lBar.High + lBar.Low + lBar.Close ) / 3.0 ;
                TSimpleTick lTick { lBar.DateTime, lPrice, lBar.Volume };
                lResult[ lRowNum ] = lTick;
            }
        } break ;
        /////////
        case TMAPoint::MAP_Open : {
            FAST_DECODER( Open );
        } break ;
        /////////
        case TMAPoint::MAP_High : {
            FAST_DECODER( High );
        } break ;
        /////////
        case TMAPoint::MAP_Low : {
            FAST_DECODER( Low );
        } break ;
        /////////
        case TMAPoint::MAP_Volume : {
            FAST_DECODER( Volume );
        } break ;
    }

    return lResult;
}

//------------------------------------------------------------------------------------------
void Reset( TSimpleBar &aBar ) {
    aBar.DateTime = gStartingTime;
    aBar.Open = 0.0;
    aBar.High = 0.0;
    aBar.Low = 0.0;
    aBar.Close = 0.0;
    aBar.Volume = -1.0;
}

//------------------------------------------------------------------------------------------
TBarSeries _CreateBars( const TBarSeries & aBars, const TBarPeriod aBarPeriod ) {
   
    const double lOutBarPeriod = getBarPeriodLength( aBarPeriod );//0.0;
    
    TBarSeries oResuiltBarSeries;
    TSimpleBar lOutBar;
    Reset( lOutBar );
    
    int GlobalBarIndex = 0;
    
    for( size_t i = 0; i < aBars.size( ); i++ ) {
        
        const TSimpleBar lCurrentBar = aBars[ i ];
        const TInnerDate lBarTime = lCurrentBar.DateTime;
        const int lCurrentBarIndex = Trunc( ( lBarTime - gStartingTime ) / lOutBarPeriod );
        
        if( lCurrentBarIndex != GlobalBarIndex ) {
            
            if( GlobalBarIndex != 0 ) {
                lOutBar.DateTime = GlobalBarIndex * lOutBarPeriod + gStartingTime;
                oResuiltBarSeries.push_back( lOutBar );
                Reset( lOutBar );
            }
            
            GlobalBarIndex = lCurrentBarIndex;
        }
        
        lOutBar = lOutBar + lCurrentBar;
        
        if( ( aBars.size( ) - i ) == 1 ) {
            lOutBar.DateTime = lCurrentBarIndex * lOutBarPeriod + gStartingTime;
            oResuiltBarSeries.push_back( lOutBar );
            Reset( lOutBar );
        }
    }

    return oResuiltBarSeries;
}

//------------------------------------------------------------------------------------------
TSimpleBar operator+( const TSimpleBar &aStarBar, const TSimpleBar &aEndBar ) {
    
    if( not IsValidBar( aEndBar ) ) {
        return aStarBar;
    }
    
    if( not IsValidBar( aStarBar ) ) {
        return aEndBar;
    }
    
    const double lOpen =  aStarBar.Open;
    const double lHigh = IsGreat( aStarBar.High, aEndBar.High ) ? aStarBar.High : aEndBar.High ;
    const double lLow = IsLess( aStarBar.Low, aEndBar.Low ) ? aStarBar.Low : aEndBar.Low;
    const double lClose = aEndBar.Close;
    const double lVolume = aStarBar.Volume + aEndBar.Volume;

    const TSimpleBar lOutBar{ aStarBar.DateTime, lOpen, lHigh, lLow, lClose, lVolume };

    return lOutBar;
}

//------------------------------------------------------------------------------------------
std::string DateToStr( const TInnerDate aDate ) {
    const time_t lTime = Round( aDate );
    const std::string lStrTime( ctime( &lTime ) );
    
    return trim( lStrTime );
}

//------------------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream &out, const TSimpleBar &aBar ) {
    std::stringstream strm;
    
    strm    << DateToStr( aBar.DateTime ) << "\t"
            << aBar.Open << "\t"
            << aBar.High << "\t"
            << aBar.Low << "\t"
            << aBar.Close << "\t"
            << aBar.Volume ;

    return out << strm.str();
}

//------------------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream &out, const TSimpleTick &aTick ) {
    std::stringstream strm;
    
    strm    << DateToStr( aTick.DateTime ) << "\t"
            << aTick.Price << "\t"
            << aTick.Volume ;
    return out << strm.str();     
}

//------------------------------------------------------------------------------------------
TPrice RoundTo( const TPrice aPrice, const TPrice aPriceStep ) {
    assert( isPositiveValue( aPriceStep ) );
    
    return std::round( aPrice / aPriceStep ) * aPriceStep ;
}

//------------------------------------------------------------------------------------------
TPrice TruncTo( const TPrice aPrice, const TPrice aPriceStep ) {
    assert( isPositiveValue( aPriceStep ) );
    
    return std::trunc( aPrice / aPriceStep ) * aPriceStep ;
}

//------------------------------------------------------------------------------------------
TPrice CeilTo( const TPrice aPrice, const TPrice aPriceStep ) {
    assert( isPositiveValue( aPriceStep ) );
    
    return std::ceil( aPrice / aPriceStep ) * aPriceStep ;
}

//------------------------------------------------------------------------------------------
