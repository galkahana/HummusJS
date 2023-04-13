var express = require('express');
var app = express();

console.log('Starting express server on port 3000');

app.get('/', function(req, res) {
    const fileName = __dirname + '/PDFWithPasswordAES.pdf';

    var hummus = require('../../hummus');

    var pdfWriter = hummus.createWriter(
        fileName,
        {
          userPassword: 'user',
          ownerPassword: 'owner',
          userProtectionFlag: 4,
          version: hummus.ePDFVersion16
        }
    );

    var page = pdfWriter.createPage(0,0,595,842);

    pdfWriter
        .startPageContentContext(page)
        .writeText(
            'Hello World',
            10,400,
            {
              font:pdfWriter.getFontForFile('../../tests/TestMaterials/fonts/arial.ttf'),
              size:50,
              colorspace:'gray',
              color:0x00
            });

    pdfWriter.writePage(page);
    pdfWriter.end();

    res.sendFile(fileName);
});

app.listen(3000);
