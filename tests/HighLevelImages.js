var pdfWriter = require('../hummus').createWriter('./output/HighLevelImages.pdf');
var page = pdfWriter.createPage(0,0,595,842);
var cxt = pdfWriter.startPageContentContext(page);

// simple image placement
cxt.drawImage(10,10,'../deps/TestMaterials/Images/soundcloud_logo.jpg')
   .drawImage(10,500,'../deps/TestMaterials/images/tiff/cramps.tif')
   .drawImage(0,0,'../deps/TestMaterials/XObjectContent.PDF');

pdfWriter.writePage(page);


page = pdfWriter.createPage(0,0,595,842);
var cxt = pdfWriter.startPageContentContext(page);

cxt.drawImage(10,10,'../deps/TestMaterials/images/tiff/multipage.tif',{index:2})
   .drawImage(10,10,'../deps/TestMaterials/Images/soundcloud_logo.jpg',{transformation:[0.25,0,0,0.25,0,0]})
   .drawImage(0,0,'../deps/TestMaterials/XObjectContent.PDF',{transformation:{width:100,height:100}})
   .drawImage(100,100,'../deps/TestMaterials/XObjectContent.PDF',{transformation:{width:100,height:100, proportional:true}});

var pathStrokeOptions = {color:'DarkMagenta', width:4};

cxt.drawRectangle(0,0,100,100,pathStrokeOptions);
cxt.drawRectangle(100,100,100,100,pathStrokeOptions);

var jpgDimensions = pdfWriter.getImageDimensions('../deps/TestMaterials/Images/soundcloud_logo.jpg');
cxt.drawRectangle(10,10,jpgDimensions.width/4,jpgDimensions.height/4,pathStrokeOptions);


pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok')