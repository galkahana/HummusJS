var hummus = require('../hummus');

describe('PDFEmbedTest', function() {
	it('should complete without error', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/PDFEmbedTest.pdf');
		var formIDs = pdfWriter.createFormXObjectsFromPDF(__dirname + '/TestMaterials/XObjectContent.PDF',hummus.ePDFPageBoxMediaBox);
		var page = pdfWriter.createPage(0,0,595,842);

		pdfWriter.startPageContentContext(page)
			.q()
			.cm(0.5,0,0,0.5,0,421)
			.doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[0]))
			.Q()
			.G(0)
			.w(1)
			.re(0,421,297.5,421)
			.S()
			.q()
			.cm(0.5,0,0,0.5,297.5,0)
			.doXObject(page.getResourcesDictionary().addFormXObjectMapping(formIDs[1]))
			.Q()
			.G(0)
			.w(1)
			.re(297.5,0,297.5,421)
			.S();

		pdfWriter.writePage(page).end();
	});
});
