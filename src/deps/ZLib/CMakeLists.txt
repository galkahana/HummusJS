cmake_minimum_required (VERSION 2.6) 
project (Zlib)

set(ZLIB_INCLUDE_DIRS ${Zlib_SOURCE_DIR} PARENT_SCOPE)
set(ZLIB_LDFLAGS Zlib PARENT_SCOPE)

add_library (Zlib 
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
crc32.h
deflate.h
gzguts.h
inffast.h
inffixed.h
inflate.h
inftrees.h
trees.h
zconf.h
zlib.h
zutil.h
)
