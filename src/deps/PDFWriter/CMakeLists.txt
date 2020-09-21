project(PDFWriter)

# Soname
# MAJOR is incremented when symbols are removed or changed in an incompatible way
# MINOR is incremented when new symbols are added
set(PDFWRITER_MAJOR 0)
set(PDFWRITER_MINOR 0)
set(PDFWRITER_LIB_VERSION ${PDFWRITER_MAJOR}.${PDFWRITER_MINOR}.0)
set(PDFWRITER_SO_VERSION ${PDFWRITER_MAJOR})


include_directories (${LIBAESGM_INCLUDE_DIRS})
include_directories (${ZLIB_INCLUDE_DIRS})

if(NOT PDFHUMMUS_NO_DCT)
	include_directories (${LIBJPEG_INCLUDE_DIRS})
else(NOT PDFHUMMUS_NO_DCT)
	add_definitions(-DPDFHUMMUS_NO_DCT=1)
endif(NOT PDFHUMMUS_NO_DCT)

if(NOT PDFHUMMUS_NO_TIFF)
	include_directories (${LIBTIFF_INCLUDE_DIRS})
else(NOT PDFHUMMUS_NO_TIFF)
	add_definitions(-DPDFHUMMUS_NO_TIFF=1)
endif(NOT PDFHUMMUS_NO_TIFF)

include_directories (${FREETYPE_INCLUDE_DIRS})

if(NOT PDFHUMMUS_NO_PNG)
	include_directories (${LIBPNG_INCLUDE_DIRS})
else(NOT PDFHUMMUS_NO_PNG)
	add_definitions(-DPDFHUMMUS_NO_PNG=1)
endif(NOT PDFHUMMUS_NO_PNG)



