
#SRCS_NASM = \
#	ia32/cpu_asm.nasm \
#	ia32/fixed_asm.nasm \
#	ia32/lpc_asm.nasm

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

#ADD_DEFINITIONS(-DFLAC__CPU_IA32 -DFLAC__HAS_NASM -DFLAC__ALIGN_MALLOC_DATA -DVERSION=|{"1.1.2")

ADD_DEFINITIONS(-DFLAC__CPU_IA32 -DFLAC__ALIGN_MALLOC_DATA -DVERSION=\"1.2.1\")
ADD_LIBRARY(myflac
	bitreader.c
	bitwriter.c
	bitmath.c 
	cpu.c 
	crc.c 
	fixed.c 
	float.c 
	format.c
	lpc.c 
	md5.c 
	memory.c 
	metadata_iterators.c 
	metadata_object.c 
	ogg_decoder_ascpect.c
	ogg_encoder_ascpect.c
	ogg_helper.c
	ogg_mapping.c
	window.c
	stream_decoder.c 
	stream_encoder.c 
	stream_encoder_framing.c
)
