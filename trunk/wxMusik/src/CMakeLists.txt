
ADD_EXECUTABLE(wxMusik WIN32 
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
    ADD_CUSTOM_COMMAND(SOURCE wxMusik 
                       COMMAND ""# dummy command because command is given by wx-config
                        ARGS `${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} ${WX_CONFIG_ARG_UNICODE} 					${WX_CONFIG_ARG_DEBUG} --rezflags`
                              ${MyExecutable_PATH}/wxMusik
			TARGET wxMusik
                       )
ENDIF(APPLE)

# Set install options

INSTALL_TARGETS(/bin wxMusik)

