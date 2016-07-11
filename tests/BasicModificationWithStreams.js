var hummus = require('../hummus');

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
});
