
function createPageFormImage(inPDFWriter,inImageForm)
{
    var page = inPDFWriter.createPage(0,0,595,842);
    inPDFWriter.startPageContentContext(page).q()
                                    .cm(1,0,0,1,0,0)
                                    .doXObject(inImageForm)
                                    .Q();
    inPDFWriter.writePage(page);
}


var pdfWriter = require('../hummus').createWriter('./output/TiffSpecialsTest.pdf');

for(var i=0;i<4;++i)
{
    var imageForm = pdfWriter.createFormXObjectFromTIFF('./TestMaterials/images/tiff/multipage.tif',{pageIndex : i});
    createPageFormImage(pdfWriter,imageForm);
}

{
    var imagePath = './TestMaterials/images/tiff/jim___ah.tif';
    var imageBW = pdfWriter.createFormXObjectFromTIFF(imagePath);
    var imageBWMask = pdfWriter.createFormXObjectFromTIFF(imagePath,{bwTreatment:{asImageMask:true , oneColor:[255,128,0]}});
    var page = pdfWriter.createPage(0,0,595,842);

    pdfWriter.startPageContentContext(page).q()
                                            .cm(1,0,0,1,0,842-195.12)
                                            .doXObject(imageBW)
                                            .Q()
                                            .q()
                                            .cm(1,0,0,1,159.36,842-195.12)
                                            .rg(0,0,1)
                                            .re(0,0,159.36,195.12)
                                            .f()
                                            .doXObject(imageBWMask)
                                            .Q();

    pdfWriter.writePage(page);
}

{
    var imagePath = './TestMaterials/images/tiff/jim___cg.tif';
    
    var imageGrayScale = pdfWriter.createFormXObjectFromTIFF(imagePath);
    var imageGrayScaleGreen = pdfWriter.createFormXObjectFromTIFF(imagePath,{grayscaleTreatment : {asColorMap:true, oneColor:[0,255,0], zeroColor:[255,255,255]}});
    var imageGrayScaleCyanMagenta = pdfWriter.createFormXObjectFromTIFF(imagePath,{grayscaleTreatment : {asColorMap:true, oneColor:[255,255,0,0], zeroColor:[0,0,0,0]}});
    var imageGrayScaleGreenVSRed = pdfWriter.createFormXObjectFromTIFF(imagePath,{grayscaleTreatment : {asColorMap:true, oneColor:[0,255,0], zeroColor:[255,0,0]}});
    var imageGrayScaleCyanVSMagenta = pdfWriter.createFormXObjectFromTIFF(imagePath,{grayscaleTreatment : {asColorMap:true, oneColor:[255,0,0,0], zeroColor:[0,255,0,0]}});

    var page = pdfWriter.createPage(0,0,595,842);
    
    pdfWriter.startPageContentContext(page).q()
                                            .cm(1,0,0,1,0,842-195.12)
                                            .doXObject(imageGrayScale)
                                            .Q()
                                            .q()
                                            .cm(1,0,0,1,159.36,842-195.12)
                                            .doXObject(imageGrayScaleGreen)
                                            .Q()
                                            .q()
                                            .cm(1,0,0,1,159.36*2,842-195.12)
                                            .doXObject(imageGrayScaleCyanMagenta)
                                            .Q()
                                            .q()
                                            .cm(1,0,0,1,0,842 - 195.12*2)
                                            .doXObject(imageGrayScaleGreenVSRed)
                                            .Q()
                                            .q()
                                            .cm(1,0,0,1,159.36,842 - 195.12*2)
                                            .doXObject(imageGrayScaleCyanVSMagenta)
                                            .Q();
    pdfWriter.writePage(page);
    
}

pdfWriter.end();
console.log('done - ok');

