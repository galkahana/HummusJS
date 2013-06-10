cmake_minimum_required (VERSION 2.6) 

project (Zlib)
add_library (Zlib 
#sources
adler32.c
compress.c
crc32.c
deflate.c
gzio.c
infback.c
inffast.c
inflate.c
inftrees.c
trees.c
uncompr.c
zutil.c

#headers
crc32.h
deflate.h
inffast.h
inffixed.h
inflate.h
inftrees.h
trees.h
zconf.h
zconf.in.h
zlib.h
zutil.h


)
