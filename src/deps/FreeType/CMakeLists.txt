cmake_minimum_required (VERSION 2.6) 
project(FreeType)

add_definitions(-DFT2_BUILD_LIBRARY=1)
include_directories (${FreeType_SOURCE_DIR}/include)
include_directories (${FreeType_SOURCE_DIR}/include/freetype)
include_directories (${FreeType_SOURCE_DIR}/include/freetype/config)
include_directories (${FreeType_SOURCE_DIR}/include/src)
include_directories (${FreeType_SOURCE_DIR}/include/freetype/internal)
include_directories (${FreeType_SOURCE_DIR}/include/freetype/internal/services)

add_library (FreeType
#sources
src/base/ftbbox.c
src/base/ftgxval.c
src/base/ftlcdfil.c
src/base/ftmm.c
src/base/ftotval.c
src/base/ftpatent.c
src/base/ftpfr.c
src/base/ftsynth.c
src/base/fttype1.c
src/base/ftwinfnt.c
src/base/ftxf86.c
#src/otvalid/otvalid.c
src/pcf/pcf.c
src/pfr/pfr.c
src/psaux/psaux.c
src/pshinter/pshinter.c
src/psnames/psmodule.c
#src/psnames/psnames.c
src/raster/raster.c
src/sfnt/sfnt.c
src/truetype/truetype.c
src/type1/type1.c
src/type42/type42.c
src/cid/type1cid.c
src/winfonts/winfnt.c
src/autofit/autofit.c
src/bdf/bdf.c
src/cff/cff.c
src/base/ftbase.c
src/base/ftbitmap.c
src/cache/ftcache.c
src/base/ftcid.c
src/base/ftdebug.c
src/base/ftfstype.c
src/base/ftgasp.c
src/base/ftglyph.c
src/gzip/ftgzip.c
src/base/ftinit.c
src/lzw/ftlzw.c
src/base/ftstroke.c
src/base/ftsystem.c
src/smooth/smooth.c

#headers
include/freetype/config/ftconfig.h
include/freetype/config/ftheader.h
include/freetype/config/ftmodule.h
include/freetype/config/ftoption.h
include/freetype/config/ftstdlib.h
include/ft2build.h
)
