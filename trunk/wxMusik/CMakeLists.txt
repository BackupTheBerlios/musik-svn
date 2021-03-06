cmake_minimum_required(VERSION 2.4)
PROJECT(wxMusik)

INCLUDE (${wxMusik_SOURCE_DIR}/scripts/cmake/TargetDistclean.cmake)
IF(NOT WIN32 )
IF(NOT APPLE)
#SET(CMAKE_CXX_FLAGS " `pkg-config  --cflags-only-I gtk+-2.0`  ${CMAKE_CXX_FLAGS}")
find_package(PkgConfig)
pkg_check_modules(MYGTK2 gtk+-2.0 )
ADD_DEFINITIONS(${MYGTK2_CFLAGS})
ENDIF(NOT APPLE)
ENDIF(NOT WIN32)

INCLUDE (TestBigEndian)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

# Configure for a wxWindows build
find_package(wxWidgets COMPONENTS base core adv html net qa xml)
if(wxWidgets_FOUND)
	INCLUDE(${wxWidgets_USE_FILE})
else(wxWidgets_FOUND)
    message("ERROR: wxWidgets not found!")
endif(wxWidgets_FOUND)

# Vorbis library
#FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbis PATHS /usr/lib /usr/local/lib)


INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib/toolkit")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib/mpeg/id3v2")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/MUSIKEngine")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/sqlite3")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/sqlite2")



# Set build variables
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ansi ")
IF(APPLE)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ppc -arch i386 ")
IF(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -pipe -fomit-frame-pointer  -march=pentium-m -mtune=prescott")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -pipe -fomit-frame-pointer  -march=pentium-m -mtune=prescott")
ELSE(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -faltivec")
ENDIF(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
ENDIF(APPLE)
IF(UNIX)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -pipe -fomit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -pipe -fomit-frame-pointer")
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
SET(MyExecutable_PATH ${EXECUTABLE_OUTPUT_PATH})

SUBDIRS(data locale 3rd-Party MUSIKEngine src)

