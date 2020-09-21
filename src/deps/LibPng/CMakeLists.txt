cmake_minimum_required (VERSION 2.6) 
project(LibPng)

# https://github.com/julienr/libpng-android/issues/6
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(CMAKE_C_FLAGS -DPNG_ARM_NEON_OPT=0 ${CMAKE_C_FLAGS})
    message (STATUS "libPng Arm64 - disable fpu Neon optimization")
endif()

set(LIBPNG_INCLUDE_DIRS ${LibPng_SOURCE_DIR} PARENT_SCOPE)
set(LIBPNG_LDFLAGS LibPng PARENT_SCOPE)

include_directories (${ZLIB_INCLUDE_DIRS})


add_library (LibPng_OBJLIB OBJECT 
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

set(LibPng_OBJECTS $<TARGET_OBJECTS:LibPng_OBJLIB>)
set(LibPng_OBJECTS $<TARGET_OBJECTS:LibPng_OBJLIB> PARENT_SCOPE)

if(IS_XCODE)
    add_library(LibPng ${LibPng_OBJECTS} pngwutil.c)
else(IS_XCODE)
    add_library(LibPng ${LibPng_OBJECTS})
endif(IS_XCODE)


target_link_libraries(LibPng ${ZLIB_LDFLAGS})

install(TARGETS LibPng
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)
