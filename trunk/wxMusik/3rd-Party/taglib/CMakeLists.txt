
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/toolkit)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/flac)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mpc)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ape)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mpeg)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ogg)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}//ogg/vorbis)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ogg/flac)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mpeg/id3v1)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mpeg/id3v2)




ADD_LIBRARY(TagLib ./ape/apefooter.cpp
	./ape/apeitem.cpp
	./ape/apetag.cpp
	./audioproperties.cpp
	./fileref.cpp
	./flac/flacfile.cpp
	./flac/flacproperties.cpp
	./mpc/mpcfile.cpp
	./mpc/mpcproperties.cpp
	./mpeg/id3v1/id3v1genres.cpp
	./mpeg/id3v1/id3v1tag.cpp
	./mpeg/id3v2/frames/attachedpictureframe.cpp
	./mpeg/id3v2/frames/commentsframe.cpp
	./mpeg/id3v2/frames/relativevolumeframe.cpp
	./mpeg/id3v2/frames/textidentificationframe.cpp
	./mpeg/id3v2/frames/uniquefileidentifierframe.cpp
	./mpeg/id3v2/frames/unknownframe.cpp
	./mpeg/id3v2/id3v2extendedheader.cpp
	./mpeg/id3v2/id3v2footer.cpp
	./mpeg/id3v2/id3v2frame.cpp
	./mpeg/id3v2/id3v2framefactory.cpp
	./mpeg/id3v2/id3v2header.cpp
	./mpeg/id3v2/id3v2synchdata.cpp
	./mpeg/id3v2/id3v2tag.cpp
	./mpeg/mpegfile.cpp
	./mpeg/mpegheader.cpp
	./mpeg/mpegproperties.cpp
	./mpeg/xingheader.cpp
	./ogg/flac/oggflacfile.cpp
	./ogg/oggfile.cpp
	./ogg/oggpage.cpp
	./ogg/oggpageheader.cpp
	./ogg/vorbis/vorbisfile.cpp
	./ogg/vorbis/vorbisproperties.cpp
	./ogg/xiphcomment.cpp
	./tag.cpp
	./toolkit/tbytevector.cpp
	./toolkit/tbytevectorlist.cpp
	./toolkit/tdebug.cpp	
	./toolkit/tfile.cpp
	./toolkit/tstring.cpp
	./toolkit/tstringlist.cpp
	./toolkit/unicode.cpp
)