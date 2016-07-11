var hummus = require('../hummus');

describe('Xcryption', function() {
	describe('Strip PDF From Password', function() {
		it('should complete without error', function() {
			hummus.recrypt(
				__dirname + '/TestMaterials/PDFWithPassword.PDF',
				__dirname + '/output/RecryptPDFWithPasswordToNothing.PDF',
				{
					password:'user'
				});
		});
	});

	describe('Encrypt PDF With a Different Password', function() {
		it('should complete without error', function() {
			hummus.recrypt(
				__dirname + '/TestMaterials/PDFWithPassword.PDF',
				__dirname + '/output/RecryptPDFWithPasswordToNewPassword.PDF',
				{
					password:'user',
					userPassword: 'user1',
					ownerPassword: 'owner1',
					userProtectionFlag: 4					
				});
		});
	});

	describe('Encrypt PDF With a Password', function() {
		it('should complete without error', function() {
			hummus.recrypt(
				__dirname + '/TestMaterials/Original.pdf',
				__dirname + '/output/RecryptPDFOriginalToPasswordProtected.PDF',
				{
					userPassword: 'user1',
					ownerPassword: 'owner1',
					userProtectionFlag: 4					
				});
		});
	});	

	describe('Create a PDF With a Password', function() {
		it('should complete without error', function() {
			var pdfWriter = hummus.createWriter(
				__dirname + '/output/PDFWithPassword.pdf',
				{
					userPassword: 'user',
					ownerPassword: 'owner',
					userProtectionFlag: 4					
				});
			var page = pdfWriter.createPage(0,0,595,842);

			pdfWriter.startPageContentContext(page)
				.drawImage(10,100, __dirname + '/TestMaterials/images/soundcloud_logo.jpg')
				.writeText('Hello',10,50,{
					font: pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/arial.ttf'),
					size: 14,
					colorspace: 'gray',
					color: 0x00
				});

			pdfWriter.writePage(page);
			pdfWriter.end();			
		});
	});	

	describe('Decrypt PDF via Appending Pages to New PDF',function() {
		it('should complete without error', function() {
			var pdfWriter = hummus.createWriter(__dirname + '/output/PDFWithPasswordDecrypted.pdf');
			var copyingContext = pdfWriter.createPDFCopyingContext(__dirname + '/TestMaterials/BasicTIFFImagesTest.PDF');
			for(var i=0;i<copyingContext.getSourceDocumentParser().getPagesCount();++i) {
				copyingContext.appendPDFPageFromPDF(i);
			}
			pdfWriter.end();
		});		
	});

	describe('Modify encrypted document', function() {
		it('should complete without error', function() {
			var pdfWriter = hummus.createWriterToModify(__dirname + '/TestMaterials/PDFWithPassword.PDF', {
				modifiedFilePath: __dirname + '/output/PDFWithPasswordModified.pdf',
				userPassword: 'user'
			});

			// modify first page to include text
			var pageModifier = new hummus.PDFPageModifier(pdfWriter,0);
			pageModifier.startContext().getContext().writeText(
				'new text on encrypted page',
				10, 805,
				{font:pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/arial.ttf'),size:14,colorspace:'gray',color:0x00}
			);

			pageModifier.endContext().writePage();

			// add new page with an image
			var page = pdfWriter.createPage(0,0,595,842);

			pdfWriter.startPageContentContext(page)
				.drawImage(10,300, __dirname + '/TestMaterials/images/soundcloud_logo.jpg');

			pdfWriter.writePage(page);

			pdfWriter.end();
		});
	});	

});
