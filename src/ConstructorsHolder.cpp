#include "ConstructorsHolder.h"
#include "PDFReaderDriver.h"
#include "DocumentCopyingContextDriver.h"
#include "PDFWriterDriver.h"
#include "OutputFileDriver.h"
#include "ObjectsContextDriver.h"
#include "PDFStreamDriver.h"
#include "FormXObjectDriver.h"
#include "PageContentContextDriver.h"
#include "InputFileDriver.h"
#include "DictionaryContextDriver.h"
#include "DocumentContextDriver.h"
#include "InfoDictionaryDriver.h"
#include "PDFArrayDriver.h"
#include "PDFBooleanDriver.h"
#include "PDFLiteralStringDriver.h"
#include "PDFHexStringDriver.h"
#include "PDFNullDriver.h"
#include "PDFNameDriver.h"
#include "PDFIntegerDriver.h"
#include "PDFRealDriver.h"
#include "PDFDictionaryDriver.h"
#include "PDFIndirectObjectReferenceDriver.h"
#include "PDFStreamInputDriver.h"
#include "PDFSymbolDriver.h"
#include "PDFPageInputDriver.h"
#include "PDFObjectParserDriver.h"
#include "PDFPageModifierDriver.h"
#include "PDFPageDriver.h"
#include "ResourcesDictionaryDriver.h"
#include "XObjectContentContextDriver.h"

using namespace v8;
using namespace node;

DEFINE_EXTERNAL_DE_CON_STRUCTORS(ConstructorsHolder)

