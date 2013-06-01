var hummus = require('../Hummus');
var inputFile = new hummus.InputFile('../deps/TestMaterials/fonts/LucidaGrande.ttc');

if(inputFile.getFilePath() != '../deps/TestMaterials/fonts/LucidaGrande.ttc')
    throw new Error('input file test error, file paths dont match');

console.log('done - ok');
