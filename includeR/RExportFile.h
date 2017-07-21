//
// Created by kan on 05.11.2015.
//

#ifndef BACKTESTER_REXPORTFILE_H
#define BACKTESTER_REXPORTFILE_H

#include <Rcpp.h>
#include "PointerValidater.h"
#include "FileManipulator.h"
#include "BasisOfStrategy.h"
#include "Indicators.h"
#include "PnlAction.h"

// [[Rcpp::export]]
Rcpp::NumericVector SimpleMA( const Rcpp::NumericMatrix & aXts, const int aPeriod, const int aType = 3 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector ExponentMA( const Rcpp::NumericMatrix & aXts, const int aPeriod, const int aType = 3 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector KaufmanMA( const Rcpp::NumericMatrix & aXts, const int aPeriod, const double aKoeff, const int aType = 3 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector TrueRange( const Rcpp::NumericMatrix & aXts );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector AverageTrueRange( const Rcpp::NumericMatrix & aXts, const int aPeriod );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::List DI( const Rcpp::NumericMatrix & aXts, const int aPeriod = 14 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector ZigZag( const Rcpp::NumericMatrix & aOHLCV, const double aGap );
//------------------------------------------------------------------------------------------
        
//' @title The Parabolic Stop-and-Reverse calculates a trailing stop. Developed by J. Welles Wilder
//' @name SAR
//' @param aOHLCV xts object contains OHLCV prices
//' @param aAccFactor Acceleration factor
//' @param aMaxAccFactor Maximum acceleration factor
//' @family indicator
//' @return Returns A object containing the Parabolic Stop and Reverse values as xts.
//' @description First value is first bars low.
//' @export
// [[Rcpp::export]]
Rcpp::NumericVector SAR( const Rcpp::NumericMatrix & aOHLCV, const double aAccFactor = 0.02, const double aMaxAccFactor = 0.2, const bool aOvernight = true );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericVector MACD( const Rcpp::NumericMatrix & aXts, const int aFastPeriod, const int aSlowPeriod, const int aSmoothPeriod, const int aType = 3 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::List Forecasting( const Rcpp::NumericMatrix & aXts, const int aForecastPeriod=100, const double aConfidenceIntervals=1.29, const int aType = 3 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::List RollMinMax( const Rcpp::NumericMatrix & aXts, const int aPeriod );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::NumericMatrix ConvertBars( const Rcpp::NumericMatrix & aXts, const int aPeriod=-1 );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
bool SaveData( const Rcpp::NumericMatrix & aOHLCV, const SEXP & aFileName );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
double DealsToPnLValue( const Rcpp::DataFrame & aDeals, const SEXP & aParams );
//------------------------------------------------------------------------------------------

// [[Rcpp::export]]
Rcpp::List DealsToCoeffUnrealized( const Rcpp::NumericMatrix & aBars, const Rcpp::DataFrame & aDeals, const SEXP & aParams );
//------------------------------------------------------------------------------------------


inline TPriceSeries XtsToPriceSeries( const Rcpp::NumericMatrix & lXts, const TMAPoint aType, std::string & aoTZone ) {
    const Rcpp::NumericVector lIndex( lXts.attr("index") );
    aoTZone.assign( Rcpp::as< std::string >( lXts.attr("tzone") ) );

    if( lXts.ncol() < 5 ) {
        throw std::logic_error( "Need Xts with OHLCV bars" );
    }

    TPriceSeries lResult( lXts.nrow() );

    if( aType == TMAPoint::MAP_Mid ) {
        for( int lRowNum = 0; lRowNum < lXts.nrow(); ++lRowNum ) {
            const TPrice lPrice = (
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_High ) ) +
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Low ) )
            ) / 2.0 ;
            const TSimpleTick lTick { lIndex[ lRowNum ], lPrice, lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Volume ) ) };
            lResult[ lRowNum ] = lTick;
        }

    } else if( aType == TMAPoint::MAP_Triple ) {
        for( int lRowNum = 0; lRowNum < lXts.nrow(); ++lRowNum ) {
            const TPrice lPrice =(
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_High ) ) +
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Low ) ) +
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Close ) )
            ) / 3.0 ;
            const TSimpleTick lTick { lIndex[ lRowNum ], lPrice, lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Volume ) ) };
            lResult[ lRowNum ] = lTick;
        }

    } else {
        for( int lRowNum = 0; lRowNum < lXts.nrow(); ++lRowNum ) {
            const TPrice lPrice =lXts.at( lRowNum, static_cast<int>( aType ) );
            const TSimpleTick lTick { lIndex[ lRowNum ], lPrice, lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Volume ) ) };
            lResult[ lRowNum ] = lTick;
        }
    }

    return lResult;
}
//------------------------------------------------------------------------------------------

