/*
 Source File : AbstractContentContextDriver.h
 
 
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

#include "AbstractContentContextDriver.h"
#include "AbstractContentContext.h"
#include "ResourcesDictionary.h"
#include "PDFFormXObject.h"
#include "FormXObjectDriver.h"
#include "UsedFontDriver.h"
#include "ImageXObjectDriver.h"
#include "CSSColors.h"
#include "PDFWriterDriver.h"
#include "PDFUsedFont.h"
#include "FreeTypeFaceWrapper.h"

#include <map>
#include <string.h>
#include <ctype.h>
#include <algorithm>

using namespace v8;


typedef std::map<std::string,unsigned long> StringToULongMap;

class ColorMap
{
public:
    ColorMap();
    
    unsigned long GetRGBForColorName(const std::string& inColorName);
    
private:
    
    StringToULongMap mColorMap;
    
};

ColorMap::ColorMap()
{
    unsigned long i;

    for(i=0;strlen(kCSSColorsArray[i].name) != 0; ++i)
        mColorMap.insert(StringToULongMap::value_type(kCSSColorsArray[i].name,kCSSColorsArray[i].rgbValue));
    
}


unsigned long ColorMap::GetRGBForColorName(const std::string& inColorName)
{
    std::string key = inColorName;
    
    // convert to lower case, to match against color names
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    
    StringToULongMap::iterator it = mColorMap.find(key);
    
    if(it == mColorMap.end())
        return 0;
    else
        return it->second;
}


// single instance of color map
static ColorMap sColorMap;

AbstractContentContextDriver::AbstractContentContextDriver()
{
    mResourcesDictionary = NULL;
}

void AbstractContentContextDriver::SetResourcesDictionary(ResourcesDictionary* inResourcesDictionary)
{
    mResourcesDictionary = inResourcesDictionary;
}

void AbstractContentContextDriver::Init(Handle<FunctionTemplate>& ioDriverTemplate)
{
    
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("b"),FunctionTemplate::New(b)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("B"),FunctionTemplate::New(B)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("bStar"),FunctionTemplate::New(bStar)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("BStar"),FunctionTemplate::New(BStar)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("s"),FunctionTemplate::New(s)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("S"),FunctionTemplate::New(S)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("f"),FunctionTemplate::New(f)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("F"),FunctionTemplate::New(F)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("fStar"),FunctionTemplate::New(fStar)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("n"),FunctionTemplate::New(n)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("m"),FunctionTemplate::New(m)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("l"),FunctionTemplate::New(l)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("c"),FunctionTemplate::New(c)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("v"),FunctionTemplate::New(v)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("y"),FunctionTemplate::New(y)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("h"),FunctionTemplate::New(h)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("re"),FunctionTemplate::New(re)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("q"),FunctionTemplate::New(q)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Q"),FunctionTemplate::New(Q)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("cm"),FunctionTemplate::New(cm)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("w"),FunctionTemplate::New(w)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("J"),FunctionTemplate::New(J)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("j"),FunctionTemplate::New(j)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("M"),FunctionTemplate::New(M)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("d"),FunctionTemplate::New(d)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("ri"),FunctionTemplate::New(ri)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("i"),FunctionTemplate::New(i)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("gs"),FunctionTemplate::New(gs)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("CS"),FunctionTemplate::New(CS)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("cs"),FunctionTemplate::New(cs)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("SC"),FunctionTemplate::New(SC)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("SCN"),FunctionTemplate::New(SCN)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("sc"),FunctionTemplate::New(sc)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("scn"),FunctionTemplate::New(scn)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("G"),FunctionTemplate::New(G)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("g"),FunctionTemplate::New(g)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("RG"),FunctionTemplate::New(RG)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("rg"),FunctionTemplate::New(rg)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("K"),FunctionTemplate::New(K)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("k"),FunctionTemplate::New(k)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("W"),FunctionTemplate::New(W)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("WStar"),FunctionTemplate::New(WStar)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("doXObject"),FunctionTemplate::New(doXObject)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tc"),FunctionTemplate::New(Tc)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tw"),FunctionTemplate::New(Tw)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tz"),FunctionTemplate::New(Tz)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("TL"),FunctionTemplate::New(TL)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tr"),FunctionTemplate::New(Tr)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Ts"),FunctionTemplate::New(Ts)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("BT"),FunctionTemplate::New(BT)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("ET"),FunctionTemplate::New(ET)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Td"),FunctionTemplate::New(Td)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("TD"),FunctionTemplate::New(TD)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tm"),FunctionTemplate::New(Tm)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("TStar"),FunctionTemplate::New(TStar)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tf"),FunctionTemplate::New(Tf)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Tj"),FunctionTemplate::New(Tj)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("Quote"),FunctionTemplate::New(Tj)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("DoubleQuote"),FunctionTemplate::New(Tj)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("TJ"),FunctionTemplate::New(Tj)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("writeFreeCode"),FunctionTemplate::New(WriteFreeCode)->GetFunction());
    
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("drawPath"),FunctionTemplate::New(DrawPath)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("drawCircle"),FunctionTemplate::New(DrawCircle)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("drawSquare"),FunctionTemplate::New(DrawSquare)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("drawRectangle"),FunctionTemplate::New(DrawRectangle)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("writeText"),FunctionTemplate::New(WriteText)->GetFunction());
    ioDriverTemplate->PrototypeTemplate()->Set(String::NewSymbol("drawImage"),FunctionTemplate::New(DrawImage)->GetFunction());
}

Handle<Value> AbstractContentContextDriver::b(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->b();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::B(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->B();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::bStar(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->bStar();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::BStar(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->BStar();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::s(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->s();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::S(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->S();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::f(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->f();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::F(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->F();
    return scope.Close(args.This());
}


Handle<Value> AbstractContentContextDriver::fStar(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->fStar();
    return scope.Close(args.This());
}


Handle<Value> AbstractContentContextDriver::n(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->n();
    return scope.Close(args.This());
}


Handle<Value> AbstractContentContextDriver::m(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, movement position")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->m(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::l(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 parameters, line to position")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->l(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::c(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 6 parameters of the curve")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->c(args[0]->ToNumber()->Value(),
                                    args[1]->ToNumber()->Value(),
                                    args[2]->ToNumber()->Value(),
                                    args[3]->ToNumber()->Value(),
                                    args[4]->ToNumber()->Value(),
                                    args[5]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::v(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 4 parameters of the curve")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->v(args[0]->ToNumber()->Value(),
                                    args[1]->ToNumber()->Value(),
                                    args[2]->ToNumber()->Value(),
                                    args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::y(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 4 parameters of the curve")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->y(args[0]->ToNumber()->Value(),
                                    args[1]->ToNumber()->Value(),
                                    args[2]->ToNumber()->Value(),
                                    args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::h(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    contentContext->GetContext()->h();
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::re(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 parameters: 2 bottom left coordinates, and width and height measures")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->re(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value(),
                                     args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::q(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->q();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::Q(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
    else
        contentContext->GetContext()->Q();
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::cm(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 6 arguments forming a 2d transformation matrix")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->cm(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value(),
                                     args[3]->ToNumber()->Value(),
                                     args[4]->ToNumber()->Value(),
                                     args[5]->ToNumber()->Value());
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::w(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, width measure")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->w(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::J(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, line cap style")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->J(args[0]->ToNumber()->Int32Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::j(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, line join style")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->j(args[0]->ToNumber()->Int32Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::M(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, miter limit")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->M(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::d(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsArray() ||
        !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 2 parameters - array for dash pattern and dash phase number")));
		return scope.Close(Undefined());
	}

    int dashArrayLength = args[0]->ToObject()->Get(v8::String::New("length"))->ToNumber()->Int32Value();
    double* dashArray = new double[dashArrayLength];
    for(int i=0; i < dashArrayLength;++i)
        dashArray[i] = args[0]->ToObject()->Get(i)->ToNumber()->Int32Value();
    
    contentContext->GetContext()->d(dashArray,dashArrayLength,args[1]->ToNumber()->Int32Value());
    
    delete[] dashArray;
                             
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::ri(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 string argument, the rendering intent")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->ri(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::i(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, flatness")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->i(args[0]->ToNumber()->Int32Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::gs(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 string argument, graphic state name")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->gs(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::CS(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide a color space name")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->CS(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::cs(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide a color space name")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->cs(*String::Utf8Value(args[0]->ToString()));
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::SC(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() == 0)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide at least one color component")));
		return scope.Close(Undefined());
	}
    
    
    double* components = new double[args.Length()];
    for(int i = 0; i < args.Length(); ++i)
        components[i] = args[i]->ToNumber()->Value();
    
    contentContext->GetContext()->SC(components,args.Length());
    
    delete[] components;
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::SCN(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() == 0 ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide at least one color component")));
		return scope.Close(Undefined());
	}
    
    bool hasPatternArgument = args[args.Length()-1]->IsString();
    int componentsLength = hasPatternArgument ? args.Length() - 1 : args.Length();
    
    double* components = new double[componentsLength];
    for(int i = 0; i < componentsLength; ++i)
        components[i] = args[i]->ToNumber()->Value();
    
    if(hasPatternArgument)
        contentContext->GetContext()->SCN(components,componentsLength,*String::Utf8Value(args[args.Length()-1]->ToString()));
    else
        contentContext->GetContext()->SCN(components,componentsLength);
    
    delete[] components;
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::sc(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() == 0)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide at least one color component")));
		return scope.Close(Undefined());
	}
    
    
    double* components = new double[args.Length()];
    for(int i = 0; i < args.Length(); ++i)
        components[i] = args[i]->ToNumber()->Value();
    
    contentContext->GetContext()->sc(components,args.Length());
    
    delete[] components;
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::scn(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() == 0 ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide at least one color component")));
		return scope.Close(Undefined());
	}
    
    bool hasPatternArgument = args[args.Length()-1]->IsString();
    int componentsLength = hasPatternArgument ? args.Length() - 1 : args.Length();
    
    double* components = new double[componentsLength];
    for(int i = 0; i < componentsLength; ++i)
        components[i] = args[i]->ToNumber()->Value();
    
    if(hasPatternArgument)
        contentContext->GetContext()->scn(components,componentsLength,*String::Utf8Value(args[args.Length()-1]->ToString()));
    else
        contentContext->GetContext()->scn(components,componentsLength);
    
    delete[] components;
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::G(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, gray value (0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->G(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::g(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context using pdfWriter.startPageContentContext(page)")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 1 parameter, gray value (0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->g(args[0]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::RG(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 3 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 3 arguments as rgb color values")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->RG(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value());
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::rg(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 3 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 3 arguments as rgb color values")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->rg(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value());
    return scope.Close(args.This());
}


Handle<Value> AbstractContentContextDriver::K(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 cmyk components (values should be 0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->K(args[0]->ToNumber()->Value(),
                                    args[1]->ToNumber()->Value(),
                                    args[2]->ToNumber()->Value(),
                                    args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::k(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		ThrowException(Exception::TypeError(String::New("Wrong Argument, please provide 4 cmyk components (values should be 0-255)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->k(args[0]->ToNumber()->Value(),
                                    args[1]->ToNumber()->Value(),
                                    args[2]->ToNumber()->Value(),
                                    args[3]->ToNumber()->Value());
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::W(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    contentContext->GetContext()->W();
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::WStar(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    contentContext->GetContext()->WStar();
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::doXObject(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext() || !contentContext->mResourcesDictionary)
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() != 1)
    {
        ThrowException(Exception::TypeError(String::New("Invalid arguments. pass an xobject")));
        return scope.Close(Undefined());
    }
    
    if(args[0]->IsString())
    {
        // string type, form name in local resources dictionary
        Local<String> stringArg = args[0]->ToString();
        String::Utf8Value utf8XObjectName(stringArg);
        
        contentContext->GetContext()->Do(*utf8XObjectName);
    }
    else if(FormXObjectDriver::HasInstance(args[0]))
    {
        // a form object
        FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->ToObject());
        if(!formDriver)
        {
            ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary")));
            return scope.Close(Undefined());
        }
        
        contentContext->GetContext()->Do(contentContext->mResourcesDictionary->AddFormXObjectMapping(formDriver->FormXObject->GetObjectID()));
    }else if(ImageXObjectDriver::HasInstance(args[0]))
    {
        // an image object
        ImageXObjectDriver* imageDriver = ObjectWrap::Unwrap<ImageXObjectDriver>(args[0]->ToObject());
        if(!imageDriver)
        {
            ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary")));
            return scope.Close(Undefined());
        }
        
        contentContext->GetContext()->Do(contentContext->mResourcesDictionary->AddImageXObjectMapping(imageDriver->ImageXObject));
    }
    else
    {
        ThrowException(Exception::TypeError(String::New("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary")));
        return scope.Close(Undefined());
    }
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::Tc(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide character space")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Tc(args[0]->ToNumber()->Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::Tw(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide word space")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Tw(args[0]->ToNumber()->Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::Tz(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide horizontal scaling")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Tz(args[0]->ToNumber()->Int32Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::TL(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide text leading")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->TL(args[0]->ToNumber()->Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::Tr(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide rendering mode")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Tr(args[0]->ToNumber()->Int32Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::Ts(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide font rise")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Ts(args[0]->ToNumber()->Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::BT(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->BT();
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::ET(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
    else
        contentContext->GetContext()->ET();
    return scope.Close(args.This());
}


Handle<Value> AbstractContentContextDriver::Td(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 arguments")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Td(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value());
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::TD(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 2 arguments")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->TD(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value());
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::Tm(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 6 arguments forming a 2d transformation matrix (for text)")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->Tm(args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value(),
                                     args[3]->ToNumber()->Value(),
                                     args[4]->ToNumber()->Value(),
                                     args[5]->ToNumber()->Value());
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::TStar(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    contentContext->GetContext()->TStar();
    return scope.Close(args.This());
}



Handle<Value> AbstractContentContextDriver::Tf(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 2 ||
        (!UsedFontDriver::HasInstance(args[0]) && !args[0]->IsString()) ||
        !args[1]->IsNumber())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide a font object (create with pdfWriter.getFontForFile) or font resource name and a size measure")));
		return scope.Close(Undefined());
	}
    
    if(args[0]->IsString())
        contentContext->GetContext()->TfLow(
                                         *String::Utf8Value(args[0]->ToString()),
                                         args[1]->ToNumber()->Value());
    else
        contentContext->GetContext()->Tf(
                                     ObjectWrap::Unwrap<UsedFontDriver>(args[0]->ToObject())->UsedFont,
                                     args[1]->ToNumber()->Value());
    return scope.Close(args.This());
    
}

Handle<Value> AbstractContentContextDriver::Tj(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if ((args.Length() != 1 && args.Length() != 2 ) ||
        (!args[0]->IsString() && !args[0]->IsArray()) ||
        (args.Length() == 2 && !args[1]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 1 argument, the string that you wish to display or a glyphs IDs array, and an optional options object")));
		return scope.Close(Undefined());
	}
        
    
    if(args[0]->IsArray())
    {
        contentContext->GetContext()->Tj(ArrayToGlyphsList(args[0]));
    }
    else
    {
        TextPlacingOptions options;
        if(args.Length() == 2)
            options = ObjectToOptions(args[1]->ToObject());

        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->TjLow(*String::Utf8Value(args[0]->ToString()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->TjHexLow(*String::Utf8Value(args[0]->ToString()));
                break;
            default:
                contentContext->GetContext()->Tj(*String::Utf8Value(args[0]->ToString()));
        }
    }
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::Quote(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if ((args.Length() != 1 && args.Length() != 2 ) ||
        (!args[0]->IsString() && !args[0]->IsArray()) ||
        (args.Length() == 2 && !args[1]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 1 argument, the string that you wish to display or a glyphs IDs array, and an optional options object")));
		return scope.Close(Undefined());
	}
    
    if(args[0]->IsArray())
    {
        contentContext->GetContext()->Quote(ArrayToGlyphsList(args[0]));
    }
    else
    {
    
        TextPlacingOptions options;
        if(args.Length() == 2)
            options = ObjectToOptions(args[1]->ToObject());
    
        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->QuoteLow(*String::Utf8Value(args[0]->ToString()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->QuoteHexLow(*String::Utf8Value(args[0]->ToString()));
                break;
            default:
                contentContext->GetContext()->Quote(*String::Utf8Value(args[0]->ToString()));
        }
    }
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::DoubleQuote(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if ((args.Length() != 3 && args.Length() != 4)||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        (!args[2]->IsString() && !args[2]->IsArray()) ||
        (args.Length() == 4 && !args[3]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide 3 arguments, word spacing, character spacing and text, and optionally an options object")));
		return scope.Close(Undefined());
	}
    
    if(args[2]->IsArray())
    {
        contentContext->GetContext()->DoubleQuote(args[0]->ToNumber()->Value(),
                                                  args[1]->ToNumber()->Value(),
                                                  ArrayToGlyphsList(args[2]));
    }
    else
    {
    
        TextPlacingOptions options;
        if(args.Length() == 4)
            options = ObjectToOptions(args[3]->ToObject());
        
        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->DoubleQuoteLow(args[0]->ToNumber()->Value(),
                                                          args[1]->ToNumber()->Value(),
                                                          *String::Utf8Value(args[2]->ToString()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->DoubleQuoteHexLow(args[0]->ToNumber()->Value(),
                                                          args[1]->ToNumber()->Value(),
                                                          *String::Utf8Value(args[2]->ToString()));
                break;
            default:
                contentContext->GetContext()->DoubleQuote(args[0]->ToNumber()->Value(),
                                                          args[1]->ToNumber()->Value(),
                                                          *String::Utf8Value(args[2]->ToString()));
        }
    }
    
    
    return scope.Close(args.This());
}

Handle<Value> AbstractContentContextDriver::TJ(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::TypeError(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    // first, scan args to see if has arrays or lists, to realize which form of TJ to run
    bool hasStrings = false;
    
    for(int i=0;i<args.Length()-1 && !hasStrings;++i)
        hasStrings = args[i]->IsString();
    
    if(hasStrings)
    {
        StringOrDoubleList params;

        TextPlacingOptions options;
        
        bool status = true;
        for(int i=0; i < args.Length() - 1 && status; ++i)
        {
            if(args[i]->IsString())
                params.push_back(StringOrDouble(*String::Utf8Value(args[i]->ToString())));
            else if(args[i]->IsNumber())
                params.push_back(StringOrDouble(args[i]->ToNumber()->Value()));
            else if((args.Length() - 1 == i) && args[i]->IsObject())
                options = ObjectToOptions(args[i]->ToObject());
            else
                status = false;
        }
        
        if(!status)
        {
            ThrowException(Exception::TypeError(String::New("Wrong arguments. please provide a variable number of elements each either string/glyphs list or number, and an optional final options object")));
            return scope.Close(Undefined());
        }
        
        
        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->TJLow(params);
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->TJHexLow(params);
                break;
            default:
                contentContext->GetContext()->TJ(params);
        }
    }
    else
    {
        GlyphUnicodeMappingListOrDoubleList params;

        bool status = true;
        for(int i=0; i < args.Length() - 1 && status; ++i)
        {
            if(args[i]->IsArray())
                params.push_back(GlyphUnicodeMappingListOrDouble(ArrayToGlyphsList(args[i])));
            else if(args[i]->IsNumber())
                params.push_back(GlyphUnicodeMappingListOrDouble(args[i]->ToNumber()->Value()));
            else
                status = false;
        }
        
        if(!status)
        {
            ThrowException(Exception::TypeError(String::New("Wrong arguments. please provide a variable number of elements each either string/glyph list or number, and an optional final options object")));
            return scope.Close(Undefined());
        }
        contentContext->GetContext()->TJ(params);
        
    }
    return scope.Close(args.This());
}

TextPlacingOptions AbstractContentContextDriver::ObjectToOptions(const Handle<Object>& inObject)
{
    TextPlacingOptions options;
    
    if(inObject->Has(String::NewSymbol("encoding")))
    {
        std::string value = *String::Utf8Value(inObject->Get(String::NewSymbol("encoding"))->ToString());
        if(value.compare("hex"))
            options.encoding = TextPlacingOptions::EEncodingHex;
        else if(value.compare("code"))
            options.encoding = TextPlacingOptions::EEncodingCode;
    
        // EEncodingText is the default
    }
    
    return options;
}

GlyphUnicodeMappingList AbstractContentContextDriver::ArrayToGlyphsList(const v8::Handle<v8::Value>& inArray)
{
    GlyphUnicodeMappingList glyphList;
    HandleScope handleScope;
    
    int arrayLength =inArray->ToObject()->Get(v8::String::New("length"))->ToNumber()->Int32Value();
    Local<Object> arrayObject = inArray->ToObject();

    for(int i=0; i < arrayLength; ++i)
    {
        if(!arrayObject->Get(i)->IsArray())
            continue;
        
        int itemLength = arrayObject->Get(i)->ToObject()->Get(v8::String::New("length"))->ToNumber()->Int32Value();
        if(0 == itemLength)
            continue;
        
        GlyphUnicodeMapping mapping;
        
        mapping.mGlyphCode = arrayObject->Get(i)->ToObject()->Get(0)->ToNumber()->Uint32Value();
        for(int j=1; j < itemLength;++j)
            mapping.mUnicodeValues.push_back(arrayObject->Get(i)->ToObject()->Get(j)->ToNumber()->Uint32Value());
			
		glyphList.push_back(mapping);
    }
   
    return glyphList;
}

Handle<Value> AbstractContentContextDriver::WriteFreeCode(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsString())
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide string to write")));
		return scope.Close(Undefined());
	}
    
    contentContext->GetContext()->WriteFreeCode(*String::Utf8Value(args[0]->ToString()));
    return scope.Close(args.This());
    
}

/* context.drawPath(x1,y1,x2,y2,x3,y3...{type:stroke, color:#FF00FF, width:3, close:true})
 */