add_library (PDFWriter_OBJLIB OBJECT
#sources
AbstractContentContext.cpp
AbstractWrittenFont.cpp
ANSIFontWriter.cpp
Ascii7Encoding.cpp
ArrayOfInputStreamsStream.cpp
CatalogInformation.cpp
CFFANSIFontWriter.cpp
CFFDescendentFontWriter.cpp
CFFEmbeddedFontWriter.cpp
CFFFileInput.cpp
CFFPrimitiveReader.cpp
CFFPrimitiveWriter.cpp
CharStringType1Interpreter.cpp
CharStringType1Tracer.cpp
CharStringType2Flattener.cpp
CharStringType2Interpreter.cpp
CharStringType2Tracer.cpp
CIDFontWriter.cpp
CMYKRGBColor.cpp
DecryptionHelper.cpp
DescendentFontWriter.cpp
DictionaryContext.cpp
DocumentContext.cpp
EncryptionHelper.cpp
EncryptionOptions.cpp
FontDescriptorWriter.cpp
FreeTypeFaceWrapper.cpp
FreeTypeOpenTypeWrapper.cpp
FreeTypeType1Wrapper.cpp
FreeTypeWrapper.cpp
GraphicState.cpp
GraphicStateStack.cpp
IndirectObjectsReferenceRegistry.cpp
InfoDictionary.cpp
InputAESDecodeStream.cpp
InputAscii85DecodeStream.cpp
InputAsciiHexDecodeStream.cpp
InputBufferedStream.cpp
InputByteArrayStream.cpp
InputCharStringDecodeStream.cpp
InputDCTDecodeStream.cpp
InputFile.cpp
InputFileStream.cpp
InputFlateDecodeStream.cpp
InputLZWDecodeStream.cpp
InputLimitedStream.cpp
InputRC4XcodeStream.cpp
InputPFBDecodeStream.cpp
InputPredictorPNGOptimumStream.cpp
InputPredictorTIFFSubStream.cpp
InputStreamSkipperStream.cpp
InputStringBufferStream.cpp
InputStringStream.cpp
JPEGImageHandler.cpp
JPEGImageInformation.cpp
JPEGImageParser.cpp
Log.cpp
MD5Generator.cpp
RC4.cpp
ObjectsContext.cpp
OpenTypeFileInput.cpp
OpenTypePrimitiveReader.cpp
OutputAESEncodeStream.cpp
OutputBufferedStream.cpp
OutputFile.cpp
OutputFileStream.cpp
OutputFlateDecodeStream.cpp
OutputFlateEncodeStream.cpp
OutputRC4XcodeStream.cpp
OutputStreamTraits.cpp
OutputStringBufferStream.cpp
PageContentContext.cpp
PageTree.cpp
ParsedPrimitiveHelper.cpp
PDFArray.cpp
PDFBoolean.cpp
PDFCosArray.cpp
PDFCosDict.cpp
PDFDate.cpp
PDFDictionary.cpp
PDFDocEncoding.cpp
PDFDocumentCopyingContext.cpp
PDFDocumentHandler.cpp
PDFFormXObject.cpp
PDFTiledPattern.cpp
TiledPatternContentContext.cpp
PDFHexString.cpp
PDFImageXObject.cpp
PDFIndirectObjectReference.cpp
PDFInteger.cpp
PDFLiteralString.cpp
PDFModifiedPage.cpp
PDFName.cpp
PDFNull.cpp
PDFObject.cpp
PDFObjectParser.cpp
PDFPage.cpp
PDFPageInput.cpp
PDFDictionaryIterator.cpp
PDFArrayIterator.cpp
PDFPageMergingHelper.cpp
PDFParser.cpp
PDFParserTokenizer.cpp
PDFParsingOptions.cpp
PDFReal.cpp
PDFRectangle.cpp
PDFStream.cpp
PDFStreamInput.cpp
PDFSymbol.cpp
PDFTextString.cpp
PDFUsedFont.cpp
PDFWriter.cpp
PFMFileReader.cpp
PNGImageHandler.cpp
PrimitiveObjectsWriter.cpp
PSBool.cpp
RefCountObject.cpp
ResourcesDictionary.cpp
SimpleStringTokenizer.cpp
StandardEncoding.cpp
StateReader.cpp
StateWriter.cpp
TIFFImageHandler.cpp
TiffUsageParameters.cpp
Timer.cpp
TimersRegistry.cpp
Trace.cpp
TrailerInformation.cpp
TrueTypeANSIFontWriter.cpp
TrueTypeDescendentFontWriter.cpp
TrueTypeEmbeddedFontWriter.cpp
TrueTypePrimitiveWriter.cpp
Type1Input.cpp
Type1ToCFFEmbeddedFontWriter.cpp
Type1ToType2Converter.cpp
Type2CharStringWriter.cpp
UnicodeString.cpp
UppercaseSequance.cpp
UsedFontsRepository.cpp
WinAnsiEncoding.cpp
WrittenFontCFF.cpp
WrittenFontTrueType.cpp
XCryptionCommon.cpp
XObjectContentContext.cpp

#headers
AbstractContentContext.h
AbstractWrittenFont.h
AdapterIByteReaderWithPositionToIReadPositionProvider.h
ANSIFontWriter.h
Ascii7Encoding.h
ArrayOfInputStreamsStream.h
BetweenIncluding.h
BoxingBase.h
CatalogInformation.h
CFFANSIFontWriter.h
CFFDescendentFontWriter.h
CFFEmbeddedFontWriter.h
CFFFileInput.h
CFFPrimitiveReader.h
CFFPrimitiveWriter.h
CharStringDefinitions.h
CharStringType1Interpreter.h
CharStringType1Tracer.h
CharStringType2Flattener.h
CharStringType2Interpreter.h
CharStringType2Tracer.h
CIDFontWriter.h
CMYKRGBColor.h
ContainerIterator.h
DecryptionHelper.h
DescendentFontWriter.h
DictionaryContext.h
DictOperand.h
DocumentContext.h
DocumentContextExtenderAdapter.h
EFontStretch.h
EHummusImageType.h
EncryptionHelper.h
EncryptionOptions.h
EPDFVersion.h
EStatusCode.h
ETokenSeparator.h
FontDescriptorWriter.h
FreeTypeFaceWrapper.h
FreeTypeOpenTypeWrapper.h
FreeTypeType1Wrapper.h
FreeTypeWrapper.h
FSType.h
GlyphUnicodeMapping.h
GraphicState.h
GraphicStateStack.h
IANSIFontWriterHelper.h
IByteReader.h
IByteReaderWithPosition.h
IByteWriter.h
IByteWriterWithPosition.h
IContentContextListener.h
IDescendentFontWriter.h
IDocumentContextExtender.h
IFontDescriptorHelper.h
IFormEndWritingTask.h
ITiledPatternEndWritingTask.h
IFreeTypeFaceExtender.h
IndirectObjectsReferenceRegistry.h
InfoDictionary.h
InputAESDecodeStream.h
InputAscii85DecodeStream.h
InputAsciiHexDecodeStream.h
InputBufferedStream.h
InputByteArrayStream.h
InputCharStringDecodeStream.h
InputDCTDecodeStream.h
InputFile.h
InputFileStream.h
InputFlateDecodeStream.h
InputLZWDecodeStream.h
InputLimitedStream.h
InputRC4XcodeStream.h
InputPFBDecodeStream.h
InputPredictorPNGOptimumStream.h
InputPredictorTIFFSubStream.h
InputStreamSkipperStream.h
InputStringBufferStream.h
InputStringStream.h
IOBasicTypes.h
IObjectsContextExtender.h
IPageEndWritingTask.h
IPDFParserExtender.h
IReadPositionProvider.h
IResourceWritingTask.h
IType1InterpreterImplementation.h
IType2InterpreterImplementation.h
IWrittenFont.h
JPEGImageHandler.h
JPEGImageInformation.h
JPEGImageParser.h
Log.h
MapIterator.h
MD5Generator.h
RC4.h
MyStringBuf.h
ObjectsBasicTypes.h
ObjectsContext.h
ObjectsContextExtenderAdapter.h
OpenTypeFileInput.h
OpenTypePrimitiveReader.h
OutputAESEncodeStream.h
OutputBufferedStream.h
OutputFile.h
OutputFileStream.h
OutputFlateDecodeStream.h
OutputFlateEncodeStream.h
OutputRC4XcodeStream.h
OutputStreamTraits.h
OutputStringBufferStream.h
PageContentContext.h
PageTree.h
ParsedPrimitiveHelper.h
PDFArray.h
PDFBoolean.h
PDFCosArray.h
PDFCosDict.h
PDFDate.h
PDFDictionary.h
PDFDocEncoding.h
PDFDocumentCopyingContext.h
PDFDocumentHandler.h
PDFEmbedParameterTypes.h
PDFFormXObject.h
PDFTiledPattern.h
TiledPatternContentContext.h
PDFHexString.h
PDFImageXObject.h
PDFIndirectObjectReference.h
PDFInteger.h
PDFLiteralString.h
PDFModifiedPage.h
PDFName.h
PDFNull.h
PDFObject.h
PDFObjectCast.h
PDFObjectParser.h
PDFPage.h
PDFPageInput.h
PDFDictionaryIterator.h
PDFArrayIterator.h
PDFPageMergingHelper.h
PDFParser.h
PDFParserTokenizer.h
PDFParsingOptions.h
PDFReal.h
PDFRectangle.h
PDFStream.h
PDFStreamInput.h
PDFSymbol.h
PDFTextString.h
PDFUsedFont.h
PDFWriter.h
PFMFileReader.h
PNGImageHandler.h
PrimitiveObjectsWriter.h
ProcsetResourcesConstants.h
PSBool.h
RefCountObject.h
RefCountPtr.h
ResourcesDictionary.h
SafeBufferMacrosDefs.h
SimpleStringTokenizer.h
Singleton.h
SingleValueContainerIterator.h
StandardEncoding.h
StateReader.h
StateWriter.h
TIFFImageHandler.h
TiffUsageParameters.h
Timer.h
TimersRegistry.h
Trace.h
TrailerInformation.h
TrueTypeANSIFontWriter.h
TrueTypeDescendentFontWriter.h
TrueTypeEmbeddedFontWriter.h
TrueTypePrimitiveWriter.h
Type1Input.h
Type1ToCFFEmbeddedFontWriter.h
Type1ToType2Converter.h
Type2CharStringWriter.h
UnicodeString.h
UppercaseSequance.h
UsedFontsRepository.h
WinAnsiEncoding.h
WrittenFontCFF.h
WrittenFontRepresentation.h
WrittenFontTrueType.h
XCryptionCommon.h
XObjectContentContext.h
)


