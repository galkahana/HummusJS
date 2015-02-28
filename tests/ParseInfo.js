var hummus = require('../hummus');
var fs = require('fs');

var pdfReader = hummus.createReader('./TestMaterials/SettingInfoValues.PDF');

var info = pdfReader.queryDictionaryObject(pdfReader.getTrailer(), "Info").toJSObject();

console.log('author value',info.Author.value);
console.log('author decoded',info.Author.toText());
console.log('creator value',info.Author.value);
console.log('creator decoded',info.Author.toText());
console.log('creation date value',info.CreationDate.value);

console.log('done - ok');