var hummus = require('../hummus');
const expect = require('chai').expect;

describe('AppendPagesTest', function() {
	it('should complete without error', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/AppendPagesTest.pdf');

		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/Original.pdf');
		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/XObjectContent.PDF');
		pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/BasicTIFFImagesTest.PDF');

		pdfWriter.end();
	});

	it('should throw an error instead of a crash', () => {
		var writerBuffer = new hummus.PDFWStreamForBuffer([]);
		var pdfWriter = hummus.createWriter(writerBuffer)
		expect(() =>
			pdfWriter.appendPDFPagesFromPDF(__dirname + '/TestMaterials/appendbreaks.pdf')
		).to.throw('unable to append')
	})	
});
