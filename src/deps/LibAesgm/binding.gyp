{
    'targets': [
        {
            'target_name': 'libaesgm',
            'type': 'static_library',
            'sources': [
                'aescrypt.c',
                'aeskey.c',
                'aes_ni.c',
                'aes_modes.c',
                'aestab.c',
                'aesopt.h',
                'brg_endian.h',
                'aes.h',
                'aestab.h',
                'brg_types.h',
                'aes_via_ace.h',
                'aes_ni.h',
                'aescpp.h'
            ]
        }
    ]        
}
