SUBDIRS(playbackart)
INCLUDE (${wxMusik_SOURCE_DIR}/scripts/cmake/TargetDistclean.cmake)
INSTALL_FILES(/share/wxMusik/data  ^[^.CcM][^Vma][^Sak].*$)
