
var hummus = require('../hummus');
var pdfWriter = hummus.createWriter('./output/SettingInfoValues.pdf',{version:hummus.ePDFVersion14});

// set the document author, title, subject, creator, creation date, additional info
var infoDictionary = pdfWriter.getDocumentContext().getInfoDictionary();

infoDictionary.author = 'Gal Kahana';
infoDictionary.title = 'PDFHummus explained';
infoDictionary.subject = 'On the wonders of one-pass PDF file generation, and how it can save the world';
infoDictionary.creator = 'PDFHummus';
infoDictionary.setCreationDate(new Date());
infoDictionary.addAdditionalInfoEntry('words of praise','amazing');

// create empty page
var page = pdfWriter.createPage();
page.mediaBox = [0,0,595,842];
pdfWriter.writePage(page);
pdfWriter.end();

console.log('done - ok');