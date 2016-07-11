var hummus = require('../hummus');
var assert = require('chai').assert;
var fs = require('fs');

describe('ParseInfo', function() {
	it('should read fields correctly', function() {
		var pdfReader = hummus.createReader(__dirname + '/TestMaterials/SettingInfoValues.pdf');
		var info = pdfReader.queryDictionaryObject(pdfReader.getTrailer(), "Info").toJSObject();

		assert.equal(info.Author.value, 'Gal Kahana', 'author value');
		assert.equal(info.Author.toText(), 'Gal Kahana', 'author decoded');
		assert.equal(info.Author.value, 'Gal Kahana', 'creator value');
		assert.equal(info.Author.toText(), 'Gal Kahana', 'creator decoded');
		assert.equal(info.CreationDate.value, 'D:20140720204655+03\'00\'', 'creation date value');
	});
});
