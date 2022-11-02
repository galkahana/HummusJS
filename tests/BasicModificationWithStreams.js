var hummus = require('../hummus');
const chai = require('chai');

describe('BasicModificationWithStreams', function() {
	it('should complete without error', function() {
		var inStream = new hummus.PDFRStreamForFile(__dirname + '/TestMaterials/MultipleChange.pdf');
		var outStream = new hummus.PDFWStreamForFile(__dirname + '/output/BasicModificationWithStreams.pdf');
		var pdfWriter = hummus.createWriterToModify(inStream,outStream);
		var pageModifier = new hummus.PDFPageModifier(pdfWriter, 0);

		pageModifier.startContext().getContext().writeText('Some added Text', 75, 805, {
			font: pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Couri.ttf'),
			size:14,
			colorspace:'gray',
			color:0x00
		});
		pageModifier.endContext().writePage();

		pdfWriter.end();
		outStream.close();
		inStream.close();
	});

	it('null for stream should throw an error and not crash', function () {
		var res = new hummus.PDFStreamForResponse(null)
		chai.expect(
			hummus.createWriter.bind(undefined, res)
		).to.throw(/Cannot read propert.*(write)?.* of null/)
	})	
});
