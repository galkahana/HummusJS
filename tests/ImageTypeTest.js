var hummus = require('../hummus');
var assert = require('chai').assert;

describe('ParseInfo', function() {
	it('should complete without error and read fields correctly', function() {
		var pdfWriter = hummus.createWriter(__dirname + '/output/DummyEmptyFile.pdf',{version:hummus.ePDFVersion14});
		
		// just using it for the pdfWriter instance. now check different image types
		assert.equal(pdfWriter.getImageType(__dirname + '/TestMaterials/images/otherStage.JPG'), 'JPEG', 'jpg image type');
		assert.equal(pdfWriter.getImageType(__dirname + '/TestMaterials/images/tiff/FLAG_T24.TIF'), 'TIFF', 'tiff image type');
		assert.equal(pdfWriter.getImageType(__dirname + '/TestMaterials/AddedItem.pdf'), 'PDF', 'pdf image type');
		assert.equal(pdfWriter.getImageType(__dirname + '/TestMaterials/fonts/arial.ttf'), undefined, 'undefined image type');

		pdfWriter.end();
	});
});