
PROJECT(wxMusik)

SUBDIRS(data locale 3rd-Party MUSIKEngine src)
# Configure for a wxWindows build
INCLUDE(${CMAKE_ROOT}/Modules/FindwxWindows.cmake)
INCLUDE (${wxMusik_SOURCE_DIR}/scripts/cmake/TargetDistclean.cmake)

IF(WXWINDOWS_INCLUDE_PATH)
    INCLUDE_DIRECTORIES(${WXWINDOWS_INCLUDE_PATH})
ENDIF(WXWINDOWS_INCLUDE_PATH)

IF(CMAKE_WX_CXX_FLAGS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_WX_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
ENDIF(CMAKE_WX_CXX_FLAGS)

# musepack library
FIND_LIBRARY(MUSEPACK_LIBRARY NAMES musepack PATHS /usr/lib /usr/local/lib)
# monkeys audio library
FIND_LIBRARY(MAC_LIBRARY NAMES mac PATHS /usr/lib /usr/local/lib)

# FLAC library
FIND_LIBRARY(FLAC_LIBRARY NAMES FLAC PATHS /usr/lib /usr/local/lib)

# SQLite library
FIND_LIBRARY(SQLITE_LIBRARY NAMES sqlite PATHS /usr/lib /usr/local/lib)


# fmod library
FIND_LIBRARY(FMOD_LIBRARY NAMES fmod-3.74 fmod PATHS /usr/lib /usr/local/lib)

# Vorbis library
#FIND_LIBRARY(VORBIS_LIBRARY NAMES vorbis PATHS /usr/lib /usr/local/lib)


INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party/taglib/toolkit")
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/MUSIKEngine")


# Set build variables
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ansi ")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -pipe -fomit-frame-pointer")

#SET(MyExecutable_PATH ${EXECUTABLE_OUTPUT_PATH})

#IF(NOT MyExecutable_PATH)
# SET(MyExecutable_PATH ${CMAKE_CURRENT_BINARY_DIR})
#ENDIF(NOT MyExecutable_PATH)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
SET(MyExecutable_PATH ${EXECUTABLE_OUTPUT_PATH})