inline Rcpp::NumericVector PriceSeriesToXts( const TPriceSeries & aPrices, const std::string & aTZone ) {
    Rcpp::NumericVector lResult( aPrices.size() );
    Rcpp::NumericVector lIndex( aPrices.size() );

    for( size_t i=0; i < aPrices.size(); ++i ) {
        lResult[ i ] = aPrices[i].Price;
        lIndex[ i ] = aPrices[i].DateTime;
    }
    
    lIndex.attr("tzone")    = aTZone.c_str();// "Europe/Moscow"; // the index has attributes
    lIndex.attr("tclass")   = "POSIXct";

    lResult.attr("dim")         = Rcpp::IntegerVector::create( aPrices.size(), 1 );
    lResult.attr("index")       = lIndex;
    Rcpp::CharacterVector klass = Rcpp::CharacterVector::create( "xts", "zoo" );
    lResult.attr("class")       = klass;
    lResult.attr(".indexCLASS") = "POSIXct";
    lResult.attr("tclass")      = "POSIXct";
    lResult.attr(".indexTZ")    = aTZone.c_str(); //"Europe/Moscow";
    lResult.attr("tzone")       = aTZone.c_str(); //"Europe/Moscow";

    colnames( lResult ) = Rcpp::CharacterVector::create( "value" );
    
    return lResult;
}
//------------------------------------------------------------------------------------------

inline TBarSeries XtsToBarSeries( const Rcpp::NumericMatrix & lXts ) {
    const int lDataSize = lXts.nrow();
    TBarSeries lResult( lDataSize );
    
    if( lDataSize > 0 ) {
        const Rcpp::NumericVector lIndex( lXts.attr("index") );

        for( int lRowNum = 0; lRowNum < lDataSize; ++lRowNum ) {
            TSimpleBar lBar {
                lIndex[ lRowNum ],
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Open ) ),
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_High ) ),
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Low ) ),
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Close ) ),
                lXts.at( lRowNum, static_cast<int>( TMAPoint::MAP_Volume ) )
            };

            lResult[ lRowNum ]= lBar;
        }
    }

    return lResult;
}
//------------------------------------------------------------------------------------------

inline TPriceSeries IndicatorToPriceSeries( const Rcpp::NumericMatrix & aData  ){
    const int lDataSize = aData.nrow();
    TPriceSeries lResult( lDataSize );
    
    if( lDataSize > 0 ) {
        const Rcpp::NumericVector lIndex( aData.attr("index") );

        for( int lRowNum = 0; lRowNum < lDataSize; ++lRowNum ) {
            const TSimpleTick lValue{
                lIndex[ lRowNum ],
                aData.at( lRowNum, 0 ),
                0
            };
            lResult[ lRowNum ]= lValue;
        }
    }

    return lResult;
}
//------------------------------------------------------------------------------------------

inline TDeals DataFrameToDeals( const Rcpp::DataFrame & aDeals, TPrice aFirstPrice = 0 ) {
    TDeals lResult;

    Rcpp::NumericVector lBP = Rcpp::as< Rcpp::NumericVector >( aDeals["open_time"] ) ;
    Rcpp::NumericVector lEP = Rcpp::as< Rcpp::NumericVector >( aDeals["close_time"] ) ;
    Rcpp::NumericVector lOpens = Rcpp::as< Rcpp::NumericVector >( aDeals["open_price"] ) ;
    Rcpp::NumericVector lCloses = Rcpp::as< Rcpp::NumericVector >( aDeals["close_price"] ) ;
    Rcpp::NumericVector lSide = Rcpp::as< Rcpp::NumericVector >( aDeals["side"] ) ;

    for( int i = 0; i < aDeals.nrows(); ++i ) {
        const Rcpp::Datetime lBeginPoint( lBP[i] );
        const Rcpp::Datetime lEndPoint( lEP[i] );
        
        const TInnerDate lOpenTime = lBeginPoint.getFractionalTimestamp();
        const TPrice lOpenPrice = lOpens[ i ];
        const TPrice lStopLoss = GetBadPrice();
        const TPrice lTakeProfit = GetBadPrice();
        const TPrice lClosePrice =  lCloses[ i ];
        const TInnerDate lCloseTime = lEndPoint.getFractionalTimestamp();
        const TDealSide lDealSide = ( lSide[ i ] > 0 ) ? TDealSide::Buy : TDealSide::Sell;
        
        lResult.push_back( { lOpenTime, lOpenPrice, lStopLoss, lTakeProfit, lClosePrice, lCloseTime, lDealSide  } );
    }

    return lResult;
}
//------------------------------------------------------------------------------------------

#endif //BACKTESTER_REXPORTFILE_H
