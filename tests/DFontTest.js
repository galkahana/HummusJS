describe('DFontTest', function() {
	it('should complete without error', function() {
		var pdfWriter = require('../hummus').createWriter(__dirname + '/output/DFontTest.pdf');
		var page = pdfWriter.createPage(0,0,595,842);
		var contentContext = pdfWriter.startPageContentContext(page);

		[
			pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Courier.dfont',0),
			pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Courier.dfont',1),
			pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Courier.dfont',2),
			pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Courier.dfont',3)
		].forEach(function(element,index,array) {
			contentContext
				.BT()
				.k(0,0,0,1)
				.Tf(element,1)
				.Tm(30,0,0,30,78.4252,662.8997 - index*100)
				.Tj('Hello World!')
				.ET();
		});
		pdfWriter.writePage(page).end();
	});
});
