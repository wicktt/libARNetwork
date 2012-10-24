###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS = -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES	=	../Includes/libNetWork/singleBuffer.h \
					../Includes/libNetWork/ringBuffer.h \
					../Includes/libNetWork/inOutBuffer.h\
					../Includes/libNetWork/sender.h \
					../Includes/libNetWork/receiver.h \
					../Includes/libNetWork/netWork.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES 	= 	$(HEADER_FILES)		\
					../Sources/singleBuffer.c \
					../Sources/ringBuffer.c	\
					../Sources/inOutBuffer.c\
					../Sources/sender.c \
					../Sources/receiver.c \
					../Sources/netWork.c
					
# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES 		= libnetWork_dbg.la
libnetWork_dbg_ladir 		= $(includedir)/libNetWork
libnetWork_dbg_la_HEADERS 	= $(HEADER_FILES)
libnetWork_dbg_la_SOURCES 	= $(SOURCE_FILES)
else
lib_LTLIBRARIES 		= libnetWork.la
libnetWork_ladir 		= $(includedir)/libNetWork
libnetWork_la_HEADERS 	= $(HEADER_FILES)
libnetWork_la_SOURCES 	= $(SOURCE_FILES)
endif

#!! check_PROGRAMS = ../TestBench/test1 ../TestBench/test2
#!! ___TestBench_test1_SOURCES 	= ../TestBench/test1.c
#!! ___TestBench_test2_SOURCES 	= ../TestBench/test2.c
#!! if DEBUG_MODE
#!! ___TestBench_test1_LDADD 	= libNetWork_dbg.la
#!! ___TestBench_test2_LDADD 	= libNetWork_dbg.la
#!! else
#!! ___TestBench_test1_LDADD 	= libNetWork.la
#!! ___TestBench_test2_LDADD 	= libNetWork.la
#!! endif

CLEAN_FILES = libNetWork.la libNetWork_dbg.la

EXTRA_DIST = bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST += doxygen.ac doxygen.am $(DX_CONFIG)
endif