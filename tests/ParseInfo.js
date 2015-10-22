var hummus = require('../hummus');
var assert = require('chai').assert;
var fs = require('fs');

describe('ParseInfo', function() {
	it('should read fields correctly', function() {
		console.log('createReader');
		var pdfReader = hummus.createReader(__dirname + '/TestMaterials/SettingInfoValues.pdf');
		console.log('queryDictionaryObject');
		var info = pdfReader.queryDictionaryObject(pdfReader.getTrailer(), "Info").toJSObject();
		console.log('values');

		// TODO: figure out what's causing these to segfault on linux
		// assert.equal(info.Author.value, 'Gal Kahana', 'author value');
		assert.equal(info.Author.toText(), 'Gal Kahana', 'author decoded');
		// assert.equal(info.Author.value, 'Gal Kahana', 'creator value');
		assert.equal(info.Author.toText(), 'Gal Kahana', 'creator decoded');
		// assert.equal(info.CreationDate.value, 'D:20140720204655+03\'00\'', 'creation date value');
		console.log('done');
	});
});
