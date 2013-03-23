
var hummus = require('../Hummus');
var pdfWriter = hummus.createWriter("SimpleContentPageTest.pdf");
var page = pdfWriter.createPage();

page.mediaBox = [0,0,595,842];

var pageContent = pdfWriter.startPageContentContext(page);

// draw a 100X100 points cyan square
pageContent.q();
pageContent.k(100,0,0,0);
pageContent.re(100,500,100,100);
pageContent.f();
pageContent.Q();

// force stream change
pdfWriter.pausePageContentContext(pageContent);

// draw a 200X100 points red rectangle
pageContent.q();
pageContent.k(0,100,100,0);
pageContent.re(200,600,200,100);
pageContent.f();
pageContent.Q();

// draw a gray line
pageContent.q();
pageContent.G(0.5);
pageContent.w(3);
pageContent.m(200,600);
pageContent.l(400,400);
pageContent.S();
pageContent.Q();

// write page will automatically close the page context
pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok');