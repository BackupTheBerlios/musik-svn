
ADD_EXECUTABLE(wxMusik
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
        plugin_common
        grabbag
        replaygain_synthesis
    )

IF(APPLE)
  FIND_PROGRAM(VTK_APPLE_RESOURCE Rez /Developer/Tools)
  IF(VTK_APPLE_RESOURCE)
    ADD_CUSTOM_COMMAND(SOURCE wxMusik
                       COMMAND ${VTK_APPLE_RESOURCE}
                       ARGS -d __DARWIN__ -t APPL Carbon.r /usr/local/lib/libwx_macu-2.5.3.r -o
                       ${MyExecutable_PATH}/wxMusik
                       TARGET wxMusik)
  ENDIF(VTK_APPLE_RESOURCE)
ENDIF(APPLE)

# Set install options

INSTALL_TARGETS(/bin wxMusik)