set(PDFWriter_OBJECTS $<TARGET_OBJECTS:PDFWriter_OBJLIB>)
set(PDFWriter_OBJECTS $<TARGET_OBJECTS:PDFWriter_OBJLIB> PARENT_SCOPE)
if(IS_XCODE)
    add_library(PDFWriter ${PDFWriter_OBJECTS} XObjectContentContext.cpp)
else(IS_XCODE)
    add_library(PDFWriter ${PDFWriter_OBJECTS})
endif(IS_XCODE)

target_link_libraries(PDFWriter ${LIBAESGM_LDFLAGS} ${LIBJPEG_LDFLAGS} ${ZLIB_LDFLAGS} ${LIBTIFF_LDFLAGS} ${FREETYPE_LDFLAGS} ${LIBPNG_LDFLAGS})
set_target_properties(PDFWriter PROPERTIES VERSION ${PDFWRITER_LIB_VERSION} SOVERSION ${PDFWRITER_SO_VERSION})

install(TARGETS PDFWriter
    RUNTIME DESTINATION bin COMPONENT libraries
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT libraries
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT libraries
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)

if(CREATE_PDFWRITER_ALL)
    set(ALL_OBJECTS ${PDFWriter_OBJECTS} ${FreeType_OBJECTS} ${LibAesgm_OBJECTS} ${LibJpeg_OBJECTS} ${LibTiff_OBJECTS} ${Zlib_OBJECTS} ${LibPng_OBJECTS})

    if(IS_XCODE)
        add_library(PDFWriterAll ${ALL_OBJECTS} XObjectContentContext.cpp)
    else(IS_XCODE)
        add_library(PDFWriterAll ${ALL_OBJECTS})
    endif(IS_XCODE)
    
    install(TARGETS PDFWriterAll
        RUNTIME DESTINATION bin COMPONENT libraries
        ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT libraries
        LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT libraries
    )