Handle<Value> AbstractContentContextDriver::DrawPath(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 2)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide path coordinates and an optional options object")));
		return scope.Close(Undefined());
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    
    contentContext->GetContext()->m(args[0]->ToNumber()->Value(),args[1]->ToNumber()->Value());
    for(int i=2;i<args.Length()-1;i+=2)
    {
        if(!args[i]->IsNumber()) // options object, stop
            break;
        contentContext->GetContext()->l(args[i]->ToNumber()->Value(),args[i+1]->ToNumber()->Value());
    }
    
    contentContext->FinishPath(args[args.Length() - 1]);
    return scope.Close(args.This());
}

void AbstractContentContextDriver::SetupColorAndLineWidth(const Handle<Value>& inMaybeOptions)
{
    HandleScope scope;
    
    if(!inMaybeOptions->IsObject())
        return;
    
    Handle<Object> options = inMaybeOptions->ToObject();
    
    bool isStroke = !options->Has(String::New("type")) ||
                    strcmp(*String::Utf8Value(options->Get(String::New("type"))),"stroke") == 0;
    SetColor(inMaybeOptions,isStroke);
    
    if(isStroke && options->Has(String::New("width")))
        GetContext()->w(options->Get(String::New("width"))->ToNumber()->Uint32Value());
}

void AbstractContentContextDriver::SetColor(const Handle<Value>& inMaybeOptions,bool inIsStroke)
{
    if(!inMaybeOptions->IsObject())
        return;
    
    Handle<Object> options = inMaybeOptions->ToObject();

    if(options->Has(String::New("color")))
    {
        if(options->Get(String::New("color"))->IsString())
        {
            // string, named color. always RGB (for now)
            SetRGBColor(sColorMap.GetRGBForColorName(*String::Utf8Value(options->Get(String::New("color"))->ToString())),inIsStroke);
        }
        else
        {
            // should be number
            unsigned long colorvalue = (unsigned long)(options->Get(String::New("color"))->ToInteger()->Value());
            std::string colorspace = options->Has(String::New("colorspace")) ?
            *String::Utf8Value(options->Get(String::New("colorspace")->ToString())) :
            "rgb";
            if(colorspace.compare("rgb") == 0)
            {
                SetRGBColor(colorvalue,inIsStroke);
            }
            else if(colorspace.compare("cmyk") == 0)
            {
                double c = (unsigned char)((colorvalue >> 24) & 0xFF);
                double m = (unsigned char)((colorvalue >> 16) & 0xFF);
                double y = (unsigned char)((colorvalue >> 8) & 0xFF);
                double k = (unsigned char)(colorvalue & 0xFF);
                
                if(inIsStroke)
                    GetContext()->K(c/255,m/255,y/255,k/255);
                else
                    GetContext()->k(c/255,m/255,y/255,k/255);
            }
            else if(colorspace.compare("gray") == 0)
            {
                double g = (unsigned char)(colorvalue & 0xFF);
                
                if(inIsStroke)
                    GetContext()->G(g/255);
                else
                    GetContext()->g(g/255);
            }
        }
    }
}

void AbstractContentContextDriver::SetRGBColor(unsigned long inColorValue,bool inIsStroke)
{
    double r = (unsigned char)((inColorValue >> 16) & 0xFF);
    double g = (unsigned char)((inColorValue >> 8) & 0xFF);
    double b = (unsigned char)(inColorValue & 0xFF);
    
    if(inIsStroke)
        GetContext()->RG(r/255,g/255,b/255);
    else
        GetContext()->rg(r/255,g/255,b/255);
}

void AbstractContentContextDriver::FinishPath(const Handle<Value>& inMaybeOptions)
{
    bool closePath = false;
    std::string type = "stroke";
    
    if(inMaybeOptions->IsObject())
    {
    
        Handle<Object> options = inMaybeOptions->ToObject();
    
        if(options->Has(String::New("type")))
            type = *String::Utf8Value(options->Get(String::New("type")));
        
        if(options->Has(String::New("close")))
            closePath = options->Get(String::New("close"))->ToBoolean()->Value();
    }
    
    if(type.compare("stroke") == 0)
    {
        if(closePath)
            GetContext()->s();
        else
            GetContext()->S();
    }
    else if(type.compare("fill") == 0)
    {
        GetContext()->f();
    }
    else if(type.compare("clip"))
    {
        if(closePath)
            GetContext()->h();
        GetContext()->W();
    }
}

/* context.drawCircle(x,y,r,[{type:stroke, color:#FF00FF, width:3, close:true}])
 */

Handle<Value> AbstractContentContextDriver::DrawCircle(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 3)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide x and y coordinates for center, radius and an optional options object")));
		return scope.Close(Undefined());
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    
    const double magic = 0.551784;
    double x = args[0]->ToNumber()->Value();
    double y = args[1]->ToNumber()->Value();
    double r = args[2]->ToNumber()->Value();
    double rmagic = r * magic;
    
    contentContext->GetContext()->m(x-r,y);
    contentContext->GetContext()->c(x-r,y+rmagic,x-rmagic,y+r,x,y+r);
    contentContext->GetContext()->c(x+rmagic,y+r,x+r,y+rmagic,x+r,y);
    contentContext->GetContext()->c(x+r,y-rmagic,x+rmagic,y-r,x,y-r);
    contentContext->GetContext()->c(x-rmagic,y-r,x-r,y-rmagic,x-r,y);
    
/*
 const double magic = 0.551784;
 double xmagic = xrad * magic;
 double ymagic = yrad * magic;
 g.MoveTo(-xrad, 0);
 g.CurveTo(-xrad, ymagic, -xmagic, yrad, 0, yrad);
 g.CurveTo(xmagic, yrad, xrad, ymagic, xrad, 0);
 g.CurveTo(xrad, -ymagic, xmagic, -yrad, 0, -yrad);

 g.CurveTo(-xmagic, -yrad, -xrad, -ymagic, -xrad, 0);
 */
    
    contentContext->FinishPath(args[args.Length() - 1]);
    return scope.Close(args.This());
}

