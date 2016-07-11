describe('AppendPagesTest', function() {
	it('should complete without error', function() {
		var pdfWriter = require('../hummus').createWriter(__dirname + '/output/AppendPagesTest.pdf');

		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/Original.pdf');
		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/XObjectContent.PDF');
		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/BasicTIFFImagesTest.PDF');

		pdfWriter.end();
	});
});
