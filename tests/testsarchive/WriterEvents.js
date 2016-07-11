var assert = require('chai').assert;

describe('WriterEvents', function() {
	it('should complete without error', function() {
		var pdfWriter = require('../hummus').createWriter(__dirname + '/output/WriterEvents.pdf');
        var onCatalogWriteCalled = 0;
        var onPageWriteCalled = 0;

        pdfWriter.getEvents().on('OnCatalogWrite',function(eventParams) {
            assert.equal(!!eventParams.catalogDictionaryContext, true, 'catalogDictionaryContext exists');
            ++onCatalogWriteCalled;
        });

        pdfWriter.getEvents().on('OnPageWrite',function(eventParams) {
            assert.equal(!!eventParams.page, true, 'page exists');
            assert.equal(!!eventParams.pageDictionaryContext, true, 'pageDictionaryContext exists');
            ++onPageWriteCalled;
        });


		var page = pdfWriter.createPage(0,0,595,842);
		var cxt = pdfWriter.startPageContentContext(page);

		var textOptions = {
			font: pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/arial.ttf'),
			size: 14,
			colorspace: 'gray',
			color: 0x00,
			underline: true
		};

		var pathFillOptions = {color:0xFF000000, colorspace:'cmyk', type:'fill'};
		var pathStrokeOptions = {color:'DarkMagenta', width:4};

		cxt.drawPath(75,640,149,800,225,640,pathFillOptions)
			.drawPath(75,540,110,440,149,540,188,440,223,540,pathStrokeOptions);

		pdfWriter.writePage(page);

		var page1 = pdfWriter.createPage(0,0,595,842);
		var cxt1 = pdfWriter.startPageContentContext(page);

		pdfWriter.writePage(page1);

		pdfWriter.end();
        
        assert.equal(onCatalogWriteCalled, 1, 'catalog write called once');
        assert.equal(onPageWriteCalled, 2, 'page write called twice');
	});
});