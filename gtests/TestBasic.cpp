#include <gtest/gtest.h>

#include "Comparers.h"
#include "DelphisRound.h"
#include "Prices.h"

//------------------------------------------------------------------------------------------
TEST( Basic, testCompare ) {
    EXPECT_TRUE( IsGreat( 5.6, 5.5 ) );
    EXPECT_FALSE( IsGreat( 5.5, 5.5 ) );
    EXPECT_FALSE( IsGreat( 5.5, 5.6 ) );
    
    
    EXPECT_FALSE( IsLess( 5.6, 5.5 ) );
    EXPECT_FALSE( IsLess( 5.5, 5.5 ) );
    EXPECT_TRUE( IsLess( 5.5, 5.6 ) );
    
    EXPECT_FALSE( IsNotEqual( 5.5, 5.5 ) );
    EXPECT_TRUE( IsNotEqual( 5.5, 5.6 ) );
    
    EXPECT_FALSE( IsEqual( 5.5, 5.6 ) );
    EXPECT_TRUE( IsEqual( 5.5, 5.5 ) );
    EXPECT_TRUE( IsEqual( 5.5, 5.5000001 ) );
}

//------------------------------------------------------------------------------------------
TEST( Basic, testPrice ) {
    EXPECT_TRUE( IsValidPrice( 5.5 ) );
    EXPECT_TRUE( IsValidPrice( 0.02 ) );
    EXPECT_TRUE( IsValidPrice( 300000.0 ) );
    
    EXPECT_FALSE( IsValidPrice( 0.0 ) );
    EXPECT_FALSE( IsValidPrice( -1.0 ) );
}

//------------------------------------------------------------------------------------------
TEST( Basic, testSpread ) {
    EXPECT_TRUE( isValidSpreadValue( 55.5 ) );
    EXPECT_TRUE( isValidSpreadValue( 0.0 ) );
    EXPECT_TRUE( isValidSpreadValue( -5.5 ) );
    
    EXPECT_FALSE( isValidSpreadValue( 100000.0 ) );
    EXPECT_FALSE( isValidSpreadValue( -100000.0 ) );
}

//------------------------------------------------------------------------------------------
TEST( Basic, testRound ) {
    EXPECT_EQ( Round( 5.6 ), 6 );
    EXPECT_EQ( Round( 5.4 ), 5 );
    EXPECT_EQ( Round( -5.6 ), -6 );
    EXPECT_EQ( Round( -5.4 ), -5 );
    
    EXPECT_EQ( Ceil( 5.6 ), 6 );
    EXPECT_EQ( Ceil( 5.4 ), 6 );
    EXPECT_EQ( Ceil( -5.6 ), -5 );
    EXPECT_EQ( Ceil( -5.4 ), -5 );
    
    EXPECT_EQ( Trunc( 5.6 ), 5 );
    EXPECT_EQ( Trunc( 5.4 ), 5 );
    EXPECT_EQ( Trunc( -5.6 ), -6 );
    EXPECT_EQ( Trunc( -5.4 ), -6 );
    
    EXPECT_TRUE( isPositiveValue( 55.5 ) );
    EXPECT_TRUE( isPositiveValue( 0.00005 ) );
    EXPECT_FALSE( isPositiveValue( 0.0 ) );
    EXPECT_FALSE( isPositiveValue( -10.0 ) );
}

//------------------------------------------------------------------------------------------
