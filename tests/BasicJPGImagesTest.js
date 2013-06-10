var pdfWriter = require('../hummus').createWriter('./output/BasicJPGImagesTest.PDF');

var page = pdfWriter.createPage(0,0,595,842);
var contentContext = pdfWriter.startPageContentContext(page).q()
                                                            .k(100,0,0,0)
                                                            .re(500,0,100,100)
                                                            .f()
                                                            .Q();
// pause  page content placement so i can now put image data into the file
pdfWriter.pausePageContentContext(contentContext);

var imageXObject = pdfWriter.createImageXObjectFromJPG('./TestMaterials/images/otherStage.JPG');

// now continue with page content placement
contentContext.q()
                .cm(500,0,0,400,0,0)
                .doXObject(imageXObject)
                .Q();

// now the same, but with form (which will already have the right size)
pdfWriter.pausePageContentContext(contentContext);

var formXObject = pdfWriter.createFormXObjectFromJPG('./TestMaterials/images/otherStage.JPG');
contentContext.q()
                .cm(1,0,0,1,0,400)
                .doXObject(formXObject)
                .Q();


pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok');