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
#include <stdlib.h>

using namespace v8;

Persistent<Function> PDFDateDriver::constructor;

void PDFDateDriver::Init(Handle<Object> inExports)
{
    // prepare the page interfrace template
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol("PDFDate"));
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->PrototypeTemplate()->Set(String::NewSymbol("toString"),FunctionTemplate::New(ToString)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("setToCurrentTime"),FunctionTemplate::New(SetToCurrentTime)->GetFunction());
    
    constructor = Persistent<Function>::New(t->GetFunction());
    inExports->Set(String::NewSymbol("PDFDate"),constructor);
}

Handle<Value> PDFDateDriver::NewInstance(const Arguments& args)
{
    HandleScope scope;
    
    if(args.Length() != 1 || !args[0]->IsDate())
    {
		ThrowException(Exception::TypeError(String::New("Wrong arguments. Provide 1 argument which is a date")));
        return scope.Close(Undefined());
    }
    
    const unsigned argc = 1;
    Handle<Value> argv[argc] = {args[0]};
    Local<Object> instance = constructor->NewInstance(argc, argv);
    
    return scope.Close(instance);
}

unsigned int PDFDateDriver::GetUIntValueFromDateFunction(Handle<Date> inDate, const char* inFunctionName)
{
    HandleScope scope;
    
    Handle<v8::Value> value = inDate->Get(String::New(inFunctionName));
    Handle<Function> func = Handle<Function>::Cast(value);
    Handle<Value> result;
    
    result = func->Call(inDate, 0, NULL);
    return result->ToNumber()->Uint32Value();
}

int PDFDateDriver::GetIntValueFromDateFunction(Handle<Date> inDate, const char* inFunctionName)
{
    HandleScope scope;
    
    Handle<v8::Value> value = inDate->Get(String::New(inFunctionName));
    Handle<Function> func = Handle<Function>::Cast(value);
    Handle<Value> result;
    
    result = func->Call(inDate, 0, NULL);
    return result->ToNumber()->Int32Value();
}

Handle<Value> PDFDateDriver::New(const Arguments& args)
{
    HandleScope scope;
    
    PDFDateDriver* element = new PDFDateDriver();
    if(args.Length() == 1 && args[0]->IsDate())
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
    
    element->Wrap(args.This());
    return args.This();
}

Handle<Value> PDFDateDriver::ToString(const Arguments& args)
{
    HandleScope scope;
    
    PDFDateDriver* element = ObjectWrap::Unwrap<PDFDateDriver>(args.This());
    
    return scope.Close(String::New(element->mDate.ToString().c_str()));
}

Handle<Value> PDFDateDriver::SetToCurrentTime(const Arguments& args)
{
    HandleScope scope;
    
    PDFDateDriver* element = ObjectWrap::Unwrap<PDFDateDriver>(args.This());
    element->mDate.SetToCurrentTime();
    
    return scope.Close(args.This());
}

