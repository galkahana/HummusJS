{
    'targets': [
    {
            'target_name': 'Hummus',
            'dependencies': [
               './deps/PDFWriter/binding.gyp:pdfwriter'
            ],
            'include_dirs': [
                './src',
                './deps/PDFWriter',
                './deps/FreeType/include'
            ],
           'sources': [
                './src/ObjectsContextDriver.cpp',
                './src/ImageXObjectDriver.cpp',
                './src/UsedFontDriver.cpp',
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
