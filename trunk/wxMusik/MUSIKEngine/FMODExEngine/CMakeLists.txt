SUBDIRS(src)

IF(USE_FMODEX)
# fmodex library
INCLUDE_DIRECTORIES("${wxMusik_SOURCE_DIR}/3rd-Party")


IF(APPLE)
IF(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
   FIND_LIBRARY(FMOD_LIBRARY NAMES fmodex  PATHS ${wxMusik_SOURCE_DIR}/3rd-Party/bin/fmodex/MacOs/intel  /usr/lib /usr/local/lib)
ELSE(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
   FIND_LIBRARY(FMOD_LIBRARY NAMES fmodex  PATHS ${wxMusik_SOURCE_DIR}/3rd-Party/bin/fmodex/MacOs/ppc  /usr/lib /usr/local/lib)
ENDIF(CMAKE_OSX_ARCHITECTURES STREQUAL i386)
ENDIF(APPLE)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
	if (CMAKE_SIZEOF_VOID_P MATCHES "8")
		FIND_LIBRARY(FMOD_LIBRARY NAMES fmodex64  PATHS ${wxMusik_SOURCE_DIR}/3rd-Party/bin/fmodex/linux )
	ELSE  (CMAKE_SIZEOF_VOID_P MATCHES "8") 
		FIND_LIBRARY(FMOD_LIBRARY NAMES fmodex  PATHS ${wxMusik_SOURCE_DIR}/3rd-Party/bin/fmodex/linux )
	ENDIF  (CMAKE_SIZEOF_VOID_P MATCHES "8") 
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
ELSE(USE_FMODEX)
SET(FMOD_LIBRARY "")
ENDIF(USE_FMODEX)