endif(CREATE_PDFWRITER_ALL)

# groups definitions
source_group("Document Context Level" FILES
AbstractContentContext.cpp
AbstractContentContext.h
CatalogInformation.cpp
CatalogInformation.h
DocumentContext.cpp
DocumentContext.h
DocumentContextExtenderAdapter.h
GlyphUnicodeMapping.h
GraphicState.cpp
GraphicState.h
GraphicStateStack.cpp
GraphicStateStack.h
IDocumentContextExtender.h
IFormEndWritingTask.h
ITiledPatternEndWritingTask.h
InfoDictionary.cpp
InfoDictionary.h
IResourceWritingTask.h
PageContentContext.cpp
PageContentContext.h
PageTree.cpp
PageTree.h
PDFFormXObject.cpp
PDFFormXObject.h
PDFTiledPattern.cpp
PDFTiledPattern.h
TiledPatternContentContext.cpp
TiledPatternContentContext.h
PDFImageXObject.cpp
PDFImageXObject.h
PDFModifiedPage.cpp
PDFModifiedPage.h
PDFPage.cpp
PDFPage.h
ProcsetResourcesConstants.h
ResourcesDictionary.cpp
ResourcesDictionary.h
TrailerInformation.cpp
TrailerInformation.h
XObjectContentContext.cpp
XObjectContentContext.h
IContentContextListener.h
IPageEndWritingTask.h
)

source_group(Images\\JPG FILES
JPEGImageHandler.cpp
JPEGImageHandler.h
JPEGImageInformation.cpp
JPEGImageInformation.h
JPEGImageParser.cpp
JPEGImageParser.h
)


source_group(Images\\TIFF FILES
TIFFImageHandler.cpp
TIFFImageHandler.h
TiffUsageParameters.cpp
TiffUsageParameters.h
)

source_group(Images\\PNG FILES
PNGImageHandler.cpp
PNGImageHandler.h
)

source_group(Infrastructure\\Basics FILES
BetweenIncluding.h
CMYKRGBColor.cpp
CMYKRGBColor.h
EHummusImageType.h
EPDFVersion.h
EStatusCode.h
MyStringBuf.h
SafeBufferMacrosDefs.h
)

source_group(Infrastructure\\Encoding FILES
Ascii7Encoding.cpp
Ascii7Encoding.h
PDFDocEncoding.cpp
PDFDocEncoding.h
StandardEncoding.cpp
StandardEncoding.h
UnicodeString.cpp
UnicodeString.h
WinAnsiEncoding.cpp
WinAnsiEncoding.h
)

source_group(Infrastructure\\Encryption FILES
MD5Generator.cpp
MD5Generator.h
RC4.cpp
RC4.h
)

source_group(Infrastructure\\Encryption\\AES FILES
aes_modes.c
aes_ni.c
aescrypt.c
aeskey.c
aestab.c
aes.h
aes_ni.h
aes_via_ace.h
aescpp.h
aesopt.h
aestab.h
brg_endian.h
brg_types.h
)

