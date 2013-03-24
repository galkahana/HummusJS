{
    'targets': [
    {
            'target_name': 'Hummus',
            'dependencies': [
               './deps/PDFWriter/binding.gyp:pdfwriter'
            ],
            'include_dirs': [
                './src',
                './deps/PDFWriter'
            ],
           'sources': [
                './src/ResourcesDictionaryDriver.cpp',
                './src/XObjectContentContextDriver.cpp',
                './src/FormXObjectDriver.cpp',
                './src/AbstractContentContextDriver.cpp',
                './src/PageContentContextDriver.cpp',
                './src/PDFPageDriver.cpp',
                './src/PDFWriterDriver.cpp',
                './src/Hummus.cpp'
            ]

	}

    ]        
}
