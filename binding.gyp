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
	       './src/PDFPageDriver.cpp',
		'./src/PDFWriterDriver.cpp',
		'./src/Hummus.cpp'
            ]

	}

    ]        
}
