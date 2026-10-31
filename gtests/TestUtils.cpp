/**
 * \file TestUtils.cpp
 * \brief gtest утилит: PointerValidater, Comparers, DelphisRound, Prices, StringHasher, FileManipulator
 *
 * Базовые случаи Comparers/Prices/Round покрыты в TestBasic.cpp, StringHasher —
 * в TestStringHasher.cpp; здесь — границы, отсутсвующие там.
 */
#include <gtest/gtest.h>

#include <memory>
#include <sstream>

#include "PointerValidater.h"
#include "Comparers.h"

//------------------------------------------------------------------------------------------
namespace {

struct TDummy {};

struct TBase { virtual ~TBase() = default; };
struct TDerived : TBase {};

/// Счётчик деструкторов для проверки FreeAndNil
struct TCounted {
    static int sDtorCount;
    ~TCounted() { ++sDtorCount; }
};
int TCounted::sDtorCount = 0;

} // namespace

//------------------------------------------------------------------------------------------
TEST( TestUtils, IsValidPointerRaw ) {
    int lValue = 0;
    EXPECT_TRUE( isValidPointer( &lValue ) );
    int * lNullInt = nullptr;
    EXPECT_FALSE( isValidPointer( lNullInt ) );

    const TDummy * lDummy = new TDummy;
    EXPECT_TRUE( isValidPointer( lDummy ) );
    delete lDummy;

    TDerived lDerived;
    TBase * lPolymorph = &lDerived;
    EXPECT_TRUE( isValidPointer( lPolymorph ) );
    EXPECT_TRUE( isValidPointer( static_cast< TBase * >( nullptr ) ) == false );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, IsValidPointerVoidAndNullptr ) {
    EXPECT_FALSE( isValidPointer( static_cast< const void * >( nullptr ) ) );

    int lValue = 0;
    const void * lVoid = &lValue;
    EXPECT_TRUE( isValidPointer( lVoid ) );

    // nullptr не выводит T шаблонной перегрузки — выбирается перегрузка const void*
    EXPECT_FALSE( isValidPointer( nullptr ) );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, IsValidPointerShared ) {
    std::shared_ptr< int > lEmpty;
    EXPECT_FALSE( isValidPointer( lEmpty ) );

    std::shared_ptr< int > lValid( new int( 5 ) );
    EXPECT_TRUE( isValidPointer( lValid ) );

    std::shared_ptr< TDummy > lReset( new TDummy );
    lReset.reset();
    EXPECT_FALSE( isValidPointer( lReset ) );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, IsValidPointerConstexpr ) {
    // constexpr-контекст: литерал nullptr и адрес статического объекта
    static_assert( not isValidPointer( nullptr ), "nullptr is invalid" );
    static int lStatic = 0;
    constexpr bool lOk = isValidPointer( &lStatic );
    EXPECT_TRUE( lOk );

    // shared_ptr-перегрузка тоже constexpr (вычисляется в compile-time для литерала)
    static_assert( isValidPointer( std::shared_ptr< int >() ) == false, "empty shared_ptr is invalid" );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, FreeAndNilDeletesAndNulls ) {
    TCounted::sDtorCount = 0;

    TCounted * lPtr = new TCounted;
    EXPECT_EQ( TCounted::sDtorCount, 0 );

    FreeAndNil( lPtr );
    EXPECT_EQ( lPtr, nullptr );
    EXPECT_EQ( TCounted::sDtorCount, 1 );

    // нулевой указатель — безопасно, деструктор не вызывается
    FreeAndNil( lPtr );
    EXPECT_EQ( lPtr, nullptr );
    EXPECT_EQ( TCounted::sDtorCount, 1 );

    // повторное удаление через второй указатель
    TCounted * lSecond = new TCounted;
    FreeAndNil( lSecond );
    EXPECT_EQ( TCounted::sDtorCount, 2 );
}

//------------------------------------------------------------------------------------------
