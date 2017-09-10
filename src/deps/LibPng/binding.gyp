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
            ]
        }
    ]        
}