/* context.drawSquare(x,y,l,[{type:stroke, color:#FF00FF, width:3, close:true}])
 */
Handle<Value> AbstractContentContextDriver::DrawSquare(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 3)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide bottom left coordinates, an edge size and optional options object")));
		return scope.Close(Undefined());
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    contentContext->GetContext()->re(
        args[0]->ToNumber()->Value(),
        args[1]->ToNumber()->Value(),
        args[2]->ToNumber()->Value(),
        args[2]->ToNumber()->Value()
    );

    contentContext->FinishPath(args[args.Length() - 1]);
    return scope.Close(args.This());
}

/* context.drawSquare(x,y,w,h,[{type:stroke, color:#FF00FF, width:3, close:true}])
 */
Handle<Value> AbstractContentContextDriver::DrawRectangle(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 4)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide bottom left coordinates, width and height and optional options object")));
		return scope.Close(Undefined());
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    contentContext->GetContext()->re(
                                     args[0]->ToNumber()->Value(),
                                     args[1]->ToNumber()->Value(),
                                     args[2]->ToNumber()->Value(),
                                     args[3]->ToNumber()->Value()
                                     );
    
    contentContext->FinishPath(args[args.Length() - 1]);
    return scope.Close(args.This());
}

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H 


double sGetUnderlineThicknessFactor(FreeTypeFaceWrapper* inFTWrapper)
{
	void* tableInfo = FT_Get_Sfnt_Table(*inFTWrapper,ft_sfnt_post);
	if(tableInfo)
	{
		TT_Postscript* theTable = (TT_Postscript*)tableInfo;
		return theTable->underlineThickness*1.0/(*inFTWrapper)->units_per_EM;
	}
	else
		return 0.05;
}

