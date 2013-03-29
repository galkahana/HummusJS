var pdfWriter = require('../Hummus').createWriter('./output/AppendPagesTest.pdf');

pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/Original.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/XObjectContent.PDF');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/BasicTIFFImagesTest.PDF');

pdfWriter.end();

console.log('done - ok');