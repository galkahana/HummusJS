
var hummus = require('../hummus');
var pdfWriter = hummus.createWriter('./output/EmptyWriter.pdf',{version:hummus.ePDFVersion14});
pdfWriter.end();
console.log('done - ok');