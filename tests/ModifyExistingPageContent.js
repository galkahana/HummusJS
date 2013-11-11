var hummus = require('../hummus');

var pdfWriter = hummus.createWriterToModify('./TestMaterials/BasicJPGImagesTest.PDF',{modifiedFilePath:'./output/BasicJPGImagesTestPageModified.pdf'});

var pageModifier = new hummus.PDFPageModifier(pdfWriter,0);




pageModifier.startContext().getContext().writeText('Test Text',
													75,
													805,
													{font:pdfWriter.getFontForFile('./TestMaterials/fonts/Couri.ttf'),size:14,colorspace:'gray',color:0x00});
pageModifier.endContext()
			.writePage();

pdfWriter.end();

console.log('done - ok');