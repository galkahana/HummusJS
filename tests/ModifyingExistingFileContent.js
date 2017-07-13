var hummus = require('../hummus');

// [look...not gonna write it 100% right...you JS experts can probably write this better]
// PDFComment object
function PDFComment(inText, inCommentator, inPosition, inColor, flag, inReplyTo) {
    this.time = new Date();
    this.text = inText;
    this.commentator = inCommentator;
    this.position = inPosition;
    this.color = inColor;
    this.replyTo = inReplyTo;
    this.objectID = 0;
    this.flag = flag;
}

// PDFCommentWriter object
function PDFCommentWriter(inPDFWriter) {
    this.pdfWriter = inPDFWriter;
}

PDFCommentWriter.prototype.writeCommentTree = function(inComment) {
    this._writeCommentsTree(inComment).first;
}

PDFCommentWriter.prototype._writeCommentsTree = function(inComment) {
    var repliedTo = 0;
    var result;

    // if already written, let go
    if (inComment.objectID != 0) {
        return inComment.objectID;
    }

    // if has referred comment, write it first
    if (inComment.replyTo) {
        repliedTo = this._writeCommentsTree(inComment.replyTo);
    }

    var objectsContext = this.pdfWriter.getObjectsContext();
    result = objectsContext.startNewIndirectObject();
    var dictionaryContext = objectsContext.startDictionary();

    dictionaryContext
        .writeKey('Type')
        .writeNameValue('Annot')
        .writeKey('Subtype')
        .writeNameValue('Text')
        .writeKey('Rect')
        .writeRectangleValue(inComment.position) // when implementing allow also 4 numbers input
        .writeKey('Contents')
        .writeLiteralStringValue(this.pdfWriter.createPDFTextString(inComment.text).toBytesArray())
        .writeKey('C');

    objectsContext.startArray();
    inComment.color.forEach(function(element, index, array) { objectsContext.writeNumber(element / 255); });
    objectsContext.endArray(hummus.eTokenSeparatorEndLine);

    dictionaryContext
        .writeKey('T')
        .writeLiteralStringValue(this.pdfWriter.createPDFTextString(inComment.commentator).toBytesArray())
        .writeKey('M')
        .writeLiteralStringValue(this.pdfWriter.createPDFDate(inComment.time).toString());

    if (inComment.replyTo) {
        dictionaryContext
            .writeKey('IRT')
            .writeObjectReferenceValue(repliedTo)
            .writeKey('RT')
            .writeNameValue('R');
    }

    dictionaryContext
        .writeKey('Open')
        .writeBooleanValue(false)
        .writeKey('Name')
        .writeNameValue('Comment')
        .writeKey('F')
        .writeIntegerValue(getFlagBitNumberByName(inComment.flag));

    objectsContext
        .endDictionary(dictionaryContext)
        .endIndirectObject();

    inComment.objectID = result;
    return result;
}

function getFlagBitNumberByName(name) {
	name = name || '';
    // 12.5.3 Annotation Flags
    switch (name.toLowerCase()) {
        case 'invisible':
            return 1;
        case 'hidden':
            return 2;
        case 'print':
            return 4;
        case 'nozoom':
            return 8;
        case 'norotate':
            return 16;
        case 'noview':
            return 32;
        case 'readonly':
            return 64;
        case 'locked':
            return 128;
        case 'togglenoview':
            return 256;
            // 1.7+
            // case 'lockedcontents':
            //     return 512;
        default:
            return 0;
    }
}

describe('ModifyingExistingFileContent', function() {
    var inPDFWriter;

    before(function() {
        inPDFWriter = hummus.createWriterToModify(__dirname + '/TestMaterials/AddedPage.pdf', { modifiedFilePath: __dirname + '/output/ModifyingExistingFileContent.pdf' });
    });
    describe('page size modification', function() {
        it('should complete without error', function() {
            var copyingContext = inPDFWriter.createPDFCopyingContextForModifiedFile();
            var thirdPageID = copyingContext.getSourceDocumentParser().getPageObjectID(2);
            var thirdPageObject = copyingContext.getSourceDocumentParser().parsePage(2).getDictionary().toJSObject();
            var objectsContext = inPDFWriter.getObjectsContext();

            objectsContext.startModifiedIndirectObject(thirdPageID);
            var modifiedPageObject = inPDFWriter.getObjectsContext().startDictionary();

            Object.getOwnPropertyNames(thirdPageObject).forEach(function(element, index, array) {
                if (element != 'MediaBox') {
                    modifiedPageObject.writeKey(element);
                    copyingContext.copyDirectObjectAsIs(thirdPageObject[element]);
                }
            });
            modifiedPageObject.writeKey('MediaBox');
            objectsContext
                .startArray()
                .writeNumber(0)
                .writeNumber(0)
                .writeNumber(500)
                .writeNumber(500)
                .endArray()
                .endLine()
                .endDictionary(modifiedPageObject)
                .endIndirectObject();
        });
    });
    describe('adding comments', function() {
        it('should complete without error', function() {
            var commentWriter = new PDFCommentWriter(inPDFWriter);
            var aComment = new PDFComment(
                'a very important text',
                'someone', [100, 500, 200, 600], [255, 0, 0], 'locked');
            commentWriter.writeCommentTree(aComment);
            var bComment = new PDFComment(
                'I have nothing to say about this',
                'someone', [100, 100, 200, 200], [255, 0, 0],
                'readonly');
            var cComment = new PDFComment(
                'yeah. me too. it\'s just perfect',
                'Someone else', [150, 150, 250, 250], [0, 255, 0], 'nozoom',
                bComment);
            commentWriter.writeCommentTree(cComment);

            var copyingContext = inPDFWriter.createPDFCopyingContextForModifiedFile();

            var fourthPageID = copyingContext.getSourceDocumentParser().getPageObjectID(3);
            var fourthPageObject = copyingContext.getSourceDocumentParser().parsePage(3).getDictionary().toJSObject();
            var objectsContext = inPDFWriter.getObjectsContext();

            objectsContext.startModifiedIndirectObject(fourthPageID);
            var modifiedPageObject = inPDFWriter.getObjectsContext().startDictionary();

            Object.getOwnPropertyNames(fourthPageObject).forEach(function(element, index, array) {
                if (element != 'Annots') {
                    modifiedPageObject.writeKey(element);
                    copyingContext.copyDirectObjectAsIs(fourthPageObject[element]);
                }
            });

            modifiedPageObject.writeKey('Annots');
            objectsContext.startArray()
            objectsContext.writeIndirectObjectReference(aComment.objectID);
            objectsContext.writeIndirectObjectReference(bComment.objectID);
            objectsContext.writeIndirectObjectReference(cComment.objectID);
            objectsContext
                .endArray()
                .endLine()
                .endDictionary(modifiedPageObject)
                .endIndirectObject();
        });
    });

    after(function() {
        inPDFWriter.end();
    });
});