double sGetUnderlinePositionFactor(FreeTypeFaceWrapper* inFTWrapper)
{
	void* tableInfo = FT_Get_Sfnt_Table(*inFTWrapper,ft_sfnt_post);
	if(tableInfo)
	{
		TT_Postscript* theTable = (TT_Postscript*)tableInfo;
		return theTable->underlinePosition*1.0/(*inFTWrapper)->units_per_EM;
	}
	else
		return -0.15;
}

/* context.writeText(text,x,y,[{font:fontObject, size:fontSize ,color:#FF00FF,underline:boolean}])
 */
Handle<Value> AbstractContentContextDriver::WriteText(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 3)
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide the text and x,y coordinate for text position. optionally also add an options object")));
		return scope.Close(Undefined());
    }
    
    contentContext->GetContext()->BT();
	if(args.Length() >= 4)
	{
		contentContext->SetColor(args[3],false);
		contentContext->SetFont(args[3]);
	}

	std::string text = *String::Utf8Value(args[0]->ToString());
	double xPos = args[1]->ToNumber()->Value();
	double yPos = args[2]->ToNumber()->Value();

    contentContext->GetContext()->Tm(1,0,0,1,xPos,yPos);
    contentContext->GetContext()->Tj(text);
    contentContext->GetContext()->ET();


	// underline
	if(args.Length() >= 4 && args[3]->IsObject())
	{
		Handle<Object> options = args[3]->ToObject();
		if(options->Has(String::New("underline")) && 
				options->Get(String::New("underline"))->ToBoolean()->Value() &&
				UsedFontDriver::HasInstance(options->Get(String::New("font"))))
		{
			// draw underline. use font data for position and thickness
			double fontSize = options->Has(String::New("size")) ? options->Get(String::New("size"))->ToNumber()->Value():1;

			PDFUsedFont* font = ObjectWrap::Unwrap<UsedFontDriver>(options->Get(String::New("font"))->ToObject())->UsedFont;
			FreeTypeFaceWrapper*  ftWrapper = font->GetFreeTypeFont();

			contentContext->SetColor(args[3],true);
			contentContext->GetContext()->w(sGetUnderlineThicknessFactor(ftWrapper)*fontSize);
			double startLine = yPos+sGetUnderlinePositionFactor(ftWrapper)*fontSize;
			contentContext->GetContext()->m(xPos,startLine);
			contentContext->GetContext()->l(xPos + font->CalculateTextAdvance(text,fontSize),startLine);
			contentContext->GetContext()->S();
		}
	}

    return scope.Close(args.This());
}

void AbstractContentContextDriver::SetFont(const v8::Handle<v8::Value>& inMaybeOptions)
{
    if(!inMaybeOptions->IsObject())
        return;
    
    Handle<Object> options = inMaybeOptions->ToObject();
    
    if(options->Has(String::New("font")) &&
       UsedFontDriver::HasInstance(options->Get(String::New("font"))))
        GetContext()->Tf(ObjectWrap::Unwrap<UsedFontDriver>(options->Get(String::New("font"))->ToObject())->UsedFont,
                         options->Has(String::New("size")) ? options->Get(String::New("size"))->ToNumber()->Value():1);
}

