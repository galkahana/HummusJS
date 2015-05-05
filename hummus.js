// start with binary objects
module.exports = require('./build/Release/hummus');
var fs = require('fs');

/*
    PDFStreamForResponse is an implementation of a write stream that writes directly to an HTTP response.
    Using this stream frees the user from having to create a PDF file on disk when generating on-demand PDFs
*/

function PDFStreamForResponse(inResponse)
{
    this.response = inResponse;
    this.position = 0;
}

PDFStreamForResponse.prototype.write = function(inBytesArray)
{
    if(inBytesArray.length > 0)
    {
        this.response.write(new Buffer(inBytesArray));
        this.position+=inBytesArray.length;
        return inBytesArray.length;
    }
    else
        return 0;
};


PDFStreamForResponse.prototype.getCurrentPosition = function()
{
    return this.position;
};

module.exports.PDFStreamForResponse = PDFStreamForResponse;


/*
    PDFWStreamForFile is an implementation of a write stream using the supplied file path.
*/

function PDFWStreamForFile(inPath)
{
    this.ws = fs.createWriteStream(inPath);
    this.position = 0;
    this.path = inPath;
}
 
PDFWStreamForFile.prototype.write = function(inBytesArray)
{
    if(inBytesArray.length > 0)
    {
        this.ws.write(new Buffer(inBytesArray));
        this.position+=inBytesArray.length;
        return inBytesArray.length;
    }
    else
        return 0;
};
 
PDFWStreamForFile.prototype.getCurrentPosition = function()
{
    return this.position;
};
 
PDFWStreamForFile.prototype.close = function(inCallback)
{
    if(this.ws)
    {
        var self = this;
 
        this.ws.end(function()
        {
            self.ws = null;
            if(inCallback)
                inCallback();
        })
    }
    else
    {
        if(inCallback)
            inCallback();
    }
};
 
 module.exports.PDFWStreamForFile = PDFWStreamForFile;
 
/*
    PDFRStreamForFile is an implementation of a read stream using the supplied file path.
*/

function PDFRStreamForFile(inPath)
{
    this.rs = fs.openSync(inPath,'r');
    this.path = inPath;
    this.rposition = 0;
    this.fileSize = fs.statSync(inPath)["size"]; 
}
 
PDFRStreamForFile.prototype.read = function(inAmount)
{
    var buffer = new Buffer(inAmount*2);
    var bytesRead = fs.readSync(this.rs, buffer, 0, inAmount,this.rposition);
    var arr = [];
 
    for(var i=0;i<bytesRead;++i)
        arr.push(buffer[i]);
    this.rposition+=bytesRead;
    return arr;
}
 
PDFRStreamForFile.prototype.notEnded = function()
{
    return this.rposition < this.fileSize;
}
 
PDFRStreamForFile.prototype.setPosition = function(inPosition)
{
    this.rposition = inPosition;
}
 
PDFRStreamForFile.prototype.setPositionFromEnd = function(inPosition)
{
    this.rposition = this.fileSize-inPosition;
}
 
PDFRStreamForFile.prototype.skip = function(inAmount)
{
    this.rposition += inAmount;
}
 
PDFRStreamForFile.prototype.getCurrentPosition = function(inAmount)
{
    return this.rposition;
}
 
PDFRStreamForFile.prototype.close = function(inCallback)
{
    fs.close(this.rs,inCallback);
};

module.exports.PDFRStreamForFile = PDFRStreamForFile;
 
/*
    PDFPageModifier is a helper class providing a content context for existing pages, when in a file modification scenarios.
    Using PDFPageModifier simplifies the process of adding content to existing pages, making it as simple to do as if it were
    a new Page, with a regular content context.  
*/

function PDFPageModifier(inModifiedFileWriter,inPageIndex)
{
    this.writer = inModifiedFileWriter;
    if(this.writer.modifiedResourcesDictionary === undefined)
        this.writer.modifiedResourcesDictionary = {}; // dictionary used to store resource dictinaries already modified
    this.pageIndex = inPageIndex;
    this.contexts = [];
    this.annotations = [];
}

