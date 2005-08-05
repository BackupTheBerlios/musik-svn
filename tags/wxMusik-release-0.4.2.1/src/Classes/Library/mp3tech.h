/*

    Copyright (C) 2005 Gunnar Roth gunnar.roth@gmx.de
    I did :
    - remove all stuff about idtag /removed functions and structures)
    - replace dependency on filename ( stat() usage) 
    - make code usable for c++ callers using extern "C"
    - move most private prottypes from .h to .c
    - remove #include "mp3info.h" and replaced it by all needed c runtime headers and mp3tech.h        
    - added Xing and VBRI vbr header detection and usage
    - detect invalid bitrate 0xF
    - try to optimize vbr detection for files without any vbr header 

    mp3tech.h - Headers for mp3tech.c

    Copyright (C) 2000-2001  Cedric Tefft <cedric@earthling.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  ***************************************************************************

  This file is based in part on:

	* MP3Info 0.5 by Ricardo Cerqueira <rmc@rccn.net>
	* MP3Stat 0.9 by Ed Sweetman <safemode@voicenet.com> and 
			 Johannes Overmann <overmann@iname.com>

*/

/* MIN_CONSEC_GOOD_FRAMES defines how many consecutive valid MP3 frames
   we need to see before we decide we are looking at a real MP3 file */
#define MIN_CONSEC_GOOD_FRAMES 4
#define FRAME_HEADER_SIZE 4
#define MIN_FRAME_SIZE 21
#define NUM_SAMPLES 3

enum VBR_REPORT { VBR_VARIABLE, VBR_AVERAGE, VBR_MEDIAN };
enum SCANTYPE { SCAN_NONE, SCAN_QUICK, SCAN_FULL };
#ifdef __cplusplus
extern "C" {
#endif 

typedef struct {
	unsigned long	sync;
	unsigned int	version;
	unsigned int	layer;
	unsigned int	crc;
	unsigned int	bitrate;
	unsigned int	freq;
	unsigned int	padding;
	unsigned int	extension;
	unsigned int	mode;
	unsigned int	mode_extension;
	unsigned int	copyright;
	unsigned int	original;
	unsigned int	emphasis;
} mp3header;

typedef struct {
	FILE *file;
	off_t datasize;
	int header_isvalid;
	mp3header header;
	int vbr;
	float vbr_average;
	int seconds;
	int frames;
	int badframes;
} mp3info;


int get_mp3_info(mp3info *mp3,int scantype, int fullscan_vbr);

#ifdef __cplusplus
}
#endif 
