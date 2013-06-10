var hummus = require('../hummus');


function prepareSampleEmptyPDF(inFileName)
{
    var pdfWriter = hummus.createWriter(inFileName);
    pdfWriter.writePage(pdfWriter.createPage(0,0,595,842)).end();
}

function createFormFromEmptyPage(inFileName)
{
    var pdfWriter = hummus.createWriter('./output/CreateFormFromEmptyPage.pdf');
    
    var formIDs = pdfWriter.createFormXObjectsFromPDF(inFileName,hummus.ePDFPageBoxMediaBox);
    
    var page = pdfWriter.createPage(0,0,595,842);
    
    pdfWriter.startPageContentContext(page).q()
                                    .cm(0.5,0,0,0.5,0,421)
                                    .doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[0]))
                                    .Q()
                                    .G(0)
                                    .w(1)
                                    .re(0,421,297.5,421)
                                    .S();
    

    pdfWriter.writePage(page)
            .end();

}

function createPageFromEmptyPage(inFileName)
{
    var pdfWriter = hummus.createWriter('./output/CreatePageFromEmptyPage.pdf');
    
    pdfWriter.appendPDFPagesFromPDF(inFileName);
    pdfWriter.appendPDFPagesFromPDF('./TestMaterials/XObjectContent.PDF');
    pdfWriter.end();
}

function mergeEmptyPagetoPage(inFileName)
{
    var pdfWriter = hummus.createWriter('./output/MergeEmptyPageToPage.pdf');
    var page = pdfWriter.createPage(0,0,595,842);
    
    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/arial.ttf');
    
    var contentContext = pdfWriter.startPageContentContext(page).BT()
                                                                .k(0,0,0,1)
                                                                .Tf(font,30)
                                                                .Tm(1,0,0,1,10,600)
                                                                .Tj("Testing file merge")
                                                                .ET()
                                                                .q()
                                                                .cm(0.5,0,0,0.5,0,0);
    pdfWriter.mergePDFPagesToPage(page,inFileName,{type:hummus.eRangeTypeSpecific,specificRanges:[[0,0]]});
    
    contentContext.Q()
                    .q()
                    .cm(1,0,0,1,30,500)
                    .k(0,100,100,0)
                    .re(0,0,200,100)
                    .f()
                    .Q();

    pdfWriter.writePage(page)
             .end();
}

function mergeEmptyPageToForm(inFileName)
{
    var pdfWriter = hummus.createWriter('./output/MergeEmptyPageToForm.pdf');
    
    var page = pdfWriter.createPage(0,0,595,842);
    var copyingContext = pdfWriter.createPDFCopyingContext(inFileName);
    var aForm = pdfWriter.createFormXObject(0,0,297.5,842);

    aForm.getContentContext().q()
                             .cm(0.5,0,0,0.5,0,421);
    copyingContext.mergePDFPageToFormXObject(aForm,0);
    aForm.getContentContext().Q();
    pdfWriter.endFormXObject(aForm);
    
    var formName = page.getResourcesDictionary().addFormXObjectMapping(aForm.id);
    
    pdfWriter.startPageContentContext(page).q()
                                        .doXObject(formName)
                                        .cm(1,0,0,1,297.5,0)
                                        .doXObject(formName)
                                        .Q();
    
    pdfWriter.writePage(page)
             .end();
    
}





var emptyFileName = './output/sampleEmptyFileForCopying.pdf';
prepareSampleEmptyPDF(emptyFileName);
createFormFromEmptyPage(emptyFileName);
createPageFromEmptyPage(emptyFileName);
mergeEmptyPagetoPage(emptyFileName);
mergeEmptyPageToForm(emptyFileName);

console.log('done - ok');
