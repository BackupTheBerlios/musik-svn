INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
ADD_LIBRARY(mymp4ff drms.c  mp4atom.c  mp4ff.c  mp4meta.c
		mp4sample.c  mp4tagupdate.c  mp4util.c)
