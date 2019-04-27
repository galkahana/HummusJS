/*
 Source File : UsedFontDriver.h
 
 
 Copyright 2013 Gal Kahana HummusJS
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 */
#include "PDFDateDriver.h"
#include "ConstructorsHolder.h"
#include <stdlib.h>

using namespace v8;

DEF_SUBORDINATE_INIT(PDFDateDriver::Init)
{
	CREATE_ISOLATE_CONTEXT;

	Local<FunctionTemplate> t = NEW_FUNCTION_TEMPLATE(New);

	t->SetClassName(NEW_STRING("PDFDate"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	SET_PROTOTYPE_METHOD(t, "toString", ToString);
	SET_PROTOTYPE_METHOD(t, "setToCurrentTime", SetToCurrentTime);
	SET_CONSTRUCTOR_EXPORT("PDFDate", t);

    // save in factory
	EXPOSE_EXTERNAL_FOR_INIT(ConstructorsHolder, holder)
    SET_CONSTRUCTOR(holder->PDFDate_constructor, t);      
}

unsigned int PDFDateDriver::GetUIntValueFromDateFunction(Local<Date> inDate, const char* inFunctionName)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<v8::Value> value = inDate->Get(GET_CURRENT_CONTEXT, NEW_STRING(inFunctionName)).ToLocalChecked();
    Local<Function> func = Local<Function>::Cast(value);
    Local<Value> result;
    
    result = func->Call(GET_CURRENT_CONTEXT, inDate, 0, NULL).ToLocalChecked();
    return TO_UINT32(result)->Value();
}

int PDFDateDriver::GetIntValueFromDateFunction(Local<Date> inDate, const char* inFunctionName)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    Local<v8::Value> value = inDate->Get(GET_CURRENT_CONTEXT, NEW_STRING(inFunctionName)).ToLocalChecked();
    Local<Function> func = Local<Function>::Cast(value);
    Local<Value> result;
    
    result = func->Call(GET_CURRENT_CONTEXT, inDate, 0, NULL).ToLocalChecked();
    return TO_INT32(result)->Value();
}

METHOD_RETURN_TYPE PDFDateDriver::New(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFDateDriver* element = new PDFDateDriver();
    if(args.Length() == 1)
    {
        if(args[0]->IsDate())
        {
            Local<Date> aDate = Local<Date>::Cast(args[0]);
            int timeZoneMinutesDifference = GetIntValueFromDateFunction(aDate,"getTimezoneOffset");
            element->mDate.SetTime(
                        GetUIntValueFromDateFunction(aDate,"getFullYear"),
                        GetUIntValueFromDateFunction(aDate,"getMonth") + 1,
                        GetUIntValueFromDateFunction(aDate,"getDate"),
                        GetUIntValueFromDateFunction(aDate,"getHours"),
                        GetUIntValueFromDateFunction(aDate,"getMinutes"),
                        GetUIntValueFromDateFunction(aDate,"getSeconds"),
                        timeZoneMinutesDifference < 0 ? PDFDate::eLater:PDFDate::eEarlier,
                        (int)(labs(timeZoneMinutesDifference) / 60),
                        (int)((labs(timeZoneMinutesDifference) - (labs(timeZoneMinutesDifference) / 60)*60) / 60));
        }
        else if(args[0]->IsString())
        {
            element->mDate.ParseString(*UTF_8_VALUE(args[0]->TO_STRING()));
        }
    }
    
    element->Wrap(args.This());
	SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE PDFDateDriver::ToString(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFDateDriver* element = ObjectWrap::Unwrap<PDFDateDriver>(args.This());
    
    SET_FUNCTION_RETURN_VALUE(NEW_STRING(element->mDate.ToString().c_str()))
}

METHOD_RETURN_TYPE PDFDateDriver::SetToCurrentTime(const ARGS_TYPE& args)
{
    CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
    
    PDFDateDriver* element = ObjectWrap::Unwrap<PDFDateDriver>(args.This());
    element->mDate.SetToCurrentTime();
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

PDFDate* PDFDateDriver::getInstance()
{
    return &mDate;
}