
var hummus = require('../hummus');
var pdfWriter = hummus.createWriter('./output/SimpleContentPageTest.pdf');
var page = new hummus.PDFPage(0,0,595,842);

var pageContent = pdfWriter.startPageContentContext(page);

// draw a 100X100 points cyan square
pageContent.q()
           .k(100,0,0,0)
           .re(100,500,100,100)
           .f()
           .Q()

// force stream change
pdfWriter.pausePageContentContext(pageContent);

// draw a 200X100 points red rectangle
pageContent.q()
           .k(0,100,100,0)
           .re(200,600,200,100)
           .f()
           .Q();

// draw a gray line
pageContent.q()
           .G(0.5)
           .w(3)
           .m(200,600)
           .l(400,400)
           .S()
           .Q();

// write page will automatically close the page context
pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok');