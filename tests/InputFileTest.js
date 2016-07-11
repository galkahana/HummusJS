var hummus = require('../hummus');

describe('InputFileTest', function() {
	it('should complete without error', function() {
		var inputFile = new hummus.InputFile(__dirname + '/TestMaterials/fonts/LucidaGrande.ttc');
		if (inputFile.getFilePath() != __dirname + '/TestMaterials/fonts/LucidaGrande.ttc') {
		    throw new Error('input file test error, file paths dont match');
		}
	});
});

