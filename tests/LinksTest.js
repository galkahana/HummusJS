
var hummus = require('../hummus');
var pdfWriter = hummus.createWriter('./output/LinksTest.PDF');
var page = pdfWriter.createPage(0,0,595,842);

var soundCloudLogo = pdfWriter.createFormXObjectFromJPG('./TestMaterials/images/soundcloud_logo.jpg');
var font = pdfWriter.getFontForFile('./TestMaterials/fonts/arial.ttf');

var contentContext = pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(11,0,0,11,90.024,709.54)
                                        .Tj('http://pdfhummus.com')
                                        .ET()
                                        .q()
                                        .cm(0.5,0,0,0.5,90.024,200)
                                        .doXObject(soundCloudLogo)
                                        .Q();

// pausing page write so can create link objects
pdfWriter.pausePageContentContext(contentContext)
         .attachURLLinktoCurrentPage('http://www.pdfhummus.com',87.75,694.56,198.76,720)
         .attachURLLinktoCurrentPage('http://www.soundcloud.com',90.024,200,367.524,375)
         .writePage(page)
         .end();

console.log('done - ok');