PDFPageModifier.prototype.startContext = function()
{
    if(!this.contextForm)
    {
        var pageMediaBox = this.writer.getModifiedFileParser().parsePage(this.pageIndex).getMediaBox();
        this.contextForm = this.writer.createFormXObject(pageMediaBox[0],pageMediaBox[1],pageMediaBox[2],pageMediaBox[3]);
    }
    return this;
};

PDFPageModifier.prototype.getContext = function()
{
    return this.contextForm ? this.contextForm.getContentContext() : null;
};


PDFPageModifier.prototype.endContext = function()
{
    this.writer.endFormXObject(this.contextForm);
    this.contexts.push(this.contextForm);
    this.contextForm = null;
    return this;
};

PDFPageModifier.prototype.attachURLLinktoCurrentPage = function(inURL,l,b,r,t)
{
    // create link annotation object
    var objCxt = this.writer.getObjectsContext();

    var annotationObjectID = objCxt.startNewIndirectObject();
    var annotationDict = objCxt.startDictionary();

    annotationDict.writeKey('Type');
    annotationDict.writeNameValue('Annot');

    annotationDict.writeKey('Subtype');
    annotationDict.writeNameValue('Link');

    annotationDict.writeKey('Rect');
    annotationDict.writeRectangleValue(l,b,r,t);

    annotationDict.writeKey('F');
    objCxt.writeNumber(4);

    annotationDict.writeKey('BS');
    var borderStyleDict = objCxt.startDictionary();
    borderStyleDict.writeKey('W');
    objCxt.writeNumber(0);
    objCxt.endDictionary(borderStyleDict);

    annotationDict.writeKey('A');
    var actionDict = objCxt.startDictionary();

    actionDict.writeKey('Type');
    actionDict.writeNameValue('Action');

    actionDict.writeKey('S');
    actionDict.writeNameValue('URI');

    actionDict.writeKey('URI');
    // encode to ascii 7...in other words drop anything which is not, and write as byte array
    var encoded = [];
    for(var i=0;i<inURL.length;++i)
    {
        if(inURL.charCodeAt(i) <= 127)
            encoded.push(inURL.charCodeAt(i));
    }
    actionDict.writeLiteralStringValue(encoded);

    objCxt.endDictionary(actionDict);
    objCxt.endDictionary(annotationDict);

    objCxt.endIndirectObject();

    this.annotations.push(annotationObjectID);

};

