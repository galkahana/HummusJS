describe('BasicPNGImagesTest', function() {
    it('should complete without error', function() {
        var pdfWriter = require('../hummus').createWriter(__dirname + '/output/BasicPNGImagesTest.pdf',{log:__dirname + '/output/BasicPNGImagesTest.log'});

        var pathFillOptions = {color:0xFF0000, colorspace:'rgb', type:'fill'};
        var imageOptions = {transformation:[0.5,0,0,0.5,0,0]}

        var page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page)
            .drawRectangle(0,0,595,842,pathFillOptions)
            .drawImage(10,200,__dirname+'/TestMaterials/images/png/original.png',imageOptions);
        pdfWriter.writePage(page);

        page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page)
            .drawRectangle(0,0,595,842,pathFillOptions)
            .drawImage(10,200,__dirname+'/TestMaterials/images/png/original_transparent.png',imageOptions);
        pdfWriter.writePage(page);

        page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page)
            .drawRectangle(0,0,595,842,pathFillOptions)
            .drawImage(10,200,__dirname+'/TestMaterials/images/png/pnglogo-grr.png',imageOptions);
        pdfWriter.writePage(page);

        page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page)
            .drawRectangle(0,0,595,842,pathFillOptions)
            .drawImage(10,200,__dirname+'/TestMaterials/images/png/gray-alpha-8-linear.png',imageOptions);
        pdfWriter.writePage(page);

        page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page)
            .drawRectangle(0,0,595,842,pathFillOptions)
            .drawImage(10,200,__dirname+'/TestMaterials/images/png/gray-16-linear.png',imageOptions);
        pdfWriter.writePage(page);


        pdfWriter.end();
    });
});
