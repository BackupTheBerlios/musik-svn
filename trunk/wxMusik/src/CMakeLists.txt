SET(PROJECT_VERSION_MAJOR "0.4.3.0")

INCLUDE( ${wxMusik_SOURCE_DIR}/MUSIKEngine/engine.cmake)

SET(MAIN_SRCS
        ./MusikApp
        ./MusikId
        ./PlaylistColumn
        ./Playlist
        ./MusikGlobals
        ./MusikUtils
        ./MusikAboutDlg
        ./Classes/ActivityBox
        ./Classes/ActivityAreaCtrl
        ./Classes/BtnDown
        ./Classes/GaugeSeek
        ./Classes/MusikLibrary
        ./Classes/MusikDb_Sqlite3
        ./Classes/MusikDb_Sqlite
        ./Classes/MusikPlayer
        ./Classes/MusikPrefs
        ./Classes/NowPlayingCtrl
        ./Classes/MusikListCtrl
        ./Classes/PlaylistCtrl
        ./Classes/PlaylistInfoCtrl
        ./Classes/SourcesBox
        ./Classes/WebServer
        ./Classes/MusikEQCtrl
        ./Classes/MusikEQGauge
        ./Classes/Tunage
        ./Classes/PictureBox
		
        ./Classes/Library/TagLibInfo
        ./Classes/Library/FlacInfo
        ./Classes/Library/CFMODInfo
        ./Classes/Library/APEInfo
        ./Classes/Library/MetaDataHandler
        ./Classes/Library/mp3tech
        
        ${wxMusik_SOURCE_DIR}/3rd-Party/Bitap/libbitap.c
        ${wxMusik_SOURCE_DIR}/3rd-Party/TagHelper/idtag.cpp
		
        ./Frames/MusikFrame
        ./Frames/MusikFrameEvents
        ./Frames/MusikLibraryFrame
        ./Frames/MusikTagFrame
        ./Frames/MusikVolumeFrame
        ./Frames/MusikFXFrame
        ./Frames/MusikAutoTaggerFrame
        ./Threads/ActivityBoxThreads
        ./Threads/PlaylistCtrlThreads
        ./Threads/MusikLibraryThreads
        ./Threads/MusikTagThreads
        ./Threads/MusikThreads

        ./Frames/Prefs/TaggingGeneralPanel
        ./Frames/Prefs/OptionAutoDJPanel
        ./Frames/Prefs/StreamingProxyServerPanel
        ./Frames/Prefs/StreamingBufferPanel
        ./Frames/Prefs/MusikPrefsFrame
        ./Frames/Prefs/OptionPlaylistPanel.cpp
        ./Frames/Prefs/OptionSelectionsPanel.cpp
        ./Frames/Prefs/OptionFileAssocPanel.cpp
        ./Frames/Prefs/TaggingAutoRenamePanel.cpp
        ./Frames/Prefs/SoundCrossfaderPanel.cpp
        ./Frames/Prefs/SoundDriverPanel.cpp
        ./Frames/Prefs/OptionGeneralPanel.cpp
        ./Frames/Prefs/OptionTunagePanel.cpp
        ./Frames/Prefs/WebServerPanel.cpp
    )
IF(OPTION_SUPPORT_FLAC)
SET(FLAC_LIBS "myflac plugin_common grabbag  replaygain_synthesis replaygain_analysis")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../MUSIKEngine/3rd-Party/include)
SEPARATE_ARGUMENTS(FLAC_LIBS)
ELSE(OPTION_SUPPORT_FLAC)
SET(FLAC_LIBS "")
ENDIF(OPTION_SUPPORT_FLAC)

IF(OPTION_SUPPORT_FAAD2)
SET(FAAD2_LIBS "myfaad2 mymp4ff mymp4v2")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../MUSIKEngine/3rd-Party)
SEPARATE_ARGUMENTS(FAAD2_LIBS)
ELSE(OPTION_SUPPORT_FAAD2)
SET(FAAD2_LIBS "")
ENDIF(OPTION_SUPPORT_FAAD2)

IF(NOT OPTION_SUPPORT_APE)
SET(MAC_LIBRARY "")
ADD_DEFINITIONS(-DMUSIKENGINE_NO_APE_SUPPORT)
ENDIF(NOT OPTION_SUPPORT_APE)

IF(NOT OPTION_SUPPORT_FLAC)
ADD_DEFINITIONS(-DMUSIKENGINE_NO_FLAC_SUPPORT)
ENDIF(NOT OPTION_SUPPORT_FLAC)

IF(NOT OPTION_SUPPORT_FAAD2)
ADD_DEFINITIONS(-DMUSIKENGINE_NO_FAAD2_SUPPORT)
ENDIF(NOT OPTION_SUPPORT_FAAD2)

