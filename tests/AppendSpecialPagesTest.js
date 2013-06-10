var pdfWriter = require('../hummus').createWriter('./output/AppendSpecialPagesTest.pdf');
var assert = require('assert');

assert.throws(function(){pdfWriter.appendPDFPagesFromPDF('./TestMaterials/Protected.pdf')});
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/ObjectStreamsModified.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/ObjectStreams.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/AddedItem.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/AddedPage.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/MultipleChange.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/RemovedItem.pdf');
pdfWriter.appendPDFPagesFromPDF('./TestMaterials/Linearized.pdf');
pdfWriter.end();

console.log('done - ok');