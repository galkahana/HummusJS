var PDFTextString = require('../hummus').PDFTextString;
var assert = require('chai').assert;

describe('PDFTextString', function() {
	it('create PDFTextString correctly', function() {
        assert.equal(new PDFTextString().toString(), '', 'empty starter');
        assert.equal(new PDFTextString('Hello World').toString(), 'Hello World', 'string starter');
        assert.equal(new PDFTextString([72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100]).toString(), 'Hello World', 'bytes array starter');
        assert.equal(new PDFTextString(new PDFTextString('Hello World').toBytesArray()).toString(), 'Hello World', 'bytes array starter, again');
	});
});