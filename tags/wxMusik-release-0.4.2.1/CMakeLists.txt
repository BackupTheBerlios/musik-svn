
PROJECT(wxMusik)
SUBDIRS(data locale 3rd-Party MUSIKEngine src)
INCLUDE (${wxMusik_SOURCE_DIR}/scripts/cmake/TargetDistclean.cmake)

IF(NOT WIN32 )
IF(NOT APPLE)
SET(CMAKE_CXX_FLAGS " `pkg-config  --cflags-only-I gtk+-2.0`  ${CMAKE_CXX_FLAGS}")
ENDIF(NOT APPLE)
ENDIF(NOT WIN32)
# Configure for a wxWindows build
INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/MyUse_wxWindows.cmake)


# SQLite library
FIND_LIBRARY(SQLITE_LIBRARY NAMES sqlite PATHS /usr/lib /usr/local/lib)



# Vorbis library
#FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbis PATHS /usr/lib /usr/local/lib)


INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib/toolkit")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib/mpeg/id3v2")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/MUSIKEngine")


# Set build variables
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ansi ")
IF(APPLE)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -faltivec")
ENDIF(APPLE)
IF(UNIX)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -pipe -fomit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -pipe -fomit-frame-pointer")
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
SET(MyExecutable_PATH ${EXECUTABLE_OUTPUT_PATH})

