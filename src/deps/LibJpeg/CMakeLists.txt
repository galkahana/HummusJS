cmake_minimum_required (VERSION 2.6) 
project(LibJpeg)

set(LIBJPEG_INCLUDE_DIRS ${LibJpeg_SOURCE_DIR} PARENT_SCOPE)
set(LIBJPEG_LDFLAGS LibJpeg PARENT_SCOPE)

add_library (LibJpeg_OBJLIB OBJECT 
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

set(LibJpeg_OBJECTS $<TARGET_OBJECTS:LibJpeg_OBJLIB>)
set(LibJpeg_OBJECTS $<TARGET_OBJECTS:LibJpeg_OBJLIB> PARENT_SCOPE)
if(IS_XCODE)
    add_library(LibJpeg ${LibJpeg_OBJECTS} jutils.c)
else(IS_XCODE)
    add_library(LibJpeg ${LibJpeg_OBJECTS})
endif(IS_XCODE)


install(TARGETS LibJpeg
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)