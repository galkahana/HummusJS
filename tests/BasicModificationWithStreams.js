var hummus = require('../hummus');
var inStream = new hummus.PDFRStreamForFile( './TestMaterials/MultipleChange.pdf' );
var outStream = new hummus.PDFWStreamForFile('./output/BasicModificationWithStreams.pdf')
var pdfWriter = hummus.createWriterToModify(inStream,outStream);
 
var pageModifier = new hummus.PDFPageModifier(pdfWriter,0);
 
pageModifier.startContext().getContext().writeText('Some added Text',
75,
805,
{font:pdfWriter.getFontForFile('./TestMaterials/fonts/Couri.ttf'),size:14,colorspace:'gray',color:0x00});
pageModifier.endContext().writePage();
 
pdfWriter.end();
outStream.close();
inStream.close();
 
console.log('done - ok');