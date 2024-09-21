
add_library(FreeType
# sources
src/autofit/autofit.c
src/base/ftbase.c
src/base/ftbbox.c
src/base/ftbdf.c
src/base/ftbitmap.c
src/base/ftcid.c
src/base/ftfstype.c
src/base/ftgasp.c
src/base/ftglyph.c
src/base/ftgxval.c
src/base/ftinit.c
src/base/ftmm.c
src/base/ftotval.c
src/base/ftpatent.c
src/base/ftpfr.c
src/base/ftstroke.c
src/base/ftsynth.c
src/base/fttype1.c
src/base/ftwinfnt.c
src/bdf/bdf.c
src/bzip2/ftbzip2.c
src/cache/ftcache.c
src/cff/cff.c
src/cid/type1cid.c
src/gzip/ftgzip.c
src/lzw/ftlzw.c
src/pcf/pcf.c
src/pfr/pfr.c
src/psaux/psaux.c
src/pshinter/pshinter.c
src/psnames/psnames.c
src/raster/raster.c
src/sdf/sdf.c
src/sfnt/sfnt.c
src/smooth/smooth.c
src/svg/svg.c
src/truetype/truetype.c
src/type1/type1.c
src/type42/type42.c
src/winfonts/winfnt.c

# additional sources (in original determined by system)
src/base/ftsystem.c
src/base/ftdebug.c

#headers
include/freetype/config/ftconfig.h
include/freetype/config/ftheader.h
include/freetype/config/ftmodule.h
include/freetype/config/ftoption.h
include/freetype/config/ftstdlib.h
include/ft2build.h  
)

add_library(PDFHummus::FreeType ALIAS FreeType)

target_include_directories(FreeType
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/FreeType/include> 
)

if (WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(
        FreeType PRIVATE DLL_EXPORT)
endif ()

# set to use FT_CONFIG_OPTION_SYSTEM_ZLIB...or in other words...another zlib
target_compile_definitions(FreeType PUBLIC FT2_BUILD_LIBRARY=1 FT_CONFIG_OPTION_SYSTEM_ZLIB=1)   
if(USING_UNBUNDLED_ZLIB)
    target_link_libraries(FreeType ZLIB::ZLIB)
else(USING_UNBUNDLED_ZLIB)
    target_link_libraries(FreeType PDFHummus::Zlib)
endif(USING_UNBUNDLED_ZLIB)


target_include_directories(FreeType
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/config
    ${CMAKE_CURRENT_SOURCE_DIR}/include/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype/internal/services
)

install(TARGETS FreeType
    EXPORT PDFHummusTargets
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION include/FreeType
    COMPONENT dependencies
    FILES_MATCHING
    PATTERN "*.h"
)
