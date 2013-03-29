// start with binary objects
module.exports = require('./build/release/Hummus');

// define pdf versions enum
module.exports.ePDFVersion10 = 10;
module.exports.ePDFVersion11 = 11;
module.exports.ePDFVersion12 = 12;
module.exports.ePDFVersion13 = 13;
module.exports.ePDFVersion14 = 14;
module.exports.ePDFVersion15 = 15;
module.exports.ePDFVersion16 = 16;
module.exports.ePDFVersion17 = 17;

// procsets for resource inclusion
module.exports.KProcsetImageB = 'ImageB';
module.exports.KProcsetImageC = 'ImageC';
module.exports.KProcsetImageI = 'ImageI';
module.exports.kProcsetPDF = 'PDF';
module.exports.kProcsetText = 'Text';

// page range values
module.exports.eRangeTypeAll = 1;
module.exports.eRangeTypeSpecific = 2;

// EPDFPageBox enumerator, for embedding pages into forms
module.exports.ePDFPageBoxMediaBox = 1;
module.exports.ePDFPageBoxCropBox = 2;
module.exports.ePDFPageBoxBleedBox = 3;
module.exports.ePDFPageBoxTrimBox = 4;
module.exports.ePDFPageBoxArtBox = 5;
