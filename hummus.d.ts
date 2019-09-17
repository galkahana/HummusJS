declare module 'hummus' {
  export type PosX = number;
  export type PosY = number;
  export type Width = number;
  export type Height = number;

  export type FilePath = string;

  export let PDFPageModifier: PDFPageModifier;
  export let PDFWStreamForFile: PDFWStreamForFile;
  export let PDFRStreamForFile: PDFRStreamForFile;
  export let PDFRStreamForBuffer: PDFRStreamForBuffer;
  export let PDFStreamForResponse: PDFStreamForResponse;

  export function createWriter(
    input: FilePath | WriteStream,
    options?: PDFWriterOptions,
  ): PDFWriter;
  export function createWriterToModify(
    inFile: FilePath,
    options?: PDFWriterToModifyOptions,
  ): PDFWriter;
  export function createWriterToModify(
    inStream: ReadStream,
    outStream: WriteStream,
    options?: PDFWriterToModifyOptions,
  ): PDFWriter;

  export function createWriterToContinue(
    restartFile: string,
    restartStateFile: string,
    options?: PDFWriterToContinueOptions,
  ): PDFWriter;
  export function createReader(input: FilePath | ReadStream, options?: PDFReaderOptions): PDFReader;
  export function recrypt(
    originalPdfPath: FilePath,
    newPdfPath: FilePath,
    options?: PDFRecryptOptions,
  ): void;
  export function recrypt(
    originalPdfStream: any,
    newPdfStream: any,
    options?: PDFRecryptOptions,
  ): void; // TODO stream

  export interface WriteStream {
    write(inBytesArray: any[]): number;
    getCurrentPosition(): number;
  }

  export interface ReadStream {
    read(inAmount: number): number[];
    notEnded(): boolean;
    setPosition(inPosition: number): void;
    setPositionFromEnd(inPosition: number): void;
    skip(inAmount: number): void;
    getCurrentPosition(): number;
  }

  export interface PDFPageInput {
    getDictionary(): PDFDictionary;
    getMediaBox(): PDFBox;
    getCropBox(): PDFBox;
    getTrimBox(): PDFBox;
    getBleedBox(): PDFBox;
    getArtBox(): PDFBox;
    getRotate(): number;
  }

  export interface PDFPageModifier {
    new (
      writer: PDFWriter,
      pageIndex?: number,
      ensureContentEncapsulation?: boolean,
    ): PDFPageModifier;
    startContext(): this;
    getContext(): XObjectContentContext;
    endContext(): this;
    attachURLLinktoCurrentPage(
      inUrl: string,
      left: number,
      bottom: number,
      right: number,
      top: number,
    ): this;
    writePage(): this;
  }

  export interface PDFRStreamForFile extends ReadStream {
    new (inPath: string): PDFRStreamForFile;
    close(inCallback?: () => void): void;
  }

  export interface PDFRStreamForBuffer extends ReadStream {
    new (buffer: Buffer): PDFRStreamForBuffer;
  }

  export interface ColorOptions {
    colorspace?: string;
    color?: string | number;
  }

  export interface GraphicOptions extends ColorOptions {
    type?: 'stroke' | 'fill' | 'clip';
    width?: number;
    close?: boolean;
  }

  export interface AbstractContentContext {
    b(): this;
    B(): this;
    bStar(): this;
    BStar(): this;
    s(): this;
    S(): this;
    f(): this;
    F(): this;
    fStar(): this;
    n(): this;
    m(x: number, y: number): this;
    l(x: number, y: number): this;
    c(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): number;
    v(x2: number, y2: number, x3: number, y3: number): this;
    y(x1: number, y1: number, x3: number, y3: number): this;
    h(): this;
    re(left: number, bottom: number, width: number, height: number): this;
    q(): this;
    Q(): this;
    /**
     * a b 0
     * c d 0
     * e f 1
     */
    cm(a: number, b: number, c: number, d: number, e: number, f: number): this;
    w(lineWidth: number): this;
    J(lineCapStyle: number): this;
    j(lineJoinStyle: number): this;
    M(miterLimit: number): this;
    d(miterLimit: number[], dashPhase: number): this;
    ri(renderingIntentName: string): this;
    i(flatness: number): this;
    gs(graphicStateName: string): this;
    CS(colorSpaceName: string): this;
    cs(colorSpaceName: string): this;
    SC(...colorComponents: number[]): this;
    SCN(...parameters: any[]): this; // This can't be materialized in TypeScript
    ////SCN(...colorComponents: number[], patternName?: string): this;
    sc(...colorComponents: number[]): this;
    scn(...parameters: any[]): this; // This can't be materialized in TypeScript
    ////scn(...colorComponents: number[], patternName?: string): this;
    G(gray: number): this;
    g(gray: number): this;
    RG(r: number, g: number, b: number): this;
    rg(r: number, g: number, b: number): this;
    K(c: number, m: number, y: number, k: number): this;
    k(c: number, m: number, y: number, k: number): this;
    W(): this;
    WStar(): this;
    doXObject(xObject: string | any): this; // TODO
    Tc(characterSpace: number): this;
    Tw(wordSpace: number): this;
    Tz(horizontalScaling: number): this;
    TL(textLeading: number): this;
    Tr(renderingMode: number): this;
    Ts(fontRise: number): this;
    BT(): this;
    ET(): this;
    Td(tX: number, tY: number): this;
    TD(tX: number, tY: number): this;
    Tm(a: number, b: number, c: number, d: number, e: number, f: number): this;
    TStar(): this;
    Tf(fontReferenced: UsedFont | string, fontSize: number): this;
    Tj(text: string | any): this; // Glyph
    Quote(text: string | any): this; // Glyph
    DoubleQuote(wordSpacing: number, characterString: number, text: string | any): this; // Glyph
    TJ(stringsAndSpacing: any): this; // TODO
    writeFreeCode(freeCode: string): this;
    drawPath(...parameters: any[]): this; // This can't be materialized in TypeScript
    ////drawPath(...xyPairs: number[], options: GraphicOptions): this;
    drawCircle(x: number, y: number, r: number, options: GraphicOptions): this;
    drawSquare(x: number, y: number, l: number, options: GraphicOptions): this;
    drawRectangle(x: number, y: number, w: number, h: number, options: GraphicOptions): this;
    writeText(text: string, x: number, y: number, options?: WriteTextOptions): this;
    drawImage(x: number, y: number, imagePath: string, options?: ImageOptions): this;
  }

  export interface TransformationObject {
    width: number;
    height: number;
    proportional?: boolean;
    fit?: 'always' | 'overflow';
  }

  export interface ImageOptions {
    index?: number;
    transformation?: number[] | TransformationObject;
    password?: string;
  }

  export interface FontOptions {
    size?: number;
    font?: UsedFont;
  }

  export interface WriteTextOptions extends FontOptions, ColorOptions {
    underline?: boolean;
  }

  export interface XObjectContentContext extends AbstractContentContext {}

  export interface PDFWStreamForFile extends WriteStream {
    new (inPath: string): PDFWStreamForFile;
    close(inCallback?: () => void): void;
  }

  export interface PDFStreamForResponse extends WriteStream {
    new (res: any): PDFStreamForResponse;
  }

  export interface PDFReaderOptions {
    password: string;
  }

  export interface PDFWriterToModifyOptions extends PDFWriterOptions {
    modifiedFilePath?: string;
  }

  export interface PDFWriterToContinueOptions {
    modifiedFilePath?: string;
    alternativeStream?: any; // TODO
    log?: string;
  }

  export interface PDFRecryptOptions extends PDFWriterOptions {
    password?: string;
  }

  export const ePDFVersion10 = 10;
  export const ePDFVersion11 = 11;
  export const ePDFVersion12 = 12;
  export const ePDFVersion13 = 13;
  export const ePDFVersion14 = 14;
  export const ePDFVersion15 = 15;
  export const ePDFVersion16 = 16;
  export const ePDFVersion17 = 17;
  export type EPDFVersion = 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17;

  export const ePDFObjectBoolean = 0;
  export const ePDFObjectLiteralString = 1;
  export const ePDFObjectHexString = 2;
  export const ePDFObjectNull = 3;
  export const ePDFObjectName = 4;
  export const ePDFObjectInteger = 5;
  export const ePDFObjectReal = 6;
  export const ePDFObjectArray = 7;
  export const ePDFObjectDictionary = 8;
  export const ePDFObjectIndirectObjectReference = 9;
  export const ePDFObjectStream = 10;
  export const ePDFObjectSymbol = 11;
  export type PDFObjectType = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11;

  export const ePDFPageBoxMediaBox = 0;
  export const ePDFPageBoxCropBox = 1;
  export const ePDFPageBoxBleedBox = 2;
  export const ePDFPageBoxTrimBox = 3;
  export const ePDFPageBoxArtBox = 4;
  export type PDFPageBoxType = 0 | 1 | 2 | 3 | 4;

  export interface PDFWriterOptions {
    version?: EPDFVersion;
    log?: string;
    compress?: boolean;

    userPassword?: string;
    ownerPassword?: string;
    userProtectionFlag?: number;
  }

  type FormXObjectId = number;

  export interface FormXObject {
    id: FormXObjectId;
  }

  export interface ResourcesDictionary {
    addFormXObjectMapping(formXObject: FormXObject): any;
    /*
    SET_PROTOTYPE_METHOD(t, "addImageXObjectMapping", AddImageXObjectMapping);
    SET_PROTOTYPE_METHOD(t, "addProcsetResource", AddProcsetResource);
    SET_PROTOTYPE_METHOD(t, "addExtGStateMapping", AddExtGStateMapping);
    SET_PROTOTYPE_METHOD(t, "addFontMapping", AddFontMapping);
    SET_PROTOTYPE_METHOD(t, "addColorSpaceMapping", AddColorSpaceMapping);
    SET_PROTOTYPE_METHOD(t, "addPatternMapping", AddPatternMapping);
    SET_PROTOTYPE_METHOD(t, "addPropertyMapping", AddPropertyMapping);
    SET_PROTOTYPE_METHOD(t, "addXObjectMapping", AddXObjectMapping);
    SET_PROTOTYPE_METHOD(t, "addShadingMapping", AddShadingMapping);
    */
  }

  export type PDFBox = [PosX, PosY, Width, Height];

  export interface PDFPage {
    mediaBox?: PDFBox;
    cropBox?: PDFBox;
    bleedBox?: PDFBox;
    trimBox?: PDFBox;
    artBox?: PDFBox;
    rotate?: number;
    getResourcesDictionary(): ResourcesDictionary;

    /*
    SET_ACCESSOR_METHODS(t, "mediaBox", GetMediaBox, SetMediaBox);
	SET_ACCESSOR_METHODS(t, "cropBox", GetCropBox, SetCropBox);
	SET_ACCESSOR_METHODS(t, "bleedBox", GetBleedBox, SetBleedBox);
	SET_ACCESSOR_METHODS(t, "trimBox", GetTrimBox, SetTrimBox);
	SET_ACCESSOR_METHODS(t, "artBox", GetArtBox, SetArtBox);
    SET_ACCESSOR_METHODS(t, "rotate",GetRotate, SetRotate);
	SET_PROTOTYPE_METHOD(t, "getResourcesDictionary", GetResourcesDictionary);
    */
  }

  export interface TextDimension {
    xMin: number;
    yMin: number;
    xMax: number;
    yMax: number;
    width: number;
    height: number;
  }

  export interface RectangleDimension {
    width: Width;
    height: Height;
  }

  export interface UsedFont {
    calculateTextDimensions(text: string | any, fontSize: number): TextDimension;
  }

  export interface ByteWriter {
    write(buffer: number[]): number;
  }

  export interface ByteReader {
    /*
	SET_PROTOTYPE_METHOD(t, "read", Read);
	SET_PROTOTYPE_METHOD(t, "notEnded", NotEnded);
    */
  }

  export interface ByteReaderWithPosition {
    /*
        SET_PROTOTYPE_METHOD(t, "read", Read);
        SET_PROTOTYPE_METHOD(t, "notEnded", NotEnded);
        SET_PROTOTYPE_METHOD(t, "setPosition", SetPosition);
        SET_PROTOTYPE_METHOD(t, "getCurrentPosition", GetCurrentPosition);
        SET_PROTOTYPE_METHOD(t, "setPositionFromEnd", SetPositionFromEnd);
        SET_PROTOTYPE_METHOD(t, "skip", Skip);
        */
  }

  export interface PDFReader {
    getPDFLevel(): number;
    getPagesCount(): number;
    getTrailer(): PDFDictionary;
    queryDictionaryObject(dictionary: PDFDictionary, name: string): PDFObject;
    /*
  SET_PROTOTYPE_METHOD(t, "queryArrayObject", QueryArrayObject);
  */
    parseNewObject(objectId: number): PDFObject;
    /*
    SET_PROTOTYPE_METHOD(t, "getPageObjectID", GetPageObjectID);
    SET_PROTOTYPE_METHOD(t, "parsePageDictionary", ParsePageDictionary);
    */
    parsePage(page: number): PDFPageInput;
    /*
    SET_PROTOTYPE_METHOD(t, "getObjectsCount", GetObjectsCount);
    SET_PROTOTYPE_METHOD(t, "isEncrypted", IsEncrypted);
    SET_PROTOTYPE_METHOD(t, "getXrefSize", GetXrefSize);
    SET_PROTOTYPE_METHOD(t, "getXrefEntry", GetXrefEntry);
    SET_PROTOTYPE_METHOD(t, "getXrefPosition", GetXrefPosition);
    SET_PROTOTYPE_METHOD(t, "startReadingFromStream", StartReadingFromStream);
    SET_PROTOTYPE_METHOD(t, "getParserStream", GetParserStream);
      */
  }

  export interface PDFStream {
    getWriteStream(): ByteWriter;
  }

  export interface PDFNull extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFName extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFLiteralString extends PDFObject {
    toText(): string;
    value: string;
  }

  export interface PDFInteger extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFIndirectObjectReference extends PDFObject {
    getObjectID(): number;
    getVersion(): number;
  }

  export interface PDFHexString extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFDictionary extends PDFObject {
    toJSObject(): object;
    exists(inName: string): boolean;
    queryObject(inName: string): PDFObject;
  }

  export interface PDFDate {
    /*
        SET_PROTOTYPE_METHOD(t, "toString", ToString);
        SET_PROTOTYPE_METHOD(t, "setToCurrentTime", SetToCurrentTime);
        */
  }

  export interface PDFBoolean extends PDFObject {
    /*
	SET_ACCESSOR_METHOD(t, "value", GetValue);
    */
  }

  export interface PDFArray extends PDFObject {
    /*
	SET_PROTOTYPE_METHOD(t, "toJSArray", ToJSArray);
	SET_PROTOTYPE_METHOD(t, "queryObject", QueryObject);
	SET_PROTOTYPE_METHOD(t, "getLength", GetLength);
    */
  }

  export interface OutputFile {
    /*
	SET_PROTOTYPE_METHOD(t, "openFile", OpenFile);
	SET_PROTOTYPE_METHOD(t, "closeFile", CloseFile);
	SET_PROTOTYPE_METHOD(t, "getFilePath", GetFilePath);
	SET_PROTOTYPE_METHOD(t, "getOutputStream", GetOutputStream);
    */
  }

  export interface InputFile {
    /*
        SET_PROTOTYPE_METHOD(t, "openFile", OpenFile);
        SET_PROTOTYPE_METHOD(t, "closeFile", CloseFile);
        SET_PROTOTYPE_METHOD(t, "getFilePath", GetFilePath);
        SET_PROTOTYPE_METHOD(t, "getFileSize", GetFileSize);
        SET_PROTOTYPE_METHOD(t, "getInputStream", GetInputStream);
    */
  }

  export interface InfoDictionary {
    /*
	SET_PROTOTYPE_METHOD(t, "addAdditionalInfoEntry", AddAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "removeAdditionalInfoEntry", RemoveAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "clearAdditionalInfoEntries", ClearAdditionalInfoEntries);
	SET_PROTOTYPE_METHOD(t, "getAdditionalInfoEntry", GetAdditionalInfoEntry);
	SET_PROTOTYPE_METHOD(t, "getAdditionalInfoEntries", GetAdditionalInfoEntries);
	SET_PROTOTYPE_METHOD(t, "setCreationDate", SetCreationDate);
	SET_PROTOTYPE_METHOD(t, "setModDate", SetModDate);
	SET_ACCESSOR_METHODS(t, "title", GetTitle, SetTitle);
	SET_ACCESSOR_METHODS(t, "author", GetAuthor, SetAuthor);
	SET_ACCESSOR_METHODS(t, "subject", GetSubject, SetSubject);
	SET_ACCESSOR_METHODS(t, "keywords", GetKeywords, SetKeywords);
	SET_ACCESSOR_METHODS(t, "creator", GetCreator, SetCreator);
	SET_ACCESSOR_METHODS(t, "producer", GetProducer, SetProducer);
	SET_ACCESSOR_METHODS(t, "trapped", GetTrapped, SetTrapped);
    */
  }

  export interface ImageXObject {
    /*
        SET_ACCESSOR_METHOD(t, "id", GetID);
        */
  }

  export interface FormObject {
    /*
        SET_ACCESSOR_METHOD(t,"id", GetID);
        SET_PROTOTYPE_METHOD(t, "getContentContext", GetContentContext);
        SET_PROTOTYPE_METHOD(t, "getResourcesDictinary", GetResourcesDictionary);
        SET_PROTOTYPE_METHOD(t, "getContentStream", GetContentStream);
        */
  }

  export interface DocumentCopyingContext {
    createFormXObjectFromPDFPage(
      sourcePageIndex: number,
      ePDFPageBox: PDFPageBoxType | PDFBox,
    ): number;
    mergePDFPageToPage(target: PDFPage, sourcePageIndex: number): void;
    appendPDFPageFromPDF(sourcePageNumber: number): number; // stream start bytes?
    mergePDFPageToFormXObject(sourcePage: PDFPage, targetPageNumber: number): any;
    /*
    SET_PROTOTYPE_METHOD(t, "getSourceDocumentParser", GetSourceDocumentParser);
    SET_PROTOTYPE_METHOD(t, "copyDirectObjectAsIs", CopyDirectObjectAsIs);
    SET_PROTOTYPE_METHOD(t, "copyObject", CopyObject);
    SET_PROTOTYPE_METHOD(t, "copyDirectObjectWithDeepCopy", CopyDirectObjectWithDeepCopy);
    SET_PROTOTYPE_METHOD(t, "copyNewObjectsForDirectObject", CopyNewObjectsForDirectObject);
    SET_PROTOTYPE_METHOD(t, "getCopiedObjectID", GetCopiedObjectID);
    SET_PROTOTYPE_METHOD(t, "getCopiedObjects", GetCopiedObjects);
    SET_PROTOTYPE_METHOD(t, "replaceSourceObjects", ReplaceSourceObjects);
    SET_PROTOTYPE_METHOD(t, "getSourceDocumentStream", GetSourceDocumentStream);
      */
  }

  export interface DocumentContext {
    /*
	SET_PROTOTYPE_METHOD(t, "getInfoDictionary", GetInfoDictionary);
    */
  }

  export interface DictionaryContext {
    /*
        SET_PROTOTYPE_METHOD(t, "writeKey", WriteKey);
        SET_PROTOTYPE_METHOD(t, "writeNameValue", WriteNameValue);
        SET_PROTOTYPE_METHOD(t, "writeRectangleValue", WriteRectangleValue);
        SET_PROTOTYPE_METHOD(t, "writeLiteralStringValue", WriteLiteralStringValue);
        SET_PROTOTYPE_METHOD(t, "writeBooleanValue", WriteBooleanValue);
        SET_PROTOTYPE_METHOD(t, "writeObjectReferenceValue", WriteObjectReferenceValue);
        */
  }

  export interface ByteWriterWithPosition {
    /*
        SET_PROTOTYPE_METHOD(t, "write", Write);
        SET_PROTOTYPE_METHOD(t, "getCurrentPosition", GetCurrentPosition);
        */
  }

  export interface ObjectsContext {
    /*
SET_PROTOTYPE_METHOD(t, "allocateNewObjectID", AllocateNewObjectID);
	SET_PROTOTYPE_METHOD(t, "startDictionary", StartDictionary);
	SET_PROTOTYPE_METHOD(t, "startArray", StartArray);
	SET_PROTOTYPE_METHOD(t, "writeNumber", WriteNumber);
	SET_PROTOTYPE_METHOD(t, "endArray", EndArray);
	SET_PROTOTYPE_METHOD(t, "endLine", EndLine);
	SET_PROTOTYPE_METHOD(t, "endDictionary", EndDictionary);
	SET_PROTOTYPE_METHOD(t, "endIndirectObject", EndIndirectObject);
	SET_PROTOTYPE_METHOD(t, "writeIndirectObjectReference", WriteIndirectObjectReference);
	SET_PROTOTYPE_METHOD(t, "startNewIndirectObject", StartNewIndirectObject);
	SET_PROTOTYPE_METHOD(t, "startModifiedIndirectObject", StartModifiedIndirectObject);
	SET_PROTOTYPE_METHOD(t, "deleteObject", DeleteObject);
	SET_PROTOTYPE_METHOD(t, "writeName", WriteName);
	SET_PROTOTYPE_METHOD(t, "writeLiteralString", WriteLiteralString);
	SET_PROTOTYPE_METHOD(t, "writeHexString", WriteHexString);
	SET_PROTOTYPE_METHOD(t, "writeBoolean", WriteBoolean);
	SET_PROTOTYPE_METHOD(t, "writeKeyword", WriteKeyword);
	SET_PROTOTYPE_METHOD(t, "writeComment", WriteComment);
	SET_PROTOTYPE_METHOD(t, "setCompressStreams", SetCompressStreams);
	SET_PROTOTYPE_METHOD(t, "startPDFStream", StartPDFStream);
	SET_PROTOTYPE_METHOD(t, "startUnfilteredPDFStream", StartUnfilteredPDFStream);
	SET_PROTOTYPE_METHOD(t, "endPDFStream", EndPDFStream);
	SET_PROTOTYPE_METHOD(t, "startFreeContext", StartFreeContext);
	SET_PROTOTYPE_METHOD(t, "endFreeContext", EndFreeContext);
    */
  }

  export interface PDFObject {
    getType(): PDFObjectType;
    toPDFIndirectObjectReference(): PDFIndirectObjectReference;
    toPDFArray(): PDFArray;
    toPDFDictionary(): PDFDictionary;
    toPDFStream(): PDFStream;
    toPDFBoolean(): PDFBoolean;
    toPDFLiteralString(): PDFLiteralString;
    toPDFHexString(): PDFHexString;
    toPDFNull(): PDFNull;
    toPDFName(): PDFName;
    toPDFInteger(): PDFInteger;
    toPDFReal(): PDFReal;
    toPDFSymbol(): PDFSymbol;
    toNumber(): number;
    toString(): string;
  }

  export interface PDFReal extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFSymbol extends PDFObject {
    /*
        SET_ACCESSOR_METHOD(t, "value", GetValue);
        */
  }

  export interface PDFStreamInput extends PDFObject {
    /*
	SET_PROTOTYPE_METHOD(t, "getDictionary", GetDictionary);
	SET_PROTOTYPE_METHOD(t, "getStreamContentStart", GetStreamContentStart);
    */
  }

  export interface PDFTextString {
    /*
    SET_PROTOTYPE_METHOD(t, "toBytesArray", ToBytesArray);
	SET_PROTOTYPE_METHOD(t, "toString", ToString);
	SET_PROTOTYPE_METHOD(t, "fromString", FromString);
    */
  }

  export interface PageContentContext extends AbstractContentContext {
    getCurrentPageContentStream(): PDFStream;
    getAssociatedPage(): PDFPage;
  }

  export interface PDFWriter {
    end(): PDFWriter;
    createPage(x: number, y: number, width: number, height: number): PDFPage;
    createPage(): PDFPage;
    writePage(page: PDFPage): this;
    writePageAndReturnID(page: PDFPage): number;
    startPageContentContext(page: PDFPage): PageContentContext;
    pausePageContentContext(pageContextContext: PageContentContext): this;
    /*
    createFormXObject();
    endFormXObject();
    createFormXObjectFromJPG();
    */
    // TODO: test streamas
    createFormXObjectFromPNG(filePath: FilePath | PDFRStreamForFile): FormXObject;

    getFontForFile(inFontFilePath: FilePath, index?: number): UsedFont;
    getFontForFile(
      inFontFilePath: FilePath,
      inOptionalMetricsFile?: string,
      index?: number,
    ): UsedFont;
    /*
    attachURLLinktoCurrentPage();
    shutdown();
    createFormXObjectFromTIFF();
    createImageXObjectFromJPG();
    retrieveJPGImageInformation();
    getObjectsContext();
    getDocumentContext();
    */
    appendPDFPagesFromPDF(source: FilePath | ReadStream): number[];
    /*
    mergePDFPagesToPage();
    */
    createPDFCopyingContext(source: FilePath | ReadStream): DocumentCopyingContext;
    /*
    createFormXObjectsFromPDF();
    createPDFCopyingContextForModifiedFile();
    createPDFTextString();
    createPDFDate();
    */
    getImageDimensions(inFontFilePath: FilePath | ReadStream): RectangleDimension;
    /*
    getImagePagesCount();
    getImageType();
    getModifiedFileParser();
    getModifiedInputFile();
    getOutputFile();
    registerAnnotationReferenceForNextPageWrite();
    */
  }
}
