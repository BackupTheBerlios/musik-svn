IF(OPTION_SUPPORT_APE)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Shared)
IF(APPLE)
IF(NOT CMAKE_OSX_ARCHITECTURES STREQUAL i386)
ADD_DEFINITIONS(-DWORS_BIGENDIAN)
ENDIF(NOT CMAKE_OSX_ARCHITECTURES STREQUAL i386)
ENDIF(APPLE)
IF(UNIX)
IF(CMAKE_WORDS_BIGENDIAN)
ADD_DEFINITIONS(-DWORS_BIGENDIAN)
ENDIF(CMAKE_WORDS_BIGENDIAN)
ENDIF(UNIX)
ADD_LIBRARY(mymac
MACLib/APECompress.cpp		MACLib/APESimple.cpp		MACLib/NewPredictor.cpp
MACLib/APECompressCore.cpp	MACLib/APETag.cpp		MACLib/Prepare.cpp
MACLib/APECompressCreate.cpp	MACLib/BitArray.cpp		MACLib/UnBitArray.cpp
MACLib/APEDecompress.cpp	MACLib/MACLib.cpp		MACLib/UnBitArrayBase.cpp
MACLib/APEHeader.cpp		MACLib/MACProgressHelper.cpp	MACLib/WAVInputSource.cpp
MACLib/APEInfo.cpp		MACLib/MD5.cpp
MACLib/APELink.cpp		MACLib/NNFilter.cpp	 	MACLib/Assembly/common.cpp
MACLib/Old/APEDecompressCore.cpp	MACLib/Old/AntiPredictorExtraHigh.cpp	MACLib/Old/AntiPredictorNormal.cpp
MACLib/Old/APEDecompressOld.cpp		MACLib/Old/AntiPredictorFast.cpp	MACLib/Old/UnBitArrayOld.cpp
MACLib/Old/Anti-Predictor.cpp		MACLib/Old/AntiPredictorHigh.cpp	MACLib/Old/UnMAC.cpp
Shared/GlobalFunctions.cpp	
Shared/CharacterHelper.cpp	Shared/MACUtils.cpp		
Shared/CircleBuffer.cpp		Shared/StdLibFileIO.cpp		Shared/WinFileIO.cpp

)

ENDIF(OPTION_SUPPORT_APE)
