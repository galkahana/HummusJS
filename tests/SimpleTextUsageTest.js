var hummus = require('../hummus');

{
    var pdfWriter = hummus.createWriter('./output/SimpleTextUsageCFF.pdf');

    var page = pdfWriter.createPage(0,0,595,842);

    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/BrushScriptStd.otf');
    var fontK = pdfWriter.getFontForFile('./TestMaterials/fonts/KozGoPro-Regular.otf');
    pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(30,0,0,30,78.4252,662.8997)
                                        .Tj('abcd')
                                        .ET()
                                        .BT()
                                        .k(0,0,0,1)
                                        .Tf(fontK,1)
                                        .Tm(30,0,0,30,78.4252,400.8997)
                                        .Tj('abcd')
                                        .ET();
    pdfWriter.writePage(page)
            .end();
}

{
    var pdfWriter = hummus.createWriter('./output/SimpleTextUsageTTF.pdf');
    
    var page = pdfWriter.createPage(0,0,595,842);
    
    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/arial.ttf');
    pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(30,0,0,30,78.4252,662.8997)
                                        .Tj('abcd')
                                        .ET();
    pdfWriter.writePage(page)
            .end();
}

{
    var pdfWriter = hummus.createWriter('./output/SimpleTextUsageType1.pdf');
    
    var page = pdfWriter.createPage(0,0,595,842);
    
    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/HLB_____.PFB',
                                        './TestMaterials/fonts/HLB_____.PFM');
    pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(30,0,0,30,78.4252,662.8997)
                                        .Tj('abcd')
                                        .ET();
    pdfWriter.writePage(page)
            .end();
}


// this one is about creating a font object, but not really using it. make sure no crash happens
{
    var pdfWriter = hummus.createWriter('./output/SimpleTextUsageType1Empty.pdf');
    
    var page = pdfWriter.createPage(0,0,595,842);
    
    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/HLB_____.PFB',
                                        './TestMaterials/fonts/HLB_____.PFM');
    pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(30,0,0,30,78.4252,662.8997)
                                        .ET();
    pdfWriter.writePage(page)
            .end();
}
// this one adds text using the GlyphIds
{
    var pdfWriter = hummus.createWriter('./output/SimpleTextUsageGlyphs.pdf');
    
    var page = pdfWriter.createPage(0,0,595,842);
    
    var font = pdfWriter.getFontForFile('./TestMaterials/fonts/arial.ttf');
    pdfWriter.startPageContentContext(page).BT()
                                        .k(0,0,0,1)
                                        .Tf(font,1)
                                        .Tm(30,0,0,30,78.4252,662.8997)
                                        .Tj([[68,97],[69,98],[70,99],[71,100]])
                                        .ET();
    pdfWriter.writePage(page)
            .end();
}


console.log('done - ok');