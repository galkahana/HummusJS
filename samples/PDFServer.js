var express = require('express');
var app = express();


app.get('/', function(req, res){
        res.writeHead(200, {'Content-Type': 'application/pdf'});
        
        var hummus = require('hummus');
        
        var pdfWriter = hummus.createWriter(new hummus.PDFStreamForResponse(res));
        var page = pdfWriter.createPage(0,0,595,842);
        pdfWriter.startPageContentContext(page).writeText('Hello ' + (req.query.id ? req.query.id : 'World'),
                                                          0,400,
                                                          {
                                                            font:pdfWriter.getFontForFile('../tests/TestMaterials/fonts/arial.ttf'),
                                                            size:50,
                                                            colorspace:'gray',
                                                            color:0x00
                                                          });
        pdfWriter.writePage(page);
        pdfWriter.end();

        res.end();
        
        });

app.listen(3000);