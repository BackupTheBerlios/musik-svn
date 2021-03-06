INCLUDE(CheckIncludeFileCXX)

IF(APPLE)
    SET(USE_FMODEX "ON")
    ADD_DEFINITIONS(-DUSE_FMODEX -DAPPLE)
    SET(MUSIK_ENGINE "FMODExEngine")
#
#ADD_DEFINITIONS(-DMUSIKENGINE_DO_NOT_USE_OWN_DECODERS -DUSE_AUDIOQUEUE -DAPPLE)
#SET(MUSIK_ENGINE "AudioQueueEngine")
#SET(APPLE_FRAMEWORKS ${APPLE_FRAMEWORKS} "-framework AudioToolbox")
ELSE(APPLE)
#CHECK_INCLUDE_FILE_CXX("fmodex/fmod.h" USE_FMODEX)
    SET(USE_FMODEX "ON")
    ADD_DEFINITIONS(-DUSE_FMODEX )
    SET(MUSIK_ENGINE "FMODExEngine")
ENDIF(APPLE)
IF(${USE_FMODEX})
    ADD_DEFINITIONS(-DUSE_FMODEX)
    SET(MUSIK_ENGINE "FMODExEngine")
ELSE(${USE_FMODEX})
    CHECK_INCLUDE_FILE_CXX("fmod.h" USE_FMOD3)
    IF(${USE_FMOD3})
        ADD_DEFINITIONS(-DUSE_FMOD3)
	SET(MUSIK_ENGINE "FMODEngine")
    ENDIF(${USE_FMOD3})
ENDIF(${USE_FMODEX})
