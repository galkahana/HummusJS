{
    'targets': [
        {
            'target_name': 'libpng',
            'type': 'static_library',
            'dependencies': [
                '<(module_root_dir)/src/deps/ZLib/binding.gyp:zlib'
            ],    
            'include_dirs': [
              '<(module_root_dir)/src/deps/ZLib',
            ],  
            'conditions': [
                # https://github.com/julienr/libpng-android/issues/6. disable neon optimization for arm to avoid runtime errors
                ['target_arch=="arm64" or target_arch=="aarch64"', {
                   'defines': [
                       'PNG_ARM_NEON_OPT=0'
                   ]
                }]
            ],                              
            'sources': [
                'png.c',
                'pngerror.c',
                'pngget.c',
                'pngmem.c',
                'pngpread.c',
                'pngread.c',
                'pngrio.c',
                'pngrtran.c',
                'pngrutil.c',
                'pngset.c',
                'pngtrans.c',
                'pngwio.c',
                'pngwrite.c',
                'pngwtran.c',
                'pngwutil.c',
                'pnglibconf.h',
                'png.h',
                'pngconf.h',
                'pngdebug.h',
                'pnginfo.h',
                'pngpriv.h',
                'pngstruct.h'
            ],
            
        }
    ]        
}
