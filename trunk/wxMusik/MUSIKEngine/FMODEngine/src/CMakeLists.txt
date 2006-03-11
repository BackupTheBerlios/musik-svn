
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../inc)

SET(MUSIK_ENGINE FMODEngine)

ADD_LIBRARY(FMODEngine fmodengine.cpp  
		  fmodstreamout.cpp
)