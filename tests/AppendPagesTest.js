var pdfWriter = require('../hummus').createWriter('./output/AppendPagesTest.pdf');

pdfWriter.appendPDFPagesFromPDF('./TestMaterials/Original.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/XObjectContent.PDF');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/BasicTIFFImagesTest.PDF');

pdfWriter.end();

console.log('done - ok');