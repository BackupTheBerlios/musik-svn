
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../inc)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../..)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-Party)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-Party/include)

ADD_LIBRARY(MUSIKEngine apedecoder.cpp  
		decoder.cpp  
		engine.cpp  
		flacdecoder.cpp  
		mpcdecoder.cpp
		stream.cpp
)