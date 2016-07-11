var hummus = require('../hummus');

describe('SettingInfoValues', function() {
	it('should complete without error', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/SettingInfoValues.pdf',{version:hummus.ePDFVersion14});

		// set the document author, title, subject, creator, creation date, additional info
		var infoDictionary = pdfWriter.getDocumentContext().getInfoDictionary();

		infoDictionary.author = 'Gal Kahana';
		infoDictionary.title = 'PDFHummus explained';
		infoDictionary.subject = 'On the wonders of one-pass PDF file generation, and how it can save the world';
		infoDictionary.creator = 'PDFHummus';
		infoDictionary.setCreationDate("D:20140720204655+03'00'"); // two options to set dates: 1. PDF encoded string (like what comes from the parser)
		infoDictionary.setModDate(new Date()); // 2. javascript date
		infoDictionary.addAdditionalInfoEntry('words of praise','amazing');

		// create empty page
		var page = pdfWriter.createPage();
		page.mediaBox = [0,0,595,842];
		pdfWriter.writePage(page);
		pdfWriter.end();
	});
});
