
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../inc)


ADD_LIBRARY(AudioQueueEngine CAStreamBasicDescription.cpp CAXException.cpp 
	 audioqueueengine.cpp	 
	 audioqueuestreamout.cpp
)