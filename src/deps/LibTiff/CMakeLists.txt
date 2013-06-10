cmake_minimum_required (VERSION 2.6) 
project(LibTiff)

include_directories (${Zlib_SOURCE_DIR})

add_definitions(
-D_CRT_SECURE_NO_DEPRECATE=1
-DAVOID_WIN32_FILEIO=1
-DCHECK_JPEG_YCBCR_SUBSAMPLING=1
-DDEFAULT_EXTRASAMPLE_AS_ALPHA=1
-DSTRIPCHOP_DEFAULT=TIFF_STRIPCHOP
-DSTRIP_SIZE_DEFAULT=8192
-DPIXARLOG_SUPPORT=1
-DZIP_SUPPORT=1
-DLOGLUV_SUPPORT=1
-DNEXT_SUPPORT=1
-DTHUNDER_SUPPORT=1
-DLZW_SUPPORT=1
-DPACKBITS_SUPPORT=1
-DCCITT_SUPPORT=1
-DTIF_PLATFORM_CONSOLE=1
-DFILLODER_LSB2MSB=1)

add_library (LibTiff 
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
tif_predict.h
uvcode.h



)
