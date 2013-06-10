var hummus = require('../hummus');
var inputFile = new hummus.InputFile('./TestMaterials/fonts/LucidaGrande.ttc');

if(inputFile.getFilePath() != './TestMaterials/fonts/LucidaGrande.ttc')
    throw new Error('input file test error, file paths dont match');

console.log('done - ok');
