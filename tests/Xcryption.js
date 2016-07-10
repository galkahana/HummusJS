var hummus = require('../hummus');

describe('Xcryption', function() {
	describe('Strip PDF From Password', function() {
		it('should complete without error', function() {
			hummus.recrypt(
				__dirname + '/TestMaterials/PDFWithPassword.PDF',
				__dirname + '/Output/RecryptPDFWithPasswordToNothing.PDF',
				{
					password:'user'
				});
		});
	});

	describe('Encrypt PDF With a Different Password', function() {
		it('should complete without error', function() {
			hummus.recrypt(
				__dirname + '/TestMaterials/PDFWithPassword.PDF',
				__dirname + '/Output/RecryptPDFWithPasswordToNewPassword.PDF',
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
				__dirname + '/TestMaterials/Original.PDF',
				__dirname + '/Output/RecryptPDFOriginalToPasswordProtected.PDF',
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

});
