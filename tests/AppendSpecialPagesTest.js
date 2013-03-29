var pdfWriter = require('../Hummus').createWriter('./output/AppendSpecialPagesTest.pdf');
var assert = require('assert');

assert.throws(function(){pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/Protected.pdf')});
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/ObjectStreamsModified.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/ObjectStreams.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/AddedItem.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/AddedPage.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/MultipleChange.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/RemovedItem.pdf');
pdfWriter.appendPDFPagesFromPDF('../deps/TestMaterials/Linearized.pdf');
pdfWriter.end();

console.log('done - ok');