{
    'targets': [
        {
            'target_name': 'zlib',
            'type': 'static_library',
            'conditions': [
                ['OS == "win"', {            
                }, {
                    "defines": [
                        'HAVE_UNISTD_H=1'
                    ]
                }]
            ],
            'sources': [
                'adler32.c',
                'compress.c',
                'crc32.c',
                'deflate.c',
				'gzclose.c',
				'gzlib.c',
				'gzread.c',
				'gzwrite.c',
                'infback.c',
                'inffast.c',
                'inflate.c',
                'inftrees.c',                
                'trees.c',
                'uncompr.c',
                'zutil.c',
                'crc32.h',
                'deflate.h',
                'gzguts.h',
                'inffast.h',
                'inffixed.h',
                'inflate.h',
                'inftrees.h',
                'trees.h',
                'zconf.h',
                'zlib.h',
                'zutil.h'
            ]
        }
    ]        
}
