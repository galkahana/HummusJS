cmake_minimum_required (VERSION 2.6) 
project(LibPng)

set(LIBPNG_INCLUDE_DIRS ${LibPng_SOURCE_DIR} PARENT_SCOPE)
set(LIBPNG_LDFLAGS LibPng PARENT_SCOPE)

include_directories (${ZLIB_INCLUDE_DIRS})

add_library (LibPng 
#sources
png.c
pngerror.c
pngget.c
pngmem.c
pngpread.c
pngread.c
pngrio.c
pngrtran.c
pngrutil.c
pngset.c
pngtrans.c
pngwio.c
pngwrite.c
pngwtran.c
pngwutil.c

#headers
png.h
pngconf.h
pngdebug.h
pnginfo.h
pnglibconf.h
pngpriv.h
pngstruct.h
)
