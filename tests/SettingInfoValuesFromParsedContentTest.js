var hummus = require('../hummus');
var fs = require('fs');

var outStream = new hummus.PDFWStreamForFile('./output/SettingInfoValuesFromParsedContent.pdf');
var pdfWriter = hummus.createWriter(outStream);

var copyCtx = pdfWriter.createPDFCopyingContext('./TestMaterials/SettingInfoValues.PDF');

var info = copyCtx.getSourceDocumentParser().queryDictionaryObject(
    copyCtx.getSourceDocumentParser().getTrailer(), "Info"
).toJSObject();

var newInfo = pdfWriter.getDocumentContext().getInfoDictionary();

newInfo.author = info.Author.toText();
console.log('encoded author',info.Author.value);
console.log('decoded author',info.Author.toText());

newInfo.creator = info.Creator.toText();
console.log('encoded creator',info.Creator.value);
console.log('decoded creator',info.Creator.toText());

//errors - date serialization?
newInfo.setCreationDate(info.CreationDate.value);
console.log('creation date encoded',info.CreationDate.value);

var page = pdfWriter.createPage();
page.mediaBox = [0,0,595,842];
pdfWriter.writePage(page);

pdfWriter.end();
outStream.close();

console.log('done - ok');