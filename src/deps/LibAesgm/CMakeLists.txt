cmake_minimum_required (VERSION 2.6) 
project(LibAesgm)

set(LIBAESGM_INCLUDE_DIRS ${LibAesgm_SOURCE_DIR} PARENT_SCOPE)
set(LIBAESGM_LDFLAGS LibAesgm PARENT_SCOPE)

add_library (LibAesgm_OBJLIB OBJECT
#sources
aescrypt.c
aeskey.c
aes_modes.c
aes_ni.c
aestab.c


#headers
aescpp.h
aes.h
aes_ni.h
aesopt.h
aestab.h
aes_via_ace.h
brg_endian.h
brg_types.h

)

set(LibAesgm_OBJECTS $<TARGET_OBJECTS:LibAesgm_OBJLIB>)
set(LibAesgm_OBJECTS $<TARGET_OBJECTS:LibAesgm_OBJLIB> PARENT_SCOPE)
if(IS_XCODE)
    add_library(LibAesgm ${LibAesgm_OBJECTS} aestab.c)
else(IS_XCODE)
    add_library(LibAesgm ${LibAesgm_OBJECTS})
endif(IS_XCODE)

install(TARGETS LibAesgm
    RUNTIME DESTINATION bin COMPONENT dependencies
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT dependencies
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)