PDFPageModifier.prototype.writePage = function()
{
    // allocate an object ID for the new contents stream (for placing the form)
    // we first create the modified page object, so that we can define a name for the new form xobject
    // that is unique
    var objCxt = this.writer.getObjectsContext();
    var newContentObjectID = objCxt.allocateNewObjectID();

    // create a copying context, so we can copy the page dictionary, and modify its contents + resources dict
    var cpyCxt = this.writer.createPDFCopyingContextForModifiedFile();

    // get last page object, convert to JS object, so it's easier to traverse
    var pageObjectID = cpyCxt.getSourceDocumentParser().getPageObjectID(this.pageIndex);
    var pageDictionaryObject = cpyCxt.getSourceDocumentParser().parsePage(this.pageIndex).getDictionary();
    var pageDictionaryJSObject = pageDictionaryObject.toJSObject();



    // create modified page object
    objCxt.startModifiedIndirectObject(pageObjectID);
    var modifiedPageObject = objCxt.startDictionary();

    // copy all elements of the page to the new page object, but the "Contents" and "Resources" elements, and  "Annots"
     Object.getOwnPropertyNames(pageDictionaryJSObject).forEach(function(element,index,array)
                                                        {
                                                            if(element != 'Resources' && element != 'Contents' && element != 'Annots')
                                                            {
                                                                modifiedPageObject.writeKey(element);
                                                                cpyCxt.copyDirectObjectAsIs(pageDictionaryJSObject[element]);
                                                            }
                                                        });
     // Write new annotations entry, joining existing annotations, and new ones (from links)
     if(pageDictionaryJSObject['Annots'] || this.annotations.length > 0)
     {
        modifiedPageObject.writeKey('Annots');
        objCxt.startArray();

        // write old annots, if any exist
        if(pageDictionaryJSObject['Annots'])
        {
            var annotsObj = cpyCxt.getSourceDocumentParser().queryDictionaryObject(pageDictionaryObject,'Annots');
            annotsObj.toPDFArray().toJSArray().forEach(function(inElement)
            {
                objCxt.writeIndirectObjectReference(inElement.toPDFIndirectObjectReference().getObjectID());
            });
        }

        // write new annots from links
        if(this.annotations.length > 0)
        {
            this.annotations.forEach(function(inElement)
            {
                objCxt.writeIndirectObjectReference(inElement);
            });
        }
        objCxt.endArray();
     }


    // Write new contents entry, joining the existing contents with the new one. take care of various scenarios of the existing Contents
    modifiedPageObject.writeKey('Contents');
    if(!pageDictionaryJSObject['Contents']) // no contents
    {
        objCxt.writeIndirectObjectReference(newContentObjectID);
    }
    else
    {
        objCxt.startArray();
        if(pageDictionaryJSObject['Contents'].getType() == module.exports.ePDFObjectArray) // contents stream array
        {
            pageDictionaryJSObject['Contents'].toPDFArray().toJSArray().forEach(function(inElement)
            {
                objCxt.writeIndirectObjectReference(inElement.toPDFIndirectObjectReference().getObjectID());
            });
        }
        else // single stream
        {
            objCxt.writeIndirectObjectReference(pageDictionaryJSObject['Contents'].toPDFIndirectObjectReference().getObjectID());
        }

        objCxt.writeIndirectObjectReference(newContentObjectID);
        objCxt.endArray();
    }

    // Write a new resource entry. copy all but the "XObject" entry, which needs to be modified. Just for kicks i'm keeping the original 
    // form (either direct dictionary, or indirect object)
    var resourcesIndirect = null;
    var newResourcesIndirect = null;
    var formResourcesNames = [];
    modifiedPageObject.writeKey('Resources');
    if(!pageDictionaryJSObject['Resources'])
    {
        // no existing resource dictionary, so write my own
        var dict = objCxt.startDictionary();
        dict.writeKey('XObject');
        var xobjectDict = objCxt.startDictionary();
        this.contexts.forEach(function(inElement,inIndex)
        {
            var formObjectName = "myForm_" + inIndex;
            xobjectDict.writeKey(formObjectName);
            xobjectDict.writeObjectReferenceValue(inElement.id);
            formResourcesNames.push(formObjectName);

        });
        objCxt.endDictionary(xobjectDict).
               endDictionary(dict);
    }
    else
    {
        // resources may be direct, or indirect. if direct, write as is, adding the new form xobject, otherwise wait till page object ends and write then
        isResorucesIndirect =  (pageDictionaryJSObject['Resources'].getType() == module.exports.ePDFObjectIndirectObjectReference);
        if(isResorucesIndirect)
        {
            resourcesIndirect = pageDictionaryJSObject['Resources'].toPDFIndirectObjectReference().getObjectID();
            if(this.writer.modifiedResourcesDictionary[resourcesIndirect])
            {
                // already modified resources dictionary. this means that
                // the resources dictionary is shared. the simplest solution for this is to 
                // create a new resources dictionary
                newResourcesIndirect = objCxt.allocateNewObjectID();
                modifiedPageObject.writeObjectReferenceValue(newResourcesIndirect);
            }
            else
            {
                this.writer.modifiedResourcesDictionary[resourcesIndirect] = true; // register resource dict as modified for a possible next time
                modifiedPageObject.writeObjectReferenceValue(resourcesIndirect);
            }

        }
        else
            formResourcesNames = writeModifiedResourcesDict(cpyCxt.getSourceDocumentParser(),pageDictionaryJSObject['Resources'],objCxt,cpyCxt,this.contexts);
    }

    // end page object and writing
    objCxt.endDictionary(modifiedPageObject).
            endIndirectObject();

    // if necessary [resource dictionary existed and was indirect], create now the resource dictionary
    if(resourcesIndirect)
    {
        if(newResourcesIndirect) // if already written modified resources dict in the past, create a new one with the added form to differ from the previous
            objCxt.startNewIndirectObject(newResourcesIndirect);
        else
            objCxt.startModifiedIndirectObject(resourcesIndirect);
        formResourcesNames = writeModifiedResourcesDict(cpyCxt.getSourceDocumentParser(),cpyCxt.getSourceDocumentParser().parseNewObject(resourcesIndirect),objCxt,cpyCxt,this.contexts);
        objCxt.endIndirectObject();
    }


    // last but not least, create the actual content stream object, placing the form
    objCxt.startNewIndirectObject(newContentObjectID);
    var streamCxt = objCxt.startUnfilteredPDFStream();

    formResourcesNames.forEach(function(inElement)
    {
        objCxt.writeKeyword('q')
                .writeNumber(1)
                .writeNumber(0)
                .writeNumber(0)
                .writeNumber(1)
                .writeNumber(0)
                .writeNumber(0)
                .writeKeyword('cm')
                .writeName(inElement)
                .writeKeyword('Do')
                .writeKeyword('Q')
    });
    objCxt.endPDFStream(streamCxt)
          .endIndirectObject(); 

    return this;
};


