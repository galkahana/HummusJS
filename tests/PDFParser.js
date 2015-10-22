var hummus = require('../hummus');
var assert = require('chai').assert;
var fs = require('fs');

describe('PDFParser', function() {
	it('should complete without error', function() {
		var mTabLevel = 0;
		var mIteratedObjectIDs = {};
		var outputFile = fs.openSync(__dirname + '/output/parseLog.txt','w');
		function logToFile(inString) {
			fs.writeSync(outputFile,addTabs() + inString + '\r\n');
		}

		function addTabs() {
			var output='';
			for (var i=0;i<mTabLevel;++i) {
				output+=' ';
			}
			return output;
		}

		function iterateObjectTypes(inObject,inReader) {
			var output = '';

			if (inObject.getType() == hummus.ePDFObjectIndirectObjectReference) {
				output+= 'Indirect object reference:';
				logToFile(output);
				var objectID = inObject.toPDFIndirectObjectReference().getObjectID();
				if (!mIteratedObjectIDs.hasOwnProperty(objectID)) {
					mIteratedObjectIDs[objectID] = true;
					iterateObjectTypes(inReader.parseNewObject(objectID),inReader);
				}
				for (var i=0;i<mTabLevel;++i) {
					output+=' ';
				}
				output+='was parsed already';
				logToFile(output);
			} else if (inObject.getType() == hummus.ePDFObjectArray) {
				output+= hummus.getTypeLabel(inObject.getType());
				logToFile(output);
				++mTabLevel;
				inObject.toPDFArray().toJSArray().forEach(function(element, index, array){iterateObjectTypes(element,inReader);});
				--mTabLevel;
			} else if (inObject.getType() == hummus.ePDFObjectDictionary) {
				output+= hummus.getTypeLabel(inObject.getType());
				logToFile(output);
				++mTabLevel;
				var aDictionary = inObject.toPDFDictionary().toJSObject();

				Object.getOwnPropertyNames(aDictionary).forEach(function(element,index,array)
					{
						logToFile(element);
						iterateObjectTypes(aDictionary[element],inReader);
					});
				--mTabLevel;
			} else if (inObject.getType() == hummus.ePDFObjectStream) {
				output+= 'Stream . iterating stream dictionary:';
				logToFile(output);
				iterateObjectTypes(inObject.toPDFStream().getDictionary(),inReader);
			} else {
				output+= hummus.getTypeLabel(inObject.getType());
				logToFile(output);
			}
		}


		var pdfReader = hummus.createReader(__dirname + '/TestMaterials/XObjectContent.PDF');
		assert.equal(pdfReader.getPDFLevel(), 1.3, 'getPDFLevel');
		assert.equal(pdfReader.getPagesCount(), 2, 'getPagesCount');
		var catalog = pdfReader.queryDictionaryObject(pdfReader.getTrailer(),'Root');
		iterateObjectTypes(catalog,pdfReader);
		fs.closeSync(outputFile);
	});
});

