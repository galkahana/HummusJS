cmake_minimum_required (VERSION 2.6) 
project(LibAesgm)

set(LIBAESGM_INCLUDE_DIRS ${LibAesgm_SOURCE_DIR} PARENT_SCOPE)
set(LIBAESGM_LDFLAGS LibAesgm PARENT_SCOPE)

add_library (LibAesgm
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
