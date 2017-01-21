#include "PDFObjectParserDriver.h"
#include "PDFObjectParser.h"
#include "PDFObjectDriver.h"
#include "RefCountPtr.h"
#include "PDFObject.h"

using namespace v8;

Persistent<Function> PDFObjectParserDriver::constructor;
Persistent<FunctionTemplate> PDFObjectParserDriver::constructor_template;

PDFObjectParserDriver::PDFObjectParserDriver()
{
    PDFObjectParserInstance = NULL;
}

void PDFObjectParserDriver::Init()
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFObjectParserDriver"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "parseNewObject", ParseNewObject);
	SET_CONSTRUCTOR(constructor,t);
	SET_CONSTRUCTOR_TEMPLATE(constructor_template,t);
}

METHOD_RETURN_TYPE PDFObjectParserDriver::NewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	
	Local<Object> instance = NEW_INSTANCE(constructor);
	SET_FUNCTION_RETURN_VALUE(instance);
}

v8::Handle<v8::Value> PDFObjectParserDriver::GetNewInstance(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

	Local<Object> instance = NEW_INSTANCE(constructor);
	return CLOSE_SCOPE(instance);
}

bool PDFObjectParserDriver::HasInstance(Handle<Value> inObject)
{
	CREATE_ISOLATE_CONTEXT;
	
	return inObject->IsObject() && HAS_INSTANCE(constructor_template, inObject);
}

METHOD_RETURN_TYPE PDFObjectParserDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFObjectParserDriver* driver = new PDFObjectParserDriver();
    driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This());
}

METHOD_RETURN_TYPE PDFObjectParserDriver::ParseNewObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFObjectParserDriver* self = ObjectWrap::Unwrap<PDFObjectParserDriver>(args.This());
    
    RefCountPtr<PDFObject> newObject = self->PDFObjectParserInstance->ParseNewObject();
    
    if(!newObject)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED);
    else
        SET_FUNCTION_RETURN_VALUE(PDFObjectDriver::CreateDriver(newObject.GetPtr()));
}