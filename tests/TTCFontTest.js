var pdfWriter = require('../hummus').createWriter('./output/TTCTestLucidaGrande.PDF');
var page = pdfWriter.createPage(0,0,595,842);

var fontLucidaGrande0 = pdfWriter.getFontForFile('./TestMaterials/fonts/LucidaGrande.ttc',0);
var fontLucidaGrande1 = pdfWriter.getFontForFile('./TestMaterials/fonts/LucidaGrande.ttc',1);

pdfWriter.startPageContentContext(page).BT()
                                .k(0,0,0,1)
                                .Tf(fontLucidaGrande0,1)
                                .Tm(30,0,0,30,78.4252,662.8997)
                                .Tj('Hello World!')
                                .ET()
                                .BT()
                                .k(0,0,0,1)
                                .Tf(fontLucidaGrande1,1)
                                .Tm(30,0,0,30,78.4252,462.8997)
                                .Tj('Hello World!')
                                .ET();
pdfWriter.writePage(page)
        .end();

console.log('done - ok');
