var pdfWriter = require('../Hummus.js').createWriter('./output/MergeToPDFForm.pdf');
var page = pdfWriter.createPage(0,0,595,842);
var copyingContext = pdfWriter.createPDFCopyingContext('./TestMaterials/Linearized.pdf');
var form = pdfWriter.createFormXObject(0,0,297.5,842);

form.getContentContext().q()
                        .cm(0.5,0,0,0.5,0,0);
copyingContext.mergePDFPageToFormXObject(form,1);
form.getContentContext().Q();
pdfWriter.endFormXObject(form);

var formName = page.getResourcesDictionary().addFormXObjectMapping(form.id);

pdfWriter.startPageContentContext(page).q()
                                    .doXObject(formName)
                                    .cm(1,0,0,1,297.5,0)
                                    .doXObject(formName)
                                    .Q();

pdfWriter.writePage(page)
            .end();

console.log('done - ok');