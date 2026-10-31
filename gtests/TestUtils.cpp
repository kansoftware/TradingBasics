/**
 * \file TestUtils.cpp
 * \brief gtest утилит: PointerValidater, Comparers, DelphisRound, Prices, StringHasher, FileManipulator
 *
 * Базовые случаи Comparers/Prices/Round покрыты в TestBasic.cpp, StringHasher —
 * в TestStringHasher.cpp; здесь — границы, отсутсвующие там.
 */
#include <gtest/gtest.h>

#include <cctype>
#include <memory>
#include <sstream>

#include "PointerValidater.h"
#include "Comparers.h"
#include "DelphisRound.h"
#include "Prices.h"
#include "StringHasher.h"
#include "FileManipulator.h"

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
    EXPECT_FALSE( isValidPointer( static_cast< TBase * >( nullptr ) ) );
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
// Comparers: границы gAbsoluteZero = 1e-9 (строгие неравенства — открытый интервал)
//------------------------------------------------------------------------------------------
TEST( TestUtils, ComparersAbsoluteZeroBounds ) {
    // isZero — открытый интервал (-eps, eps): сами границы не считаются нулём
    EXPECT_TRUE( isZero( 0.0 ) );
    EXPECT_TRUE( isZero( 1e-10 ) );
    EXPECT_TRUE( isZero( -1e-10 ) );
    EXPECT_FALSE( isZero( gAbsoluteZero ) );
    EXPECT_FALSE( isZero( -gAbsoluteZero ) );
    EXPECT_FALSE( isZero( 2e-9 ) );
    EXPECT_FALSE( isZero( -2e-9 ) );

    // IsEqual/IsNotEqual через ту же эпсилон-зону
    EXPECT_TRUE( IsEqual( 1e-10, 0.0 ) );
    EXPECT_FALSE( IsEqual( 1e-9, 0.0 ) );
    EXPECT_FALSE( IsEqual( -2e-9, 0.0 ) );
    EXPECT_FALSE( IsNotEqual( 1e-10, 0.0 ) );
    EXPECT_TRUE( IsNotEqual( 2e-9, 0.0 ) );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, ComparersGreatLessBounds ) {
    // IsGreat: A - B > eps (строго); разность ровно eps — ещё не "больше"
    EXPECT_FALSE( IsGreat( 1e-9, 0.0 ) );
    EXPECT_TRUE( IsGreat( 2e-9, 0.0 ) );
    EXPECT_FALSE( IsGreat( 1e-10, 0.0 ) );
    EXPECT_TRUE( IsGreat( 0.0, -2e-9 ) );

    // IsLess: B - A > eps (строго), симметрично
    EXPECT_FALSE( IsLess( 0.0, 1e-9 ) );
    EXPECT_TRUE( IsLess( 0.0, 2e-9 ) );
    EXPECT_FALSE( IsLess( 0.0, 1e-10 ) );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, ComparersIsPositiveAndSign ) {
    // isPositiveValue: строго больше eps (не ">= 0")
    EXPECT_FALSE( isPositiveValue( 1e-9 ) );
    EXPECT_TRUE( isPositiveValue( 2e-9 ) );
    EXPECT_FALSE( isPositiveValue( 1e-10 ) );
    EXPECT_FALSE( isPositiveValue( -1e-10 ) );

    // Sign: 1 / -1 / 0 с той же эпсилон-зоной
    EXPECT_EQ( Sign( 2e-9 ), 1 );
    EXPECT_EQ( Sign( -2e-9 ), -1 );
    EXPECT_EQ( Sign( 1e-10 ), 0 );
    EXPECT_EQ( Sign( -1e-10 ), 0 );
    EXPECT_EQ( Sign( 0.0 ), 0 );
    EXPECT_EQ( Sign( 5 ), 1 );
    EXPECT_EQ( Sign( -5 ), -1 );
    EXPECT_EQ( Sign( 0 ), 0 );
}

