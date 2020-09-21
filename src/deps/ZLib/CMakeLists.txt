cmake_minimum_required (VERSION 2.6) 
project (Zlib)

include(CheckIncludeFile)

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)
if(Z_HAVE_UNISTD_H)
ADD_DEFINITIONS(-DHAVE_UNISTD_H=1)
endif(Z_HAVE_UNISTD_H)

set(ZLIB_INCLUDE_DIRS ${Zlib_SOURCE_DIR} PARENT_SCOPE)
set(ZLIB_LDFLAGS Zlib PARENT_SCOPE)

add_library (Zlib_OBJLIB OBJECT 
#sources
adler32.c
compress.c
crc32.c
deflate.c
gzclose.c
gzlib.c
gzread.c
gzwrite.c
infback.c
inffast.c
inflate.c
inftrees.c
trees.c
uncompr.c
zutil.c

#headers
zconf.h
zlib.h
crc32.h
deflate.h
gzguts.h
inffast.h
inffixed.h
inflate.h
inftrees.h
trees.h
zutil.h
)

set(Zlib_OBJECTS $<TARGET_OBJECTS:Zlib_OBJLIB>)
set(Zlib_OBJECTS $<TARGET_OBJECTS:Zlib_OBJLIB> PARENT_SCOPE)
if(IS_XCODE)
    add_library(Zlib ${Zlib_OBJECTS} zutil.c)
else(IS_XCODE)
    add_library(Zlib ${Zlib_OBJECTS})
endif(IS_XCODE)

install(TARGETS Zlib
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)