var hummus = require('../hummus');
var fs = require('fs');

describe('StreamCopyingContext', function() {
	it('should complete without error', function() {
		var inStreamA = new hummus.PDFRStreamForFile(__dirname + '/TestMaterials/BasicJPGImagesTest.PDF');
		var inStreamB = new hummus.PDFRStreamForFile(__dirname + '/TestMaterials/AddedPage.pdf');
		var outStream = new hummus.PDFWStreamForFile(__dirname + '/output/StreamCopyingContext.pdf');

		var pdfWriter = hummus.createWriterToModify(inStreamB, outStream);

		var copyCtx = pdfWriter.createPDFCopyingContext(__dirname + '/TestMaterials/BasicJPGImagesTest.PDF');
		copyCtx.appendPDFPageFromPDF(0);

		var copyCtx = pdfWriter.createPDFCopyingContext(inStreamA);
		copyCtx.appendPDFPageFromPDF(0);

		pdfWriter.end();
		outStream.close();
		inStreamA.close();
		inStreamB.close();
	});
});