// creators
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFWriter(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	NEW_INSTANCE(PDFWriter_constructor, instance);
	PDFWriterDriver* element = node::ObjectWrap::Unwrap<PDFWriterDriver>(instance);
	element->holder = this;
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewByteReader(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	NEW_INSTANCE(ByteReader_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFReader(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	if (args.Length() == 1)
	{
		const unsigned argc = 1;
		Local<Value> argv[argc] = { args[0] };
        NEW_INSTANCE_ARGS(PDFReader_constructor, instance, argc, argv);
        PDFReaderDriver* element = node::ObjectWrap::Unwrap<PDFReaderDriver>(instance);
        element->holder = this;
        return CLOSE_SCOPE(instance);
	}
	else
	{
		NEW_INSTANCE(PDFReader_constructor, instance);
        PDFReaderDriver* element = node::ObjectWrap::Unwrap<PDFReaderDriver>(instance);
        element->holder = this;
        return CLOSE_SCOPE(instance);
	}

}


v8::Local<v8::Value> ConstructorsHolder::GetNewDocumentCopyingContext(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(DocumentCopyingContext_constructor, instance);
    DocumentCopyingContextDriver* element = node::ObjectWrap::Unwrap<DocumentCopyingContextDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}


v8::Local<v8::Value> ConstructorsHolder::GetNewByteWriterWithPosition(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ByteWriterWithPosition_constructor, instance);
	return CLOSE_SCOPE(instance);
}


v8::Local<v8::Value> ConstructorsHolder::GetNewOutputFile(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(OutputFile_constructor, instance);
    OutputFileDriver* element = node::ObjectWrap::Unwrap<OutputFileDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}


v8::Local<v8::Value> ConstructorsHolder::GetNewObjectsContext(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ObjectsContext_constructor, instance);
    ObjectsContextDriver* element = node::ObjectWrap::Unwrap<ObjectsContextDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewByteWriter(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ByteWriter_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFStream(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFStream_constructor, instance);
    PDFStreamDriver* element = node::ObjectWrap::Unwrap<PDFStreamDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewFormXObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(FormXObject_constructor, instance);
    FormXObjectDriver* element = node::ObjectWrap::Unwrap<FormXObjectDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPageContentContext(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PageContentContext_constructor, instance);
    PageContentContextDriver* element = node::ObjectWrap::Unwrap<PageContentContextDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewByteReaderWithPosition(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ByteReaderWithPosition_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewInputFile(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(InputFile_constructor, instance);
    InputFileDriver* element = node::ObjectWrap::Unwrap<InputFileDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewDictionaryContext(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(DictionaryContext_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetInstanceFor(DictionaryContext* inDictionaryContextInstance) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(DictionaryContext_constructor, instance);
	DictionaryContextDriver* element = node::ObjectWrap::Unwrap<DictionaryContextDriver>(instance);
	element->DictionaryContextInstance = inDictionaryContextInstance;
	return CLOSE_SCOPE(instance);    
}	

v8::Local<v8::Value> ConstructorsHolder::GetNewDocumentContext(const ARGS_TYPE& args) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(DocumentContext_constructor, instance);
    DocumentContextDriver* element = node::ObjectWrap::Unwrap<DocumentContextDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewInfoDictionary(const ARGS_TYPE& args){
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(InfoDictionary_constructor, instance);
    InfoDictionaryDriver* element = node::ObjectWrap::Unwrap<InfoDictionaryDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFDate(const ARGS_TYPE& args) {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	const unsigned argc = 1;

	if (args.Length() != 1 || (!args[0]->IsDate() && !args[0]->IsString()))
	{
		THROW_EXCEPTION("Wrong arguments. Provide 1 argument which is a date");
		Local<Value> argv[argc] = { NEW_STRING("") };
		NEW_INSTANCE_ARGS(PDFDate_constructor, instance, argc, argv);

		return CLOSE_SCOPE(instance);
	} else {
		Local<Value> argv[argc] = { args[0] };
		NEW_INSTANCE_ARGS(PDFDate_constructor, instance, argc, argv);

		return CLOSE_SCOPE(instance);
	}	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewImageXObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ImageXObject_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFArray(){
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFArray_constructor, instance);
    PDFArrayDriver* element = node::ObjectWrap::Unwrap<PDFArrayDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}


v8::Local<v8::Value> ConstructorsHolder::GetNewPDFBoolean() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFBoolean_constructor, instance);
    PDFBooleanDriver* element = node::ObjectWrap::Unwrap<PDFBooleanDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFLiteralString() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFLiteralString_constructor, instance);
    PDFLiteralStringDriver* element = node::ObjectWrap::Unwrap<PDFLiteralStringDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFHexString() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFHexString_constructor, instance);
    PDFHexStringDriver* element = node::ObjectWrap::Unwrap<PDFHexStringDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFNull() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFNull_constructor, instance);
    PDFNullDriver* element = node::ObjectWrap::Unwrap<PDFNullDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFName() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFName_constructor, instance);
    PDFNameDriver* element = node::ObjectWrap::Unwrap<PDFNameDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFInteger() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFInteger_constructor, instance);
    PDFIntegerDriver* element = node::ObjectWrap::Unwrap<PDFIntegerDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFReal() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFReal_constructor, instance);
    PDFRealDriver* element = node::ObjectWrap::Unwrap<PDFRealDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFDictionary() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFDictionary_constructor, instance);
    PDFDictionaryDriver* element = node::ObjectWrap::Unwrap<PDFDictionaryDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFIndirectObjectReference() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFIndirectObjectReference_constructor, instance);
    PDFIndirectObjectReferenceDriver* element = node::ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFStreamInput() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFStreamInput_constructor, instance);
    PDFStreamInputDriver* element = node::ObjectWrap::Unwrap<PDFStreamInputDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}
v8::Local<v8::Value> ConstructorsHolder::GetNewPDFSymbol() {
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFSymbol_constructor, instance);
    PDFSymbolDriver* element = node::ObjectWrap::Unwrap<PDFSymbolDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetInstanceFor(PDFObject* inObject)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	ESCAPABLE_HANDLE(Value) newInstance;

    switch(inObject->GetType())
    {
        case PDFObject::ePDFObjectBoolean:
        {
            newInstance = GetNewPDFBoolean();
            ObjectWrap::Unwrap<PDFBooleanDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectLiteralString:
        {
			newInstance = GetNewPDFLiteralString();
            ObjectWrap::Unwrap<PDFLiteralStringDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;            
        }
        case PDFObject::ePDFObjectHexString:
        {
			newInstance = GetNewPDFHexString();
            ObjectWrap::Unwrap<PDFHexStringDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectNull:
        {
			newInstance = GetNewPDFNull();
            ObjectWrap::Unwrap<PDFNullDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectName:
        {
            newInstance = GetNewPDFName();
            ObjectWrap::Unwrap<PDFNameDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectInteger:
        {
            newInstance = GetNewPDFInteger();
            ObjectWrap::Unwrap<PDFIntegerDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectReal:
        {
            newInstance = GetNewPDFReal();
            ObjectWrap::Unwrap<PDFRealDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectArray:
        {
            newInstance = GetNewPDFArray();
            ObjectWrap::Unwrap<PDFArrayDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
            
        }
        case PDFObject::ePDFObjectDictionary:
        {
            newInstance = GetNewPDFDictionary();
            ObjectWrap::Unwrap<PDFDictionaryDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectIndirectObjectReference:
        {
            newInstance = GetNewPDFIndirectObjectReference();
            ObjectWrap::Unwrap<PDFIndirectObjectReferenceDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectStream:
        {
            newInstance = GetNewPDFStreamInput();
            ObjectWrap::Unwrap<PDFStreamInputDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
        }
        case PDFObject::ePDFObjectSymbol:
        {
            newInstance = GetNewPDFSymbol();
            ObjectWrap::Unwrap<PDFSymbolDriver>(newInstance->TO_OBJECT())->TheObject = inObject;
            break;
            
        }
    }
	return CLOSE_SCOPE(newInstance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFObjectParser(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFObjectParser_constructor, instance);
    PDFObjectParserDriver* element = node::ObjectWrap::Unwrap<PDFObjectParserDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFPageInput(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFPageInput_constructor, instance);
    PDFPageInputDriver* element = node::ObjectWrap::Unwrap<PDFPageInputDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewResourcesDictionary(const ARGS_TYPE& args) 
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ResourcesDictionary_constructor, instance);
    ResourcesDictionaryDriver* element = node::ObjectWrap::Unwrap<ResourcesDictionaryDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewUsedFont(const ARGS_TYPE& args) 
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(UsedFont_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewXObjectContentContext(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(XObjectContentContext_constructor, instance);
	XObjectContentContextDriver* driver = ObjectWrap::Unwrap<XObjectContentContextDriver>(instance->TO_OBJECT());
	driver->holder = this;

	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFTextString(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFTextString_constructor, instance);
	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetInstanceFor(PDFPage* inPage) 
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Value> argv[1] = { NEW_BOOLEAN(false) };
	NEW_INSTANCE_ARGS(PDFPage_constructor, instance, 1, argv);

	// this version links to a page and does not own it!

	PDFPageDriver* driver = ObjectWrap::Unwrap<PDFPageDriver>(instance->TO_OBJECT());
	driver->holder = this;
	driver->mPDFPage = inPage;
	driver->mOwnsPage = false;

	return CLOSE_SCOPE(instance);
}

v8::Local<v8::Value> ConstructorsHolder::GetInstanceFor(ResourcesDictionary* inResourcesDictionary)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(ResourcesDictionary_constructor, instance);
    ObjectWrap::Unwrap<ResourcesDictionaryDriver>(instance->TO_OBJECT())->ResourcesDictionaryInstance = inResourcesDictionary;
	return CLOSE_SCOPE(instance);   
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFPageModifier(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	NEW_INSTANCE(PDFPageModifier_constructor, instance);
    PDFPageModifierDriver* element = node::ObjectWrap::Unwrap<PDFPageModifierDriver>(instance);
    element->holder = this;	
	return CLOSE_SCOPE(instance);	
}

v8::Local<v8::Value> ConstructorsHolder::GetNewPDFPage(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Value> argv[1] = { NEW_BOOLEAN(false) };
	NEW_INSTANCE_ARGS(PDFPage_constructor, instance, 1, argv);

	// this version creates an instance of a page, and owns it. this is the one used by javascript

	PDFPageDriver* driver = ObjectWrap::Unwrap<PDFPageDriver>(instance->TO_OBJECT());
	driver->holder = this;
	driver->mPDFPage = new PDFPage();
	driver->mOwnsPage = true;

	if (args.Length() == 4 && args[0]->IsNumber() && args[1]->IsNumber() && args[2]->IsNumber() && args[3]->IsNumber())
	{
		driver->mPDFPage->SetMediaBox(PDFRectangle(TO_NUMBER(args[0])->Value(),
			TO_NUMBER(args[1])->Value(),
			TO_NUMBER(args[2])->Value(),
			TO_NUMBER(args[3])->Value()));
	}

	return CLOSE_SCOPE(instance);
}

bool ConstructorsHolder::IsInstance(v8::Local<v8::Value> inObject, const v8::Persistent<v8::FunctionTemplate>& constructor_template) {
	CREATE_ISOLATE_CONTEXT;

	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

bool ConstructorsHolder::IsByteReaderInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, ByteReader_constructor_template );
}


bool ConstructorsHolder::IsByteReaderWithPositionInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, ByteReaderWithPosition_constructor_template );
}


bool ConstructorsHolder::IsByteWriterInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, ByteWriter_constructor_template );
}


bool ConstructorsHolder::IsByteWriterWithPositionInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, ByteWriterWithPosition_constructor_template );
}


bool ConstructorsHolder::IsDictionaryContextInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, DictionaryContext_constructor_template );
}


bool ConstructorsHolder::IsFormXObjectInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, FormXObject_constructor_template );
}


bool ConstructorsHolder::IsImageXObjectInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, ImageXObject_constructor_template );
}


bool ConstructorsHolder::IsInputFileInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, InputFile_constructor_template );
}


bool ConstructorsHolder::IsOutputFileInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, OutputFile_constructor_template );
}


bool ConstructorsHolder::IsPageContentContextInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PageContentContext_constructor_template );
}


bool ConstructorsHolder::IsPDFArrayInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFArray_constructor_template );
}


bool ConstructorsHolder::IsPDFBooleanInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFBoolean_constructor_template );
}


bool ConstructorsHolder::IsPDFDictionaryInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFDictionary_constructor_template );
}


bool ConstructorsHolder::IsPDFHexStringInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFHexString_constructor_template );
}


bool ConstructorsHolder::IsPDFIndirectObjectReferenceInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFIndirectObjectReference_constructor_template );
}


bool ConstructorsHolder::IsPDFIntegerInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFInteger_constructor_template );
}


bool ConstructorsHolder::IsPDFLiteralStringInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFLiteralString_constructor_template );
}


bool ConstructorsHolder::IsPDFNameInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFName_constructor_template );
}


bool ConstructorsHolder::IsPDFNullInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFNull_constructor_template );
}


bool ConstructorsHolder::IsPDFObjectParserInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFObjectParser_constructor_template );
}


bool ConstructorsHolder::IsPDFPageInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFPage_constructor_template );
}


bool ConstructorsHolder::IsPDFPageModifierInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFPageModifier_constructor_template );
}


bool ConstructorsHolder::IsPDFReaderInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFReader_constructor_template );
}


bool ConstructorsHolder::IsPDFRealInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFReal_constructor_template );
}


bool ConstructorsHolder::IsPDFStreamInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFStream_constructor_template );
}


bool ConstructorsHolder::IsPDFStreamInputInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFStreamInput_constructor_template );
}


bool ConstructorsHolder::IsPDFSymbolInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, PDFSymbol_constructor_template );
}


bool ConstructorsHolder::IsUsedFontInstance(v8::Local<v8::Value> inObject)
{
	return ConstructorsHolder::IsInstance(inObject, UsedFont_constructor_template );
}