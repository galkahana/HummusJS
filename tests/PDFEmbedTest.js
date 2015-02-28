var hummus = require('../hummus');
var pdfWriter = hummus.createWriter('./output/PDFEmbedTest.pdf');

var formIDs = pdfWriter.createFormXObjectsFromPDF('./TestMaterials/XObjectContent.PDF',hummus.ePDFPageBoxMediaBox);

var page = pdfWriter.createPage(0,0,595,842);

pdfWriter.startPageContentContext(page).q()
                                    .cm(0.5,0,0,0.5,0,421)
                                    .doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[0]))
                                    .Q()
                                    .G(0)
                                    .w(1)
                                    .re(0,421,297.5,421)
                                    .S()
                                    .q()
                                    .cm(0.5,0,0,0.5,297.5,0)
                                    .doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[1]))
                                    .Q()
                                    .G(0)
                                    .w(1)
                                    .re(297.5,0,297.5,421)
                                    .S();

pdfWriter.writePage(page)
         .end();

console.log('done - ok');