//------------------------------------------------------------------------------------------
// DelphisRound: ToSize_t/ToInt/Round/Ceil/Trunc и size_t-варианты.
// Функции собраны парами "NDEBUG-макрос / debug-шаблон" — различаются только
// assert'ами, поэтому проверяются лишь семантики, одинаковые в обоих режимах.
//------------------------------------------------------------------------------------------
TEST( TestUtils, ToSize_tNegativeAndTrunc ) {
    EXPECT_EQ( ToSize_t( -5.0 ), 0U );
    EXPECT_EQ( ToSize_t( -0.9 ), 0U );
    EXPECT_EQ( ToSize_t( 0.0 ), 0U );
    EXPECT_EQ( ToSize_t( 3.7 ), 3U ); // усечение, не округление
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, ToIntTruncTowardZero ) {
    EXPECT_EQ( ToInt( 5.7 ), 5 );
    EXPECT_EQ( ToInt( -5.7 ), -5 ); // static_cast<int> усекает к нулю
    EXPECT_EQ( ToInt( 5 ), 5 );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, RoundAwayFromZero ) {
    // std::round округляет "от нуля" (0.5 -> 1, -0.5 -> -1), в отличие от
    // банковского округления Delphi — факт зафиксирован намеренно.
    EXPECT_EQ( Round( 2.5 ), 3 );
    EXPECT_EQ( Round( -2.5 ), -3 );
    EXPECT_EQ( Round( 0.5 ), 1 );
    EXPECT_EQ( Round( -0.5 ), -1 );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, RoundToSize_tBounds ) {
    EXPECT_EQ( RoundToSize_t( 0.5 ), 1U );
    EXPECT_EQ( RoundToSize_t( -0.5 ), 0U ); // отрицательное -> 0
    EXPECT_EQ( RoundToSize_t( 2.5 ), 3U );
    EXPECT_EQ( RoundToSize_t( 0.0 ), 0U );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, CeilTruncToSize_tNegativeIsZero ) {
    EXPECT_EQ( CeilToSize_t( 0.5 ), 1U );
    EXPECT_EQ( CeilToSize_t( 2.1 ), 3U );
    EXPECT_EQ( CeilToSize_t( -1.9 ), 0U );

    EXPECT_EQ( TruncToSize_t( 2.7 ), 2U );
    EXPECT_EQ( TruncToSize_t( 0.9 ), 0U );
    EXPECT_EQ( TruncToSize_t( -2.7 ), 0U );
}

//------------------------------------------------------------------------------------------
// Prices: границы валидности — открытый интервал (gMinPrice, gMaxPrice)
//------------------------------------------------------------------------------------------
TEST( TestUtils, PricesBounds ) {
    // сами границы невалидны (интервал открытый)
    EXPECT_FALSE( IsValidPrice( gMinPrice ) );
    EXPECT_FALSE( IsValidPrice( gMaxPrice ) );
    EXPECT_TRUE( IsValidPrice( 1.0 ) );

    // sentinel: bad-цена отрицательна и невалидна
    EXPECT_EQ( GetBadPrice(), -gMaxPrice );
    EXPECT_FALSE( IsValidPrice( GetBadPrice() ) );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, PricesSpreadBounds ) {
    // спред: открытый интервал (-gSpreadValueLimit, gSpreadValueLimit)
    EXPECT_TRUE( isValidSpreadValue( 0.0 ) );
    EXPECT_TRUE( isValidSpreadValue( gSpreadValueLimit - 1.0 ) );
    EXPECT_FALSE( isValidSpreadValue( gSpreadValueLimit ) );
    EXPECT_FALSE( isValidSpreadValue( -gSpreadValueLimit ) );
}

//------------------------------------------------------------------------------------------
// StringHasher: алфавит [0-9a-z_] (36 символов, без uppercase — наследие binance)
//------------------------------------------------------------------------------------------
TEST( TestUtils, StringHasherAlphabet ) {
    EXPECT_EQ( GetSymbolsCount(), 36U );

    EXPECT_EQ( GetSymbolByIdx( 0 ), '0' );
    EXPECT_EQ( GetSymbolByIdx( 9 ), '9' );

    for( size_t i = 0; i < GetSymbolsCount(); ++i ) {
        const char lSymbol = GetSymbolByIdx( i );
        EXPECT_TRUE( ( lSymbol >= '0' and lSymbol <= '9' ) or ( lSymbol >= 'a' and lSymbol <= 'z' ) )
            << "index " << i;
        EXPECT_FALSE( isupper( lSymbol ) ) << "uppercase at index " << i;
    }

    // вне диапазона — служебный '_'
    EXPECT_EQ( GetSymbolByIdx( GetSymbolsCount() ), '_' );
    EXPECT_EQ( GetSymbolByIdx( 100 ), '_' );
}

