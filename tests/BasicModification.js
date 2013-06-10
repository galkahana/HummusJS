var hummus = require('../hummus');


function testInPlaceFileModification(inFileName)
{
    var fs = require('fs');
    
    var ws = fs.createWriteStream('./output/InPlaceModified' + inFileName + '.pdf');
    var rs = fs.createReadStream('./TestMaterials/' + inFileName + '.pdf');
    ws.on('close', function()
           {
            var pdfWriter = hummus.createWriterToModify('./output/InPlaceModified' + inFileName + '.pdf');
            var page = pdfWriter.createPage(0,0,595,842);
           
            pdfWriter.startPageContentContext(page).BT()
                                                    .k(0,0,0,1)
                                                    .Tf(pdfWriter.getFontForFile('./TestMaterials/fonts/Courier.dfont',0),1)
                                                    .Tm(30,0,0,30,78.4252,662.8997)
                                                    .Tj('about')
                                                    .ET();
           pdfWriter.writePage(page);
           pdfWriter.end();
           });
    rs.pipe(ws);
}


testInPlaceFileModification('Linearized');
testInPlaceFileModification('MultipleChange');
testInPlaceFileModification('RemovedItem');
testInPlaceFileModification('ObjectStreams');
testInPlaceFileModification('ObjectStreamsModified');

console.log('done - ok');