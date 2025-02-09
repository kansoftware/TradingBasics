cmake_minimum_required(VERSION 3.11)
project( TradingBasics)

option( BUILD_TESTING "BUILD_TESTING" OFF )
option( USER_PEDANT "Show me lots of warrnings" OFF )

set( CMAKE_CXX_STANDARD 14)
set( CMAKE_CXX_STANDARD_REQUIRED ON)

if( USER_PEDANT )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wlogical-op -Wconversion -Wno-unused-parameter -Wsuggest-attribute=noreturn -Wsuggest-override -Wold-style-cast" )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -gdwarf-2 -Og" )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Og" )
    message( WARNING "debug mode" )

else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0 -Ofast -fno-finite-math-only -DNDEBUG" )
    #message( WARNING "release mode" )

endif() 

find_package(Boost REQUIRED) #  COMPONENTS math

file( GLOB SRC_FILES "src/*.cpp")
add_library( ${PROJECT_NAME} SHARED ${SRC_FILES})

target_include_directories( ${PROJECT_NAME} PUBLIC "include")

set_target_properties( ${PROJECT_NAME} PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if( BUILD_TESTING )
    enable_testing()
    add_subdirectory( "gtests")
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
set( CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-math-dev")
set( CPACK_PACKAGE_DESCRIPTION "These classes, types and functions are suitable for creating trading strategies and bots using C++ compilers.")
set( CPACK_PACKAGE_CONTACT "Andrew KAN kan@kansoftware.ru")
#set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")

include( CPack)
