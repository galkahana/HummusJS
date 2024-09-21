add_library(LibPng
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

add_library(PDFHummus::LibPng ALIAS LibPng)

target_include_directories(LibPng
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/LibPng> 
)

# https://github.com/julienr/libpng-android/issues/6
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
    target_compile_definitions(LibPng PRIVATE 
        PNG_ARM_NEON_OPT=0
    )
endif()

# drop obsolete symbol, causing trouble when compiling with clang on mac, and in general unneeded
target_compile_options(LibPng PRIVATE -UTARGET_OS_MAC)

if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(LibPng PUBLIC PNG_USE_DLL)
endif()

if(USING_UNBUNDLED_ZLIB)
    target_link_libraries(LibPng ZLIB::ZLIB)
else(USING_UNBUNDLED_ZLIB)
    target_link_libraries(LibPng PDFHummus::Zlib)
endif(USING_UNBUNDLED_ZLIB)

install(TARGETS LibPng
    EXPORT PDFHummusTargets
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    COMPONENT dependencies
    FILES_MATCHING
    PATTERN "*.h"
)