source_group(Infrastructure\\IO FILES
AdapterIByteReaderWithPositionToIReadPositionProvider.h
IByteReader.h
IByteReaderWithPosition.h
IByteWriter.h
IByteWriterWithPosition.h
InputAESDecodeStream.cpp
InputAESDecodeStream.h
InputAscii85DecodeStream.cpp
InputAscii85DecodeStream.h
InputAsciiHexDecodeStream.cpp
InputAsciiHexDecodeStream.h
InputBufferedStream.cpp
InputBufferedStream.h
InputByteArrayStream.cpp
InputByteArrayStream.h
InputDCTDecodeStream.cpp
InputDCTDecodeStream.h
InputFile.cpp
InputFile.h
InputFileStream.cpp
InputFileStream.h
InputFlateDecodeStream.cpp
InputFlateDecodeStream.h
InputLZWDecodeStream.cpp
InputLZWDecodeStream.h
InputLimitedStream.cpp
InputLimitedStream.h
InputRC4XcodeStream.cpp
InputRC4XcodeStream.h
InputStreamSkipperStream.cpp
InputStreamSkipperStream.h
InputStringBufferStream.cpp
InputStringBufferStream.h
InputStringStream.cpp
InputStringStream.h
IOBasicTypes.h
IReadPositionProvider.h
OutputAESEncodeStream.cpp
OutputAESEncodeStream.h
OutputBufferedStream.cpp
OutputBufferedStream.h
OutputFile.cpp
OutputFile.h
OutputFileStream.cpp
OutputFileStream.h
OutputFlateDecodeStream.cpp
OutputFlateDecodeStream.h
OutputFlateEncodeStream.cpp
OutputFlateEncodeStream.h
OutputRC4XcodeStream.cpp
OutputRC4XcodeStream.h
OutputStreamTraits.cpp
OutputStreamTraits.h
OutputStringBufferStream.cpp
OutputStringBufferStream.h
)

source_group(Infrastructure\\Patterns FILES
BoxingBase.h
ContainerIterator.h
MapIterator.h
RefCountObject.cpp
RefCountObject.h
RefCountPtr.h
Singleton.h
SingleValueContainerIterator.h
)

source_group(Infrastructure\\Timers FILES
Timer.cpp
Timer.h
TimersRegistry.cpp
TimersRegistry.h
)

source_group("Infrastructure\\Trace and Log" FILES
Log.cpp
Log.h
Trace.cpp
Trace.h
)

source_group("Objects Context Level" FILES
DictionaryContext.cpp
DictionaryContext.h
ETokenSeparator.h
IndirectObjectsReferenceRegistry.cpp
IndirectObjectsReferenceRegistry.h
IObjectsContextExtender.h
ObjectsBasicTypes.h
ObjectsContext.cpp
ObjectsContext.h
ObjectsContextExtenderAdapter.h
PDFDate.cpp
PDFDate.h
PDFRectangle.cpp
PDFRectangle.h
PDFStream.cpp
PDFStream.h
PDFTextString.cpp
PDFTextString.h
PrimitiveObjectsWriter.cpp
PrimitiveObjectsWriter.h
UppercaseSequance.cpp
UppercaseSequance.h
)

source_group("PDF Embedding\\Parsed Objects" FILES
PDFArray.cpp
PDFArray.h
PDFBoolean.cpp
PDFBoolean.h
PDFCosArray.cpp
PDFCosArray.h
PDFCosDict.cpp
PDFCosDict.h
PDFDictionary.cpp
PDFDictionary.h
PDFHexString.cpp
PDFHexString.h
PDFIndirectObjectReference.cpp
PDFIndirectObjectReference.h
PDFInteger.cpp
PDFInteger.h
PDFLiteralString.cpp
PDFLiteralString.h
PDFName.cpp
PDFName.h
PDFNull.cpp
PDFNull.h
PDFObject.cpp
PDFObject.h
PDFObjectCast.h
PDFReal.cpp
PDFReal.h
PDFStreamInput.cpp
PDFStreamInput.h
PDFSymbol.cpp
PDFSymbol.h
)

source_group("PDF Embedding\\Parsed objects Helpers" FILES
ParsedPrimitiveHelper.cpp
ParsedPrimitiveHelper.h
PDFPageInput.cpp
PDFPageInput.h
PDFDictionaryIterator.cpp
PDFDictionaryIterator.h
PDFArrayIterator.cpp
PDFArrayIterator.h
)

source_group("PDF Embedding\\PredictionFilters" FILES
InputPredictorPNGOptimumStream.cpp
InputPredictorPNGOptimumStream.h
InputPredictorTIFFSubStream.cpp
InputPredictorTIFFSubStream.h
)

