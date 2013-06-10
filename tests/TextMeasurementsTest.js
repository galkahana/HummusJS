var pdfWriter = require('../hummus').createWriter('./output/TextMeasurementsTest.pdf');
var page = pdfWriter.createPage(0,0,595,842);
var cxt = pdfWriter.startPageContentContext(page);
var arialFont = pdfWriter.getFontForFile('./TestMaterials/fonts/arial.ttf');
var pathStrokeOptions = {color:'DarkMagenta', width:4};

var textOptions = {font:arialFont,size:14,colorspace:'gray',color:0x00};

// write some text, with top and bottom lines, which position is based on the text dimensions
var textDimensions = arialFont.calculateTextDimensions('Hello World',14);

cxt.writeText('Hello World',10,100,textOptions)
   .drawPath(10+textDimensions.xMin,98+textDimensions.yMin,10+textDimensions.xMax,98+textDimensions.yMin,pathStrokeOptions)
   .drawPath(10+textDimensions.xMin,102+textDimensions.yMax,10+textDimensions.xMax,102+textDimensions.yMax,pathStrokeOptions)

pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok')