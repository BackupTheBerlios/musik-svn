## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir)/include

noinst_LTLIBRARIES = libplugin_common.la

noinst_HEADERS = \
	all.h \
	canonical_tag.h \
	charset.h \
	defs.h \
	dither.h \
	locale_hack.h \
	vorbiscomment.h

libplugin_common_la_SOURCES = \
	canonical_tag.c \
	charset.c \
	dither.c \
	vorbiscomment.c

EXTRA_DIST = \
	Makefile.lite \
	README \
	plugin_common_static.dsp

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