IF(NOT OPTION_SUPPORT_MPC)
SET(MUSEPACK_LIBRARY "")
ADD_DEFINITIONS(-DMUSIKENGINE_NO_MPC_SUPPORT)
ENDIF(NOT OPTION_SUPPORT_MPC)

IF(WIN32)
    SET(WIN32_EXECUTABLE WIN32)
ENDIF(WIN32)



IF(APPLE)
    SET(OTHER_SRCS  ./wxmod_listctrl)

    SET(MACOSX_BUNDLE_EXECUTABLE_NAME wxMusik)
    SET(MACOSX_BUNDLE_INFO_STRING "wxMusik version ${PROJECT_VERSION_MAJOR}, (c) Gunnar Roth")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "roth.gunnar.wxMusik")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION_MAJOR}, (c) 2006 Gunnar  Roth")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "wxMusik")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION_MAJOR}")
    SET(MACOSX_BUNDLE_ICON_FILE "wxMusik.icns")
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright 2006 Gunnar Roth")
ENDIF(APPLE)

ADD_EXECUTABLE(wxMusik ${WIN32_EXECUTABLE} MACOSX_BUNDLE  ${MAIN_SRCS} ${OTHER_SRCS})

TARGET_LINK_LIBRARIES(wxMusik
        ${MUSIK_ENGINE}
        MUSIKEngine
        shibatch
        mysqlite3
        mysqlite2
        ${WXWINDOWS_LIBRARY}
        ${MUSEPACK_LIBRARY}
        ${MAC_LIBRARY}
        ${FMOD_LIBRARY}
        TagLib
        ${FLAC_LIBS}
	${FAAD2_LIBS}
        ${FLAC_LIBRARY}
    )


IF(APPLE)
    INCLUDE( ${wxMusik_SOURCE_DIR}/scripts/cmake/CopyIfDifferent.cmake)

    MAKE_DIRECTORY(${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/MacOS)
    CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/scripts/cmake/MacOSXBundleInfo.plist.in
                   ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Info.plist)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/images/${MACOSX_BUNDLE_ICON_FILE}
		   ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE} COPYONLY)


    CONFIGURE_FILE(${FMOD_LIBRARY} 
                    ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/FrameWorks/libfmodex.dylib  COPYONLY)

    SET(RSRC_FILES about.html musik.png wxMusik.jpg)
    COPY_IF_DIFFERENT( ${PROJECT_SOURCE_DIR}/data  ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Resources "${RSRC_FILES}"  RsrcTargets "RsrcFiles")

    SET(RSRC_FILES2 rating1.png rating3.png rating5.png  rating7.png  rating9.png       ratingminus2.png  ratingminus4.png  ratingminus6.png  ratingminus8.png
                    rating0.png     rating2.png  rating4.png  rating6.png  rating8.png  ratingminus1.png  ratingminus3.png  ratingminus5.png  ratingminus7.png  ratingminus9.png)
    COPY_IF_DIFFERENT( ${PROJECT_SOURCE_DIR}/data/ratingart ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Resources/ratingart  "${RSRC_FILES2}" RsrcTargets2 "RsrcFiles2")   

    SET(RSRC_FILES3 Next.png  Next_down.png  Pause.png  Pause_down.png  Play.png  Play_down.png  Prev.png  Prev_down.png  Stop.png  Stop_down.png  Volume.png  Volume_down.png )
    COPY_IF_DIFFERENT( ${PROJECT_SOURCE_DIR}/data/playbackart ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/Resources/playbackart  "${RSRC_FILES3}" RsrcTargets3 "RsrcFiles3") 
    ADD_CUSTOM_TARGET(CopyRsrcFiles ALL DEPENDS ${RsrcTargets} ${RsrcTargets2} ${RsrcTargets3})

    WRITE_FILE(${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/PkgInfo "APPL????")
#    ADD_CUSTOM_COMMAND(
#                SOURCE wxMusik.app/Contents/MacOS/wxMusik
#                COMMAND ${CMAKE_COMMAND}                ARGS -E chdir "\"${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents\""
#                ARGS `${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} ${WX_CONFIG_ARG_UNICODE} ${WX_CONFIG_ARG_DEBUG} --rezflags`
#                "\"${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/MacOS/wxMusik\""
#                TARGET wxMusik.app/Contents/MacOS/wxMusik
#                )
    ADD_CUSTOM_TARGET(WXMUSIK_NAME ALL
                COMMAND /usr/bin/install_name_tool -change /usr/local/lib/libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib ${EXECUTABLE_OUTPUT_PATH}/wxMusik.app/Contents/MacOS/wxMusik
                )
    ADD_DEPENDENCIES(WXMUSIK_NAME  wxMusik)
    ADD_DEPENDENCIES(CopyRsrcFiles wxMusik )	
ENDIF(APPLE)

# Set install options

INSTALL_TARGETS(/bin wxMusik)
