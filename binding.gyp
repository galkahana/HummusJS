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
           
                './src/PDFArrayDriver.cpp',
                './src/PDFDictionaryDriver.cpp',
                './src/PDFStreamInputDriver.cpp',
                './src/PDFIndirectObjectReferenceDriver.cpp',
                './src/PDFBooleanDriver.cpp',
                './src/PDFLiteralStringDriver.cpp',
                './src/PDFHexStringDriver.cpp',
                './src/PDFNullDriver.cpp',
                './src/PDFNameDriver.cpp',
                './src/PDFIntegerDriver.cpp',
                './src/PDFRealDriver.cpp',
                './src/PDFSymbolDriver.cpp',
                './src/PDFObjectDriver.cpp',
                './src/PDFReaderDriver.cpp',
                './src/DocumentCopyingContextDriver.cpp',
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
