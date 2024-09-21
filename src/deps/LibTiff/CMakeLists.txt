add_library(LibTiff 
#sources
tif_aux.c
tif_close.c
tif_codec.c
tif_color.c
tif_compress.c
tif_dir.c
tif_dirinfo.c
tif_dirread.c
tif_dirwrite.c
tif_dumpmode.c
tif_error.c
tif_extension.c
tif_fax3.c
tif_fax3sm.c
tif_flush.c
tif_getimage.c
tif_hash_set.c
tif_jbig.c
tif_jpeg.c
tif_luv.c
tif_lzw.c
tif_next.c
tif_ojpeg.c
tif_open.c
tif_packbits.c
tif_pixarlog.c
tif_predict.c
tif_print.c
tif_read.c
tif_stream.cxx
tif_strip.c
tif_swab.c
tif_thunder.c
tif_tile.c
tif_unix.c
tif_version.c
tif_warning.c
tif_write.c
tif_zip.c


#headers
t4.h
tiff.h
tiffconf.h
tiffio.h
tiffio.hxx
tiffiop.h
tiffvers.h
tif_config.h
tif_dir.h
tif_fax3.h
tif_hash_set.h
tif_predict.h
uvcode.h



)

add_library(PDFHummus::LibTiff ALIAS LibTiff)

target_include_directories(LibTiff
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/LibTiff> 
)

#
# Check for unistd.h
#
include(CheckIncludeFile)
check_include_file(unistd.h T_HAVE_UNISTD_H)
if(T_HAVE_UNISTD_H)
    target_compile_definitions(LibTiff PUBLIC HAVE_UNISTD_H=1)
endif(T_HAVE_UNISTD_H)

target_compile_definitions(LibTiff PUBLIC 
    _CRT_SECURE_NO_DEPRECATE=1
    AVOID_WIN32_FILEIO=1
    CHECK_JPEG_YCBCR_SUBSAMPLING=1
    DEFAULT_EXTRASAMPLE_AS_ALPHA=1
    STRIPCHOP_DEFAULT=TIFF_STRIPCHOP
    STRIP_SIZE_DEFAULT=8192
    PIXARLOG_SUPPORT=1
    ZIP_SUPPORT=1
    LOGLUV_SUPPORT=1
    NEXT_SUPPORT=1
    THUNDER_SUPPORT=1
    LZW_SUPPORT=1
    PACKBITS_SUPPORT=1
    CCITT_SUPPORT=1
    TIF_PLATFORM_CONSOLE=1
    FILLODER_LSB2MSB=1
)

if(USING_UNBUNDLED_ZLIB)
    target_link_libraries(LibTiff ZLIB::ZLIB)
else(USING_UNBUNDLED_ZLIB)
    target_link_libraries(LibTiff PDFHummus::Zlib)
endif(USING_UNBUNDLED_ZLIB)


if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(LibTiff PROPERTIES  WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

install(TARGETS LibTiff
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
