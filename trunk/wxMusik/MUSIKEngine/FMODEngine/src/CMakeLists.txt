
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../inc)


ADD_LIBRARY(FMODEngine fmodengine.cpp  
		  fmodstreamout.cpp
		  fmodequalizer.cpp
)