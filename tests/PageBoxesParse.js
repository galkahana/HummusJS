var hummus = require('../hummus');
var assert = require('chai').assert;
var fs = require('fs');

describe('ParseBoxes', function() {
	it('should read page boxes and rotate correctly', function() {
		var pdfReader = hummus.createReader(__dirname + '/TestMaterials/PageBoxes.pdf');
		var pageInfo = pdfReader.parsePage(0);

		// media box
		assert.deepEqual(pageInfo.getMediaBox(), [0,0,595,842], 'defined media box');

		// crop box
		assert.deepEqual(pageInfo.getCropBox(), [1,1,594,841], 'defined crop box');

		// bleed box
		assert.deepEqual(pageInfo.getBleedBox(), [2,2,593,840], 'defined bleed box');

		// trim box
		assert.deepEqual(pageInfo.getTrimBox(), [3,3,592,839], 'defined trim box');

		// art box
		assert.deepEqual(pageInfo.getArtBox(), [4,4,591,838], 'defined art box');

		// rotate
		assert.equal(pageInfo.getRotate(), 90, 'defined rotate');

	});
});
