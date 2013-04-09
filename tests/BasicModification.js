var hummus = require('../Hummus');
var assert = require('assert');

function testBasicFileModification(inFileName)
{
    var pdfWriter = hummus.createWriterToModify('../deps/TestMaterials/' + inFileName + '.pdf','./Output/Modified' + inFileName + '.pdf');
    var page = pdfWriter.createPage(0,0,595,842);

    pdfWriter.startPageContentContext(page).BT()
                                            .k(0,0,0,1)
                                            .Tf(pdfWriter.getFontForFile('../deps/TestMaterials/fonts/Courier.dfont',0),1)
                                            .Tm(30,0,0,30,78.4252,662.8997)
                                            .Tj('about')
                                            .ET();
    
    pdfWriter.writePage(page);
    pdfWriter.end();
}

function testInPlaceFileModification(inFileName)
{
    var fs = require('fs');
    
    var ws = fs.createWriteStream('./Output/InPlaceModified' + inFileName + '.pdf');
    ws.on('close', function()
           {
            var pdfWriter = hummus.createWriterToModify('./Output/InPlaceModified' + inFileName + '.pdf');
            var page = pdfWriter.createPage(0,0,595,842);
           
            pdfWriter.startPageContentContext(page).BT()
                                                    .k(0,0,0,1)
                                                    .Tf(pdfWriter.getFontForFile('../deps/TestMaterials/fonts/Courier.dfont',0),1)
                                                    .Tm(30,0,0,30,78.4252,662.8997)
                                                    .Tj('about')
                                                    .ET();
           pdfWriter.writePage(page);
           pdfWriter.end();
           });
    fs.createReadStream('../deps/TestMaterials/' + inFileName + '.pdf').pipe(ws);
}

testBasicFileModification('BasicTIFFImagesTest');
testBasicFileModification('Linearized');
testBasicFileModification('MultipleChange');
testBasicFileModification('RemovedItem');
assert.throws(function(){pdfWriter.testBasicFileModification('Protected')});
testBasicFileModification('ObjectStreams');
testBasicFileModification('ObjectStreamsModified');
testInPlaceFileModification('BasicTIFFImagesTest');
testInPlaceFileModification('Linearized');
testInPlaceFileModification('MultipleChange');
testInPlaceFileModification('RemovedItem');
testInPlaceFileModification('ObjectStreams');
testInPlaceFileModification('ObjectStreamsModified');

console.log('done - ok');