//------------------------------------------------------------------------------------------
TEST( TestUtils, IntToCodeBoundaries ) {
    // <=0 вырождается в "0"
    EXPECT_EQ( IntToCode( 0 ), "0" );
    EXPECT_EQ( IntToCode( -5 ), "0" );

    // base-36 по алфавиту gSymbols: индекс 35 — последний символ 'm'
    EXPECT_EQ( IntToCode( 1 ), "1" );
    EXPECT_EQ( IntToCode( 35 ), "m" );
    EXPECT_EQ( IntToCode( 36 ), "10" ); // перенос разряда
}

//------------------------------------------------------------------------------------------
namespace {

/// Обратная сборка значения из кода: позиционное base-36 по gSymbols
/// (инверсия GetSymbolByIdx). Обратного CodeToInt в модуле нет.
long CodeToIntInverse( const std::string & aCode ) {
    long lValue = 0;
    for( const char lSymbol : aCode ) {
        size_t lIdx = 0;
        while( lIdx < GetSymbolsCount() and GetSymbolByIdx( lIdx ) != lSymbol ) { ++lIdx; }
        EXPECT_LT( lIdx, GetSymbolsCount() ) << "symbol out of alphabet: " << lSymbol;
        lValue = lValue * static_cast< long >( GetSymbolsCount() ) + static_cast< long >( lIdx );
    }
    return lValue;
}

} // namespace

TEST( TestUtils, IntToCodeLargeNumber ) {
    // Обратного преобразования нет (CodeToInt отсутствует) — проверяем
    // реконструкцией значения из строки (позиционное base-36 по алфавиту).
    // Диапазон после отказа от ToInt в цикле — весь long (> INT_MAX допустим).
    const long lValues[] = { 1234567890L, 2147483648L, 1125899906842624L };

    for( const long lValue : lValues ) {
        const std::string lCode = IntToCode( lValue );
        EXPECT_FALSE( lCode.empty() );

        for( const char lSymbol : lCode ) {
            EXPECT_TRUE( ( lSymbol >= '0' and lSymbol <= '9' ) or ( lSymbol >= 'a' and lSymbol <= 'z' ) )
                << "symbol " << lSymbol;
        }

        EXPECT_EQ( CodeToIntInverse( lCode ), lValue ) << "value " << lValue;
    }

    // Конвенция <= 0 -> "0" сохраняется
    EXPECT_EQ( CodeToIntInverse( IntToCode( 0 ) ), 0L );
    EXPECT_EQ( IntToCode( -5 ), "0" );
}

//------------------------------------------------------------------------------------------
// FileManipulator: сырой бинарный IO через reinterpret_cast (см. доку заголовка)
//------------------------------------------------------------------------------------------
namespace {

/// POD-структура для roundtrip-теста (только тривиально копируемые поля)
struct TPodRecord {
    int mId;
    double mValue;
    char mTag;
};

} // namespace

TEST( TestUtils, FileManipulatorPodRoundtrip ) {
    const TPodRecord lSource{ 42, 3.14159, 'x' };

    std::stringstream lStream( std::ios::in | std::ios::out | std::ios::binary );
    lStream << TWrapFileData< TPodRecord >( lSource );
    EXPECT_TRUE( lStream.good() );

    TWrapFileData< TPodRecord > lWrap; // нулевая инициализация поля
    lStream >> lWrap;
    EXPECT_TRUE( lStream.good() );

    const TPodRecord lRead = lWrap; // неявное преобразование к T const&
    EXPECT_EQ( lRead.mId, 42 );
    EXPECT_EQ( lRead.mValue, 3.14159 );
    EXPECT_EQ( lRead.mTag, 'x' );
}

//------------------------------------------------------------------------------------------
