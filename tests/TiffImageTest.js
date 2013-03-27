function addPageForTiff(inPDFWriter,inTiffPath)
{
    var page = inPDFWriter.createPage(0,0,595,842);
    var tiffForm = inPDFWriter.createFormXObjectFromTIFFFile(inTiffPath);
    
    inPDFWriter.startPageContentContext(page).q()
                                            .cm(1,0,0,1,0,0)
                                            .doXObject(tiffForm)
                                            .Q();
    
    inPDFWriter.writePage(page);
}



var hummus = require('../Hummus');

var pdfWriter = hummus.createWriter('./output/TiffImageTest.PDF');

for(var i=1;i<9;++i)
    addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/CCITT_' + i + '.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/FLAG_T24.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/G4.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/G4S.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/G31D.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/G31DS.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/G32D.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/GMARBLES.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/MARBIBM.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/MARBLES.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/XING_T24.TIF');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/cramps-tile.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/cramps.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/dscf0013.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/fax2d.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/g3test.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/jello.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/jim___ah.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/jim___cg.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/jim___dg.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/jim___gg.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/oxford.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/quad-lzw.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/quad-tile.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/strike.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/ycbcr-cat.tif');
for(var i=2;i<9;i=i*2)
    addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-minisblack-' + i + '.tif');
for(var i=2;i<9;i=i*2)
    addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-palette-' + i + '.tif');
for(var i=2;i<9;i=i*2)
    addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-rgb-contig-' + i + '.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-rgb-planar-8.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-separated-contig-8.tif');
addPageForTiff(pdfWriter,'../deps/TestMaterials/Images/tiff/flower-separated-planar-8.tif');

pdfWriter.end();

console.log('Done - ok');