function writeModifiedResourcesDict(inParser,inSourceDirect,inObjCxt,inCpyCxt,inNewXObjects)
{
    var formResourcesNames = [];
    var sourceObject = inSourceDirect.toPDFDictionary().toJSObject();
    var dict = inObjCxt.startDictionary();
    Object.getOwnPropertyNames(sourceObject).forEach(function(element,index,array)
                                                    {
                                                        if(element != 'XObject')
                                                        {
                                                            dict.writeKey(element);
                                                            inCpyCxt.copyDirectObjectAsIs(sourceObject[element]);
                                                        }
                                                    });

    // now write a new xobject entry.
    dict.writeKey('XObject');
    var xobjectDict = inObjCxt.startDictionary();
    var imageObjectName;

    if(sourceObject['XObject']) // original exists, copy its keys
    {
        // i'm having a very sophisticated algo here to create a new unique name. 
        // i'm making sure it's different in one letter from any name, using a well known discrete math proof method
        imageObjectName = '';
        // getting the dict via inParser, cause it could be an indirect reference, or direct object and i don't want to know
        var jsDict = inParser.queryDictionaryObject(inSourceDirect.toPDFDictionary(),'XObject').toPDFDictionary().toJSObject();
        
        Object.getOwnPropertyNames(jsDict).forEach(function(element,index,array)
                                                    {
                                                            xobjectDict.writeKey(element);
                                                            inCpyCxt.copyDirectObjectAsIs(jsDict[element]);
                                                            imageObjectName+=String.fromCharCode(
                                                                    getDifferentChar(element.length >= index+1 ? element.charCodeAt(index) : 0x39));
                                                    });     
        inObjCxt.endLine();
    }
    else
        imageObjectName = 'fm';


    inNewXObjects.forEach(function(inElement,inIndex)
    {
        var formObjectName = imageObjectName + '_' + inIndex;
        xobjectDict.writeKey(formObjectName);
        xobjectDict.writeObjectReferenceValue(inElement.id);
        formResourcesNames.push(formObjectName);

    });

    inObjCxt.endDictionary(xobjectDict)
            .endDictionary(dict);

    return formResourcesNames;
}

function getDifferentChar(inCharCode)
{
    // numerals
    if(inCharCode >= 0x30 && inCharCode <= 0x38)
        return inCharCode+1;
    if(inCharCode == 0x39)
        return 0x30;

    // lowercase
    if(inCharCode >= 0x61 && inCharCode <= 0x79)
        return inCharCode+1;
    if(inCharCode == 0x7a)
        return 0x61;

    // uppercase
    if(inCharCode >= 0x41 && inCharCode <= 0x59)
        return inCharCode+1;
    if(inCharCode == 0x5a)
        return 0x41;

    return 0x41;
}

module.exports.PDFPageModifier = PDFPageModifier;