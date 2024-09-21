#pragma once

#if defined(_WIN32)
#  if defined(LIBJPEG_SHARED)
#    if defined(LIBJPEG_EXPORTS)
#      define LIBJPEG_EXPORT_API __declspec(dllexport)
#    else
#      define LIBJPEG_EXPORT_API __declspec(dllimport)
#    endif
#  else
#    define LIBJPEG_EXPORT_API
#  endif
#else
#  define LIBJPEG_EXPORT_API
#endif