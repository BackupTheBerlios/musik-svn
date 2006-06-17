IF(OPTION_SUPPORT_FLAC)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
SUBDIRS(libFLAC plugin_common grabbag replaygain_synthesis replaygain_analysis)
ENDIF(OPTION_SUPPORT_FLAC)