Handle<Value> AbstractContentContextDriver::DrawImage(const Arguments& args)
{
    HandleScope scope;
    
    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        ThrowException(Exception::Error(String::New("Null content context. Please create a context")));
        return scope.Close(Undefined());
    }
    
    if(args.Length() < 3 ||
       !args[0]->IsNumber() ||
       !args[1]->IsNumber() ||
       !args[2]->IsString() ||
       (args.Length() >= 4 && !args[3]->IsObject()))
    {
		ThrowException(Exception::TypeError(String::New("Wrong Arguments, please provide bottom left coordinates, an edge size and optional options object")));
		return scope.Close(Undefined());
    }

    /*
     when placing the image, do the following:
     create object id for image file + index [we can reuse], if one does not exist yet. store it in a general dictionary for the pdfwriter (need to get it from the context). register task to write the image to the file, if first
     use object id to register the image
     determine transformation matrix
     if it's just a plain transform (or none), add to position
     if it's fitting, determine the fit (consider proportional), add to position
     gsave, apply determined transformation matrix, place image with do, grestore
     */
    
    unsigned long imageIndex = 0;
    std::string imagePath = *(String::Utf8Value(args[2]->ToString()));
    double transformation[6] = {1,0,0,1,0,0};
    
    if(args.Length() >= 4)
    {
        Handle<Object> optionsObject = args[3]->ToObject();
        
        if(optionsObject->Has(String::New("index")))
            imageIndex = optionsObject->Get(String::New("index"))->ToNumber()->Uint32Value();
        
        if(optionsObject->Has(String::New("transformation")))
        {
            Handle<Value> transformationValue = optionsObject->Get(String::New("transformation"));
            
            if(transformationValue->IsArray() || transformationValue->IsObject())
            {
                Handle<Object> transformationObject = transformationValue->ToObject();
                
                if(transformationValue->IsArray() && transformationObject->Get(String::New("length"))->ToNumber()->Value() == 6)
                {
                    for(int i=0;i<6;++i)
                        transformation[i] = transformationObject->Get(i)->ToNumber()->Value();
                }
                else if(transformationValue->IsObject())
                {
                    // fitting object, determine transformation according to image dimensions relation to width/height
                    double constraintWidth = transformationObject->Get(String::New("width"))->ToNumber()->Value();
                    double constraintheight = transformationObject->Get(String::New("height"))->ToNumber()->Value();
                    bool proportional = transformationObject->Has(String::New("proportional")) ?
                                            transformationObject->Get(String::New("proportional"))->ToBoolean()->Value() :
                                            false;
                    bool fitAlways = transformationObject->Has(String::New("fit")) ?
                                    strcmp("always",*String::Utf8Value(transformationObject->Get(String::New("fit"))->ToString())) == 0:
                                    false;
                    
                    // getting the image dimensions from the pdfwriter to allow optimization on image reads
                    DoubleAndDoublePair imageDimensions = contentContext->GetPDFWriter()->GetImageDimensions(imagePath,imageIndex);
                    
                    double scaleX = 1;
                    double scaleY = 1;
                    
                    if(fitAlways)
                    {
                        scaleX = constraintWidth / imageDimensions.first;
                        scaleY = constraintheight / imageDimensions.second;
                        

                    }
                    else if(imageDimensions.first > constraintWidth || imageDimensions.second > constraintheight) // overflow
                    {
                        scaleX = imageDimensions.first > constraintWidth ? constraintWidth / imageDimensions.first : 1;
                        scaleY = imageDimensions.second > constraintheight ? constraintheight / imageDimensions.second : 1;
                    }
                    
                    if(proportional)
                    {
                        scaleX = std::min(scaleX,scaleY);
                        scaleY = scaleX;
                    }
                    
                    transformation[0] = scaleX;
                    transformation[3] = scaleY;
                }
                
            }
        }
    }
    
    transformation[4]+= args[0]->ToNumber()->Value();
    transformation[5]+= args[1]->ToNumber()->Value();
   
    
    // registering the images at pdfwriter to allow optimization on image writes
    ObjectIDTypeAndBool result = contentContext->GetPDFWriter()->RegisterImageForDrawing(imagePath,imageIndex);
    if(result.second)
    {
        // if first usage, write the image
        contentContext->ScheduleImageWrite(imagePath,imageIndex,result.first);
    }
    
    contentContext->GetContext()->q();
    contentContext->GetContext()->cm(transformation[0],transformation[1],transformation[2],transformation[3],transformation[4],transformation[5]);
    contentContext->GetContext()->Do(contentContext->mResourcesDictionary->AddFormXObjectMapping(result.first));
    contentContext->GetContext()->Q();
    
    return scope.Close(args.This());
}
