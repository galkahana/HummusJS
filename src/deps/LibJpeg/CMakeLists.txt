

add_library(LibJpeg
#sources
jaricom.c
jcapimin.c
jcapistd.c
jcarith.c
jccoefct.c
jccolor.c
jcdctmgr.c
jchuff.c
jcinit.c
jcmainct.c
jcmarker.c
jcmaster.c
jcomapi.c
jcparam.c
jcprepct.c
jcsample.c
jctrans.c
jdapimin.c
jdapistd.c
jdarith.c
jdatadst.c
jdatasrc.c
jdcoefct.c
jdcolor.c
jddctmgr.c
jdhuff.c
jdinput.c
jdmainct.c
jdmarker.c
jdmaster.c
jdmerge.c
jdpostct.c
jdsample.c
jdtrans.c
jerror.c
jfdctflt.c
jfdctfst.c
jfdctint.c
jidctflt.c
jidctfst.c
jidctint.c
jmemmgr.c
jmemnobs.c
jquant1.c
jquant2.c
jutils.c

#headers
jconfig.h
jdct.h
jerror.h
jinclude.h
jmemsys.h
jmorecfg.h
jpegint.h
jpeglib.h
jversion.h

)

add_library(PDFHummus::LibJpeg ALIAS LibJpeg)

if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(LibJpeg PUBLIC LIBJPEG_SHARED)
    target_compile_definitions(LibJpeg PRIVATE LIBJPEG_EXPORTS)
endif ()

target_include_directories(LibJpeg
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/LibJpeg> 
)  

install(TARGETS LibJpeg
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
