var hummus = require('../hummus');

describe('EmptyPagesTest', function() {
	it('should complete without error', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/EmptyPages.pdf',{version:hummus.ePDFVersion14});
		var page = pdfWriter.createPage();

		page.mediaBox = [0,0,595,842];
		for (var i=0; i < 4; ++i) {
			pdfWriter.writePage(page);
		}

		pdfWriter.end();
	});
});