source_group("PDF Embedding" FILES
ArrayOfInputStreamsStream.cpp
ArrayOfInputStreamsStream.h
IPDFParserExtender.h
PDFDocumentCopyingContext.cpp
PDFDocumentCopyingContext.h
PDFDocumentHandler.cpp
PDFDocumentHandler.h
PDFEmbedParameterTypes.h
PDFObjectParser.cpp
PDFObjectParser.h
PDFPageMergingHelper.cpp
PDFPageMergingHelper.h
PDFParser.cpp
PDFParser.h
PDFParserTokenizer.cpp
PDFParserTokenizer.h
PDFParsingOptions.cpp
PDFParsingOptions.h
SimpleStringTokenizer.cpp
SimpleStringTokenizer.h
)

source_group("XCryption" FILES
DecryptionHelper.cpp
DecryptionHelper.h
EncryptionHelper.cpp
EncryptionHelper.h
EncryptionOptions.cpp
EncryptionOptions.h
XCryptionCommon.cpp
XCryptionCommon.h
)

source_group("State Serialization" FILES
StateReader.cpp
StateReader.h
StateWriter.cpp
StateWriter.h
)

source_group(Text\\CFF FILES
CFFEmbeddedFontWriter.cpp
CFFEmbeddedFontWriter.h
CFFFileInput.cpp
CFFFileInput.h
CFFPrimitiveReader.cpp
CFFPrimitiveReader.h
CFFPrimitiveWriter.cpp
CFFPrimitiveWriter.h
CharStringDefinitions.h
CharStringType2Flattener.cpp
CharStringType2Flattener.h
CharStringType2Interpreter.cpp
CharStringType2Interpreter.h
CharStringType2Tracer.cpp
CharStringType2Tracer.h
DictOperand.h
IType2InterpreterImplementation.h
)

source_group(Text\\FreeType FILES
EFontStretch.h
FreeTypeFaceWrapper.cpp
FreeTypeFaceWrapper.h
FreeTypeOpenTypeWrapper.cpp
FreeTypeOpenTypeWrapper.h
FreeTypeType1Wrapper.cpp
FreeTypeType1Wrapper.h
FreeTypeWrapper.cpp
FreeTypeWrapper.h
IFreeTypeFaceExtender.h
PFMFileReader.cpp
PFMFileReader.h
)

source_group(Text\\OpenType FILES
OpenTypeFileInput.cpp
OpenTypeFileInput.h
OpenTypePrimitiveReader.cpp
OpenTypePrimitiveReader.h
)

source_group("Text\\PDF Font Definitions" FILES
AbstractWrittenFont.cpp
AbstractWrittenFont.h
ANSIFontWriter.cpp
ANSIFontWriter.h
CFFANSIFontWriter.cpp
CFFANSIFontWriter.h
CFFDescendentFontWriter.cpp
CFFDescendentFontWriter.h
CIDFontWriter.cpp
CIDFontWriter.h
DescendentFontWriter.cpp
DescendentFontWriter.h
FontDescriptorWriter.cpp
FontDescriptorWriter.h
FSType.h
IANSIFontWriterHelper.h
IDescendentFontWriter.h
IFontDescriptorHelper.h
IWrittenFont.h
TrueTypeANSIFontWriter.cpp
TrueTypeANSIFontWriter.h
TrueTypeDescendentFontWriter.cpp
TrueTypeDescendentFontWriter.h
WrittenFontCFF.cpp
WrittenFontCFF.h
WrittenFontRepresentation.h
WrittenFontTrueType.cpp
WrittenFontTrueType.h
)

source_group(Text\\TrueType FILES
TrueTypeEmbeddedFontWriter.cpp
TrueTypeEmbeddedFontWriter.h
TrueTypePrimitiveWriter.cpp
TrueTypePrimitiveWriter.h
)

source_group(Text\\Type1 FILES
CharStringType1Interpreter.cpp
CharStringType1Interpreter.h
CharStringType1Tracer.cpp
CharStringType1Tracer.h
InputCharStringDecodeStream.cpp
InputCharStringDecodeStream.h
InputPFBDecodeStream.cpp
InputPFBDecodeStream.h
IType1InterpreterImplementation.h
PSBool.cpp
PSBool.h
Type1Input.cpp
Type1Input.h
Type1ToCFFEmbeddedFontWriter.cpp
Type1ToCFFEmbeddedFontWriter.h
Type1ToType2Converter.cpp
Type1ToType2Converter.h
Type2CharStringWriter.cpp
Type2CharStringWriter.h
)

source_group(Text FILES
PDFUsedFont.cpp
PDFUsedFont.h
UsedFontsRepository.cpp
UsedFontsRepository.h
)

source_group(Main FILES
PDFWriter.cpp
PDFWriter.h
)
