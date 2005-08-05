# add custom target distclean
# cleans and removes cmake generated files etc.
# Jan Woetzel 04/2003
#

IF (UNIX)
  ADD_CUSTOM_TARGET(
    distclean 
    @echo cleaning for source distribution 
    DEPENDS clean)
  
  SET(DISTCLEAN_FILES
    cmake.depends
    cmake.check_depends
    CMakeCache.txt
    cmake.check_cache
    CMakeOutput.log
    *.cmake
    Makefile
    core core.*
    gmon.out bb.out
    *~ 
    *%
    SunWS_cache
    ii_files
    *.so
    *.o
    *.a
    CopyOfCMakeCache.txt
    html latex Doxyfile 
    )
  
  SET(DISTCLEAN_DIRS
    CMakeTmp
    )
  
  ## for 1.8.x:
  ADD_CUSTOM_COMMAND(
    TARGET distclean
    PRE_BUILD
    COMMAND rm
    ARGS    -Rf ${DISTCLEAN_FILES} ${DISTCLEAN_DIRS}
    COMMENT
    )
  
ENDIF(UNIX)
