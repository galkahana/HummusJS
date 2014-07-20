var hummus = require('../hummus');
var fs = require('fs');

var inStreamA = new hummus.PDFRStreamForFile('./TestMaterials/BasicJPGImagesTest.PDF');
var inStreamB = new hummus.PDFRStreamForFile('./TestMaterials/AddedPage.pdf');
var outStream = new hummus.PDFWStreamForFile('./output/StreamCopyingContext.pdf');

var pdfWriter = hummus.createWriterToModify(inStreamB, outStream);

var copyCtx = pdfWriter.createPDFCopyingContext('./TestMaterials/BasicJPGImagesTest.PDF');
copyCtx.appendPDFPageFromPDF(0);

var copyCtx = pdfWriter.createPDFCopyingContext(inStreamA);
copyCtx.appendPDFPageFromPDF(0);

pdfWriter.end();
outStream.close();
inStreamA.close();
inStreamB.close();


console.log('done - ok');