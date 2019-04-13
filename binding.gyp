{
    'targets': [
    {
            'target_name': 'hummus',
            'type': 'loadable_module',
			'product_extension': 'node',
            'dependencies': [
               './src/deps/PDFWriter/binding.gyp:pdfwriter'
            ],
            'include_dirs': [
                './src',
                './src/deps/PDFWriter',
                './src/deps/FreeType/include'
            ],
           'sources': [
                './src/ConstructorsHolder.cpp',
                './src/PDFStreamDriver.cpp',
                './src/DictionaryContextDriver.cpp',
                './src/PDFTextStringDriver.cpp',
                './src/PDFDateDriver.cpp',
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
                './src/DocumentContextDriver.cpp',
                './src/ImageXObjectDriver.cpp',
                './src/UsedFontDriver.cpp',
                './src/ResourcesDictionaryDriver.cpp',
                './src/XObjectContentContextDriver.cpp',
                './src/FormXObjectDriver.cpp',
                './src/AbstractContentContextDriver.cpp',
                './src/PageContentContextDriver.cpp',
                './src/PDFPageDriver.cpp',
                './src/PDFPageModifierDriver.cpp',
                './src/PDFWriterDriver.cpp',
                './src/PDFPageInputDriver.cpp',
                './src/InputFileDriver.cpp',
                './src/OutputFileDriver.cpp',
                './src/InfoDictionaryDriver.cpp',
                './src/ByteReaderDriver.cpp',
                './src/ByteReaderWithPositionDriver.cpp',
                './src/ByteWriterDriver.cpp',
                './src/ByteWriterWithPositionDriver.cpp',
                './src/ObjectByteReader.cpp',
                './src/ObjectByteReaderWithPosition.cpp',
                './src/ObjectByteWriter.cpp',
                './src/ObjectByteWriterWithPosition.cpp',
                './src/PDFObjectParserDriver.cpp',
                './src/hummus.cpp'
            ]

	   },
		{
			'target_name': 'action_after_build',
			'type': 'none',
			'dependencies': [ '<(module_name)' ],
			'copies': [
				{
					'files': [
						'<(PRODUCT_DIR)/hummus.node'
					],
					'destination': '<(module_path)'
				}
			]
		}

    ]        
}
