var hummus = require('../hummus');
var assert = require('assert');

function testBasicFileModification(inFileName, throws) {
	describe(inFileName, function() {
		it(throws ? 'should error' : 'should complete without error', function() {
			var wrapper = throws ? assert.throws : function(fn) { fn(); };

			wrapper(function() {
				var pdfWriter = hummus.createWriterToModify(__dirname + '/TestMaterials/' + inFileName + '.pdf', {
					modifiedFilePath: __dirname+'/output/Modified' + inFileName + '.pdf'
				});

				var page = pdfWriter.createPage(0,0,595,842);
				pdfWriter.startPageContentContext(page)
					.BT()
					.k(0,0,0,1)
					.Tf(pdfWriter.getFontForFile(__dirname + '/TestMaterials/fonts/Courier.dfont',0),1)
					.Tm(30,0,0,30,78.4252,662.8997)
					.Tj('about')
					.ET();

				pdfWriter.writePage(page);
				pdfWriter.end();
			});
		});
	});
}

describe('BasicModification2', function() {
	testBasicFileModification('Linearized');
	testBasicFileModification('MultipleChange');
	testBasicFileModification('RemovedItem');
	testBasicFileModification('Protected', true);
	testBasicFileModification('ObjectStreams');
	testBasicFileModification('ObjectStreamsModified');
});
