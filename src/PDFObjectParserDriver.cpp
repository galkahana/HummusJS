#include "PDFObjectParserDriver.h"
#include "PDFObjectParser.h"
#include "PDFObjectDriver.h"
#include "RefCountPtr.h"
#include "PDFObject.h"
#include "ConstructorsHolder.h"

using namespace v8;



PDFObjectParserDriver::PDFObjectParserDriver()
{
    PDFObjectParserInstance = NULL;
}

PDFObjectParserDriver::~PDFObjectParserDriver() {
	delete PDFObjectParserInstance;
}

DEF_SUBORDINATE_INIT(PDFObjectParserDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE_EXTERNAL(New);

	t->SetClassName(NEW_STRING("PDFObjectParserDriver"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "parseNewObject", ParseNewObject);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFObjectParser_constructor, t);  	
	SET_CONSTRUCTOR_TEMPLATE(holder->PDFObjectParser_constructor_template,t);
}

METHOD_RETURN_TYPE PDFObjectParserDriver::New(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	EXPOSE_EXTERNAL_ARGS(ConstructorsHolder, externalHolder)

    PDFObjectParserDriver* driver = new PDFObjectParserDriver();
    driver->holder = externalHolder;
	driver->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFObjectParserDriver::ParseNewObject(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    PDFObjectParserDriver* self = ObjectWrap::Unwrap<PDFObjectParserDriver>(args.This());
    
    RefCountPtr<PDFObject> newObject = self->PDFObjectParserInstance->ParseNewObject();
    
    if(!newObject)
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    else
        SET_FUNCTION_RETURN_VALUE(self->holder->GetInstanceFor(newObject.GetPtr()))
}