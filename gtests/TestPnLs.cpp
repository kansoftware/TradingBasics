#include <gtest/gtest.h>

#include "BasisOfStrategy.h"
#include "PnlAction.h"
#include "DelphisRound.h"

//------------------------------------------------------------------------------------------
TPriceSeries GetSimplePnl(){
    TPriceSeries lPnLs( 10 );

    for( size_t i=0; i<lPnLs.size(); ++i ) {
        TSimpleTick lTick {
            gStartingTime + ToDouble(i)*gOneDay,
            1.0,
            2.0
        };

        lPnLs[ i ] = lTick;
    }
    
    return lPnLs;
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsToMoneyResult ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    
    EXPECT_EQ( PnLsToMoneyResult( lPnLs, false ), 10.0 );
    EXPECT_EQ( PnLsToMoneyResult( lPnLs, true ), 20.0 );
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnlsToDaily ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    const TPriceSeries lPnls_Daily( PnlsToDaily(lPnLs) );
    
    EXPECT_EQ( lPnLs.size(), lPnls_Daily.size() );
    
    double lCumPnl = 0.0;
    for( size_t i = 0; i<lPnLs.size(); ++i ){
        lCumPnl += lPnLs[i].Price*lPnLs[i].Volume;
        EXPECT_EQ( lCumPnl, lPnls_Daily[i].Price );
        EXPECT_EQ( lPnls_Daily[i].Volume, 1.0 );
    }
    
}

//------------------------------------------------------------------------------------------
TEST( PnLs, PnLsAmplifier ) {

    const TPriceSeries lPnLs( GetSimplePnl() );
    
    const TPriceSeries lPnLs_amp( PnLsAmplifier( lPnLs, {1,2,4} ) );
    EXPECT_EQ( lPnLs_amp.size(), 22 );
    EXPECT_GE( lPnLs_amp.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp2( PnLsAmplifier( lPnLs, {1,2,4}, gStartingTime, gStartingTime + gOneDay * 10.0 ) );
    EXPECT_EQ( lPnLs_amp2.size(), 22 );
    EXPECT_GE( lPnLs_amp2.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp2.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp2.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp3( PnLsAmplifier( lPnLs, {1,2,4}, gStartingTime - gOneDay * 3.0, gStartingTime + gOneDay * 14.0 ) );
    EXPECT_EQ( lPnLs_amp3.size(), 21 );
    EXPECT_GE( lPnLs_amp3.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp3.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp3.back().Price, lPnLs.back().Price );
    
    
    const TPriceSeries lPnLs_amp4( PnLsAmplifier( lPnLs, {1,1,1} ) );
    EXPECT_EQ( lPnLs_amp4.size(), lPnLs.size() );
    EXPECT_EQ( lPnLs_amp4.front().Price, lPnLs.front().Price );
    EXPECT_EQ( lPnLs_amp4.back().Price, lPnLs.back().Price );
    
//    for( const TSimpleTick& lTick : lPnLs_amp4 ){
//        std::cout << lTick << std::endl;
//    }
}

//------------------------------------------------------------------------------------------

