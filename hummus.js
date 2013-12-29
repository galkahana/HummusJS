// start with binary objects
module.exports = require('./build/Release/hummus');

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
    PDFPageModifier is a helper class providing a content context for existing pages, when in a file modification scenarios.
    Using PDFPageModifier simplifies the process of adding content to existing pages, making it as simple to do as if it were
    a new Page, with a regular content context.  
*/

function PDFPageModifier(inModifiedFileWriter,inPageIndex)
{
    this.writer = inModifiedFileWriter;
    this.pageIndex = inPageIndex;
    this.contexts = [];
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
    var pageDictionaryObject = cpyCxt.getSourceDocumentParser().parsePage(this.pageIndex).getDictionary().toJSObject();


    // create modified page object
    objCxt.startModifiedIndirectObject(pageObjectID);
    var modifiedPageObject = objCxt.startDictionary();

    // copy all elements of the page to the new page object, but the "Contents" and "Resources" elements
     Object.getOwnPropertyNames(pageDictionaryObject).forEach(function(element,index,array)
                                                        {
                                                            if(element != 'Resources' && element != 'Contents')
                                                            {
                                                                modifiedPageObject.writeKey(element);
                                                                cpyCxt.copyDirectObjectAsIs(pageDictionaryObject[element]);
                                                            }
                                                        });

    // Write new contents entry, joining the existing contents with the new one. take care of various scenarios of the existing Contents
    modifiedPageObject.writeKey('Contents');
    if(!pageDictionaryObject['Contents']) // no contents
    {
        objCxt.writeIndirectObjectReference(newContentObjectID);
    }
    else
    {
        objCxt.startArray();
        if(pageDictionaryObject['Contents'].getType() == module.exports.ePDFObjectArray) // contents stream array
        {
            pageDictionaryObject['Contents'].toPDFArray().toJSArray().forEach(function(inElement)
            {
                objCxt.writeIndirectObjectReference(inElement.toPDFIndirectObjectReference().getObjectID());
            });
        }
        else // single stream
        {
            objCxt.writeIndirectObjectReference(pageDictionaryObject['Contents'].toPDFIndirectObjectReference().getObjectID());
        }

        objCxt.writeIndirectObjectReference(newContentObjectID);
        objCxt.endArray();
    }

    // Write a new resource entry. copy all but the "XObject" entry, which needs to be modified. Just for kicks i'm keeping the original 
    // form (either direct dictionary, or indirect object)
    var resourcesIndirect = null;
    var formResourcesNames = [];
    modifiedPageObject.writeKey('Resources');
    if(!pageDictionaryObject['Resources'])
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
        isResorucesIndirect =  (pageDictionaryObject['Resources'].getType() == module.exports.ePDFObjectIndirectObjectReference);
        if(isResorucesIndirect)
        {
            resourcesIndirect = pageDictionaryObject['Resources'].toPDFIndirectObjectReference().getObjectID();
            modifiedPageObject.writeObjectReferenceValue(resourcesIndirect);
        }
        else
            formResourcesNames = writeModifiedResourcesDict(pageDictionaryObject['Resources'],objCxt,cpyCxt,this.contexts);
    }

    // end page object and writing
    objCxt.endDictionary(modifiedPageObject).
            endIndirectObject();

    // if necessary, create now the resource dictionary
    if(resourcesIndirect)
    {
        objCxt.startModifiedIndirectObject(resourcesIndirect);
        formResourcesNames = writeModifiedResourcesDict(cpyCxt.getSourceDocumentParser().parseNewObject(resourcesIndirect),objCxt,cpyCxt,this.contexts);
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


function writeModifiedResourcesDict(inSourceDirect,inObjCxt,inCpyCxt,inNewXObjects)
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
        var jsDict = sourceObject['XObject'].toPDFDictionary().toJSObject();
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