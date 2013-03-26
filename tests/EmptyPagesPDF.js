
var hummus = require('../Hummus');
var pdfWriter = hummus.createWriter("EmptyPages.pdf",hummus.ePDFVersion14);
var page = pdfWriter.createPage();

page.mediaBox = [0,0,595,842];

for(var i=0; i < 4; ++i)
	pdfWriter.writePage(page);

pdfWriter.end();

console.log('done - ok');