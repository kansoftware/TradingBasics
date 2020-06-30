#include <gtest/gtest.h>

#include "BasisOfStrategy.h"
#include "PnlAction.h"
#include "DelphisRound.h"
#include "VolatilityBuffer.h"
#include "VolatilityBarBuffer.h"

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
TEST( ITime, main ) {
    EXPECT_EQ( ITime(""), 0.0 );
    EXPECT_EQ( ITime("00:00"), 0.0 );
    EXPECT_EQ( ITime("10:00"), 36000.0 );
    EXPECT_EQ( ITime("10:01:01"), 36061.0 );
    EXPECT_EQ( ITime(1592826299), 42299.0 ); //Mon, 22 Jun 2020 11:44:59 GMT
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBuffer, main ) {
    TVolatilityBuffer<double> lbuf(5);
    EXPECT_FALSE( lbuf.isFill() );
    EXPECT_TRUE( std::isnan(lbuf.getMax()) );
    for (size_t i = 0; i < 4; i++) {
        EXPECT_FALSE( lbuf.add( ToDouble(i) ) );
        EXPECT_TRUE( std::isnan(lbuf.getErr()) );
    }
    EXPECT_TRUE( lbuf.add( ToDouble(4) ) );
    EXPECT_NEAR( lbuf.getErr(), 0.7559289, 0.0000001 );
    EXPECT_EQ( lbuf.getMin(), 0 );
    EXPECT_EQ( lbuf.getMax(), 4 );
    EXPECT_EQ( lbuf.getMean(), 2 );
}

//------------------------------------------------------------------------------------------
TEST( VolatilityBarBuffer, main ) {
    TVolatilityBarBuffer lbuf(5);
    EXPECT_FALSE( lbuf.isFill() );
    EXPECT_TRUE( std::isnan(lbuf.getMax()) );
    for (size_t i = 0; i < 4; i++) {
        EXPECT_FALSE( lbuf.add( TSimpleBar{gStartingTime+ToDouble(i),ToDouble(i),ToDouble(i),ToDouble(i),ToDouble(i),1.0} ) );
        EXPECT_TRUE( std::isnan(lbuf.getErr()) );
    }
    EXPECT_TRUE( lbuf.add( TSimpleBar{gStartingTime+4.0,4.0,4.0,4.0,4.0,1.0} ) );
    //EXPECT_NEAR( lbuf.getErr(), 0.7559289, 0.0000001 );
    EXPECT_EQ( lbuf.getMin(), 0 );
    EXPECT_EQ( lbuf.getMax(), 4 );
    EXPECT_EQ( lbuf.getMean(), 0 );
}

//------------------------------------------------------------------------------------------
