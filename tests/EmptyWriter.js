var hummus = require('../hummus');

describe('EmptyWriter', function() {
	it('should complete without error', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/EmptyWriter.pdf', {
			version:hummus.ePDFVersion14
		});
		pdfWriter.end();
	});
});
