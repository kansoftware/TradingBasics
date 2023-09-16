cmake_minimum_required( VERSION 3.7 )
project( TradingBasics VERSION "0.0.1")

find_package(Boost 1.50 REQUIRED)
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -Wlogical-op -Wconversion -Wno-unused-parameter -Wsuggest-attribute=noreturn -Wsuggest-override -Wold-style-cast" )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -gdwarf-2" )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1" )
    
    message( WARNING "debug mode" )

else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fno-finite-math-only -DNDEBUG" )

    message( WARNING "release mode" )

endif() 

file( GLOB SRC_FILES
    "src/*.cpp"
)
# set( SRC_FILES
#      src/BarPeriod.cpp
#      src/BasisOfStrategy.cpp
#      src/CIndicatorES.cpp
#      src/Forecasting.cpp
#      src/Indicators.cpp
#      src/PnlAction.cpp
#      src/PowellsMethod.cpp
#      src/StringHasher.cpp
# )
add_library( ${PROJECT_NAME} SHARED ${SRC_FILES})

target_include_directories( ${PROJECT_NAME} PUBLIC "include" )

set_target_properties( ${PROJECT_NAME} PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if( BUILD_TESTING )
    enable_testing()
    add_subdirectory( "gtests" )
endif()

INSTALL( TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kansoftware)

INSTALL( DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
    DESTINATION /usr/include/kansoftware
    FILES_MATCHING
    PATTERN "*.h"
)

set( CPACK_GENERATOR "DEB")
set( CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set( CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set( CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
#set( CPACK_DEBIAN_PACKAGE_DEPENDS " ... deps none ... ")
set( CPACK_PACKAGE_DESCRIPTION "IBPP, where the 'PP' stands for '++', is a C ++ client interface for Firebird SQL")
set( CPACK_PACKAGE_CONTACT "Andrew KAN kan@kansoftware.ru")
#set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")

include( CPack)
