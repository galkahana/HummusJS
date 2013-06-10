var pdfWriter = require('../hummus').createWriter('./output/PDFCopyingContextTest.PDF');

var copyingContext = pdfWriter.createPDFCopyingContext('./TestMaterials/BasicTIFFImagesTest.PDF');
copyingContext.appendPDFPageFromPDF(1);
copyingContext.appendPDFPageFromPDF(18);
copyingContext.appendPDFPageFromPDF(4);

pdfWriter.end();

console.log('done - ok');
