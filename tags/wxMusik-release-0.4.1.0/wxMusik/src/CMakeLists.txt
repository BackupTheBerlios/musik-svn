SET(PROJECT_VERSION_MAJOR "0.4.1")

SET(MAIN_SRCS
        ./MusikApp
        ./MusikGlobals
        ./MusikUtils
	    ./MusikAboutDlg
        ./Classes/ActivityBox
        ./Classes/ActivityAreaCtrl
        ./Classes/BtnDown
        ./Classes/GaugeSeek
        ./Classes/MusikLibrary
        ./Classes/MusikPlayer
        ./Classes/MusikPrefs
        ./Classes/NowPlayingCtrl
        ./Classes/MusikListCtrl
        ./Classes/PlaylistCtrl
	    ./Classes/PlaylistInfoCtrl
        ./Classes/SourcesBox
        ./Classes/WebServer
        ./Classes/MusikFX
        ./Classes/MusikEQCtrl
        ./Classes/MusikEQGauge
        ./Classes/Tunage
        ./Classes/PictureBox
		
        ./Classes/Library/TagLibInfo
        ./Classes/Library/CFMODInfo
        ./Classes/Library/APEInfo
        ./Classes/Library/MetaDataHandler
	    ${wxMusik_SOURCE_DIR}/3rd-Party/Bitap/libbitap.c
	    ${wxMusik_SOURCE_DIR}/3rd-Party/TagHelper/idtag.cpp
		
        ./Frames/MusikFrame
        ./Frames/MusikFrameEvents
        ./Frames/MusikLibraryFrame
        ./Frames/MusikPrefsFrame
        ./Frames/MusikTagFrame
        ./Frames/MusikVolumeFrame
        ./Frames/MusikFXFrame
	    ./Frames/MusikAutoTaggerFrame
 	    ./Threads/ActivityBoxThreads
        ./Threads/PlaylistCtrlThreads
        ./Threads/MusikLibraryThreads
        ./Threads/MusikTagThreads
        ./Threads/MusikThreads

        ./shibatch/Equ.cpp
        ./shibatch/Fftsg_fl.cpp
    )
IF(OPTION_SUPPORT_FLAC)
SET(FLAC_ADDITIONAL_LIBS "plugin_common grabbag  replaygain_synthesis")
SEPARATE_ARGUMENTS(FLAC_ADDITIONAL_LIBS)
ELSE(OPTION_SUPPORT_FLAC)
SET(FLAC_ADDITIONAL_LIBS "")
ENDIF(OPTION_SUPPORT_FLAC)

IF(WIN32)
    SET(WIN32_EXECUTABLE WIN32)
ENDIF(WIN32)


ADD_EXECUTABLE(wxMusik ${WIN32_EXECUTABLE} ${MAIN_SRCS})

TARGET_LINK_LIBRARIES(wxMusik
        FMODEngine
        MUSIKEngine
        ${WXWINDOWS_LIBRARY}
        ${SQLITE_LIBRARY}
        ${MUSEPACK_LIBRARY}
        ${MAC_LIBRARY}
        ${FLAC_LIBRARY}
        ${FMOD_LIBRARY}
        TagLib
        ${FLAC_ADDITIONAL_LIBS}
    )


  IF(APPLE)
    SET(MACOSX_BUNDLE_EXECUTABLE_NAME wxMusik)
    SET(MACOSX_BUNDLE_INFO_STRING "wxMusik version ${PROJECT_VERSION_MAJOR}, (c) Gunnar Roth")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "roth.gunnar.wxMusik")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION_MAJOR}, (c) 2004 Gunnar  Roth")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "wxMusik")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION_MAJOR}")
    SET(MACOSX_BUNDLE_ICON_FILE "wxMusik.icns")
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright 2004 Gunnar Roth")
    CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/scripts/cmake/MacOSXBundleInfo.plist.in
                   ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Info.plist)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/images/${MACOSX_BUNDLE_ICON_FILE}
		   ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE} COPYONLY)
    MAKE_DIRECTORY(${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/MacOS)
    WRITE_FILE(${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/PkgInfo "APPL????")
    ADD_EXECUTABLE(wxMusik.app/Contents/MacOS/wxMusik  ${MAIN_SRCS})

    TARGET_LINK_LIBRARIES(wxMusik.app/Contents/MacOS/wxMusik
        FMODEngine
        MUSIKEngine
        ${WXWINDOWS_LIBRARY}
        ${SQLITE_LIBRARY}
        ${MUSEPACK_LIBRARY}
        ${MAC_LIBRARY}
        ${FLAC_LIBRARY}
        ${FMOD_LIBRARY}
        TagLib
        ${FLAC_ADDITIONAL_LIBS}
    )
    ADD_CUSTOM_COMMAND(
                SOURCE wxMusik.app/Contents/MacOS/wxMusik
                COMMAND ${CMAKE_COMMAND}
                ARGS -E chdir "\"${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents\""
                ARGS `${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} ${WX_CONFIG_ARG_UNICODE} ${WX_CONFIG_ARG_DEBUG} --rezflags`
                "\"${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/MacOS/wxMusik\""
                TARGET wxMusik.app/Contents/MacOS/wxMusik
                )

    ADD_CUSTOM_COMMAND(SOURCE wxMusik 
                       COMMAND ""# dummy command because command is given by wx-config
                        ARGS `${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} ${WX_CONFIG_ARG_UNICODE} ${WX_CONFIG_ARG_DEBUG} --rezflags`
                              ${MyExecutable_PATH}/wxMusik
			TARGET wxMusik
                       )
ENDIF(APPLE)

# Set install options

INSTALL_TARGETS(/bin wxMusik)

