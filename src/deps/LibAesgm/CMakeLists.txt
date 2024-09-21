

add_library(LibAesgm
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

add_library(PDFHummus::LibAesgm ALIAS LibAesgm)

target_include_directories(LibAesgm
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/LibAesgm> 
)

if(WIN32 AND BUILD_SHARED_LIBS)
    # target_compile_definitions(LibAesgm INTERFACE DLL_IMPORT) # Not sure i understand completely why, but with this PDFWriter cant link
    target_compile_definitions(LibAesgm PRIVATE DLL_EXPORT)
endif()

install(TARGETS LibAesgm
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
