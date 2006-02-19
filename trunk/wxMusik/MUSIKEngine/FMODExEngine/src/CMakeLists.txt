
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../inc)

SET(MUSIK_ENGINE "FMODExEngine")

ADD_LIBRARY(FMODExEngine fmodexengine.cpp  
		  fmodexstreamout.cpp
)