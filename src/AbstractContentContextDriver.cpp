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
#include "ConstructorsHolder.h"

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

void AbstractContentContextDriver::Init(Local<FunctionTemplate>& ioDriverTemplate)
{
	SET_PROTOTYPE_METHOD(ioDriverTemplate,"b",b);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "B", B);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "bStar", bStar);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "BStar", BStar);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "s", s);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "S", S);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "f", f);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "F", F);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "fStar", fStar);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "n", n);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "m", m);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "l", l);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "c", c);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "v", v);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "y", y);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "h", h);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "re", re);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "q", q);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Q", Q);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "cm", cm);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "w", w);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "J", J);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "j", j);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "M", M);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "d", d);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "ri", ri);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "i", i);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "gs", gs);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "CS", CS);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "cs", cs);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "SC", SC);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "SCN", SCN);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "sc", sc);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "scn", scn);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "G", G);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "g", g);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "RG", RG);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "rg", rg);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "K", K);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "k", k);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "W", W);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "WStar", WStar);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "doXObject", doXObject);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tc", Tc);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tw", Tw);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tz", Tz);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "TL", TL);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tr", Tr);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Ts", Ts);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "BT", BT);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "ET", ET);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Td", Td);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "TD", TD);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tm", Tm);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "TStar", TStar);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tf", Tf);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Tj", Tj);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "Quote", Quote);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "DoubleQuote", DoubleQuote);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "TJ", TJ);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "writeFreeCode", WriteFreeCode);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "drawPath", DrawPath);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "drawCircle", DrawCircle);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "drawSquare", DrawSquare);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "drawRectangle", DrawRectangle);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "writeText", WriteText);
	SET_PROTOTYPE_METHOD(ioDriverTemplate, "drawImage", DrawImage);
}

METHOD_RETURN_TYPE AbstractContentContextDriver::b(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->b();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::B(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->B();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::bStar(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->bStar();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::BStar(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->BStar();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::s(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
	if (!contentContext->GetContext())
		THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->s();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::S(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->S();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::f(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->f();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::F(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->F();
    SET_FUNCTION_RETURN_VALUE(args.This())
}


METHOD_RETURN_TYPE AbstractContentContextDriver::fStar(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->fStar();
    SET_FUNCTION_RETURN_VALUE(args.This())
}


METHOD_RETURN_TYPE AbstractContentContextDriver::n(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->n();
    SET_FUNCTION_RETURN_VALUE(args.This())
}


METHOD_RETURN_TYPE AbstractContentContextDriver::m(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 2 parameters, movement position");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->m(TO_NUMBER(args[0])->Value(),TO_NUMBER(args[1])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::l(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 2 parameters, line to position");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->l(TO_NUMBER(args[0])->Value(),TO_NUMBER(args[1])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::c(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 6 parameters of the curve");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->c(TO_NUMBER(args[0])->Value(),
                                    TO_NUMBER(args[1])->Value(),
                                    TO_NUMBER(args[2])->Value(),
                                    TO_NUMBER(args[3])->Value(),
                                    TO_NUMBER(args[4])->Value(),
                                    TO_NUMBER(args[5])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::v(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 4 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 4 parameters of the curve");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->v(TO_NUMBER(args[0])->Value(),
                                    TO_NUMBER(args[1])->Value(),
                                    TO_NUMBER(args[2])->Value(),
                                    TO_NUMBER(args[3])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::y(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 4 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 4 parameters of the curve");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->y(TO_NUMBER(args[0])->Value(),
                                    TO_NUMBER(args[1])->Value(),
                                    TO_NUMBER(args[2])->Value(),
                                    TO_NUMBER(args[3])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::h(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->GetContext()->h();
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::re(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		THROW_EXCEPTION("Wrong Argument, please provide 4 parameters: 2 bottom left coordinates, and width and height measures");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->re(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value(),
                                     TO_NUMBER(args[3])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::q(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
    else
        contentContext->GetContext()->q();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Q(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
    else
        contentContext->GetContext()->Q();
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::cm(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 6 arguments forming a 2d transformation matrix");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->cm(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value(),
                                     TO_NUMBER(args[3])->Value(),
                                     TO_NUMBER(args[4])->Value(),
                                     TO_NUMBER(args[5])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::w(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, width measure");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->w(TO_NUMBER(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::J(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, line cap style");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->J(TO_INT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::j(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, line join style");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->j(TO_INT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::M(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, miter limit");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->M(TO_NUMBER(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::d(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsArray() ||
        !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 2 parameters - array for dash pattern and dash phase number");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}

    int dashArrayLength = TO_INT32(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked())->Value();
    double* dashArray = new double[dashArrayLength];
    for(int i=0; i < dashArrayLength;++i)
        dashArray[i] = TO_INT32(args[0]->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked())->Value();
    
    contentContext->GetContext()->d(dashArray,dashArrayLength,TO_INT32(args[1])->Value());
    
    delete[] dashArray;
                             
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::ri(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsString()) {
		THROW_EXCEPTION("Wrong Argument, please provide 1 string argument, the rendering intent");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->ri(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::i(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, flatness");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->i(TO_INT32(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::gs(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsString()) {
		THROW_EXCEPTION("Wrong Argument, please provide 1 string argument, graphic state name");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->gs(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::CS(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("Wrong Argument, please provide a color space name");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->CS(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::cs(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsString())
    {
		THROW_EXCEPTION("Wrong Argument, please provide a color space name");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->cs(*UTF_8_VALUE(args[0]->TO_STRING()));
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::SC(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() == 0)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide at least one color component");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    
    double* components = new double[args.Length()];
    for(int i = 0; i < args.Length(); ++i)
        components[i] = TO_NUMBER(args[i])->Value();
    
    contentContext->GetContext()->SC(components,args.Length());
    
    delete[] components;
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::SCN(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() == 0 ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide at least one color component");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    bool hasPatternArgument = args[args.Length()-1]->IsString();
    int componentsLength = hasPatternArgument ? args.Length() - 1 : args.Length();
    
    double* components = new double[componentsLength];
    for(int i = 0; i < componentsLength; ++i)
        components[i] = TO_NUMBER(args[i])->Value();
    
    if(hasPatternArgument)
        contentContext->GetContext()->SCN(components,componentsLength,*UTF_8_VALUE(args[args.Length()-1]->TO_STRING()));
    else
        contentContext->GetContext()->SCN(components,componentsLength);
    
    delete[] components;
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::sc(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() == 0)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide at least one color component");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    
    double* components = new double[args.Length()];
    for(int i = 0; i < args.Length(); ++i)
        components[i] = TO_NUMBER(args[i])->Value();
    
    contentContext->GetContext()->sc(components,args.Length());
    
    delete[] components;
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::scn(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() == 0 ||
        (args.Length() == 1 && !args[0]->IsNumber()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide at least one color component");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    bool hasPatternArgument = args[args.Length()-1]->IsString();
    int componentsLength = hasPatternArgument ? args.Length() - 1 : args.Length();
    
    double* components = new double[componentsLength];
    for(int i = 0; i < componentsLength; ++i)
        components[i] = TO_NUMBER(args[i])->Value();
    
    if(hasPatternArgument)
        contentContext->GetContext()->scn(components,componentsLength,*UTF_8_VALUE(args[args.Length()-1]->TO_STRING()));
    else
        contentContext->GetContext()->scn(components,componentsLength);
    
    delete[] components;
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::G(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, gray value (0-255)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->G(TO_NUMBER(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::g(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context using pdfWriter.startPageContentContext(page)");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 || !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Argument, please provide 1 parameter, gray value (0-255)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->g(TO_NUMBER(args[0])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::RG(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 3 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 3 arguments as rgb color values");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->RG(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::rg(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 3 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 3 arguments as rgb color values");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->rg(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}


METHOD_RETURN_TYPE AbstractContentContextDriver::K(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		THROW_EXCEPTION("Wrong Argument, please provide 4 cmyk components (values should be 0-255)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->K(TO_NUMBER(args[0])->Value(),
                                    TO_NUMBER(args[1])->Value(),
                                    TO_NUMBER(args[2])->Value(),
                                    TO_NUMBER(args[3])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::k(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 4 || !args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
		THROW_EXCEPTION("Wrong Argument, please provide 4 cmyk components (values should be 0-255)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->k(TO_NUMBER(args[0])->Value(),
                                    TO_NUMBER(args[1])->Value(),
                                    TO_NUMBER(args[2])->Value(),
                                    TO_NUMBER(args[3])->Value());
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::W(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->GetContext()->W();
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::WStar(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->GetContext()->WStar();
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::doXObject(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext() || !contentContext->mResourcesDictionary)
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() != 1)
    {
        THROW_EXCEPTION("Invalid arguments. pass an xobject");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args[0]->IsString())
    {
        // string type, form name in local resources dictionary
        contentContext->GetContext()->Do(*UTF_8_VALUE(args[0]->TO_STRING()));
    }
    else if(contentContext->holder->IsFormXObjectInstance(args[0]))
    {
        // a form object
        FormXObjectDriver* formDriver = ObjectWrap::Unwrap<FormXObjectDriver>(args[0]->TO_OBJECT());
        if(!formDriver)
        {
            THROW_EXCEPTION("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        }
        
        contentContext->GetContext()->Do(contentContext->mResourcesDictionary->AddFormXObjectMapping(formDriver->FormXObject->GetObjectID()));
    }else if(contentContext->holder->IsImageXObjectInstance(args[0]))
    {
        // an image object
        ImageXObjectDriver* imageDriver = ObjectWrap::Unwrap<ImageXObjectDriver>(args[0]->TO_OBJECT());
        if(!imageDriver)
        {
            THROW_EXCEPTION("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        }
        
        contentContext->GetContext()->Do(contentContext->mResourcesDictionary->AddImageXObjectMapping(imageDriver->ImageXObject));
    }
    else
    {
        THROW_EXCEPTION("Wrong arguments, provide an xobject as the single parameter or its name according to the local resource dictionary");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tc(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide character space");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Tc(TO_NUMBER(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tw(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide word space");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Tw(TO_NUMBER(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tz(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide horizontal scaling");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Tz(TO_INT32(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::TL(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide text leading");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->TL(TO_NUMBER(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tr(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide rendering mode");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Tr(TO_INT32(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Ts(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide font rise");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Ts(TO_NUMBER(args[0])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::BT(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->BT();
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::ET(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
        THROW_EXCEPTION("Null content context. Please create a context");
    else
        contentContext->GetContext()->ET();
    SET_FUNCTION_RETURN_VALUE(args.This())
}


METHOD_RETURN_TYPE AbstractContentContextDriver::Td(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 2 arguments");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Td(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::TD(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 2 arguments");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->TD(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tm(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 6 ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber() ||
        !args[5]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 6 arguments forming a 2d transformation matrix (for text)");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->Tm(TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value(),
                                     TO_NUMBER(args[3])->Value(),
                                     TO_NUMBER(args[4])->Value(),
                                     TO_NUMBER(args[5])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::TStar(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->GetContext()->TStar();
    SET_FUNCTION_RETURN_VALUE(args.This())
}



METHOD_RETURN_TYPE AbstractContentContextDriver::Tf(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 2 ||
        (!contentContext->holder->IsUsedFontInstance(args[0]) && !args[0]->IsString()) ||
        !args[1]->IsNumber())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide a font object (create with pdfWriter.getFontForFile) or font resource name and a size measure");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    if(args[0]->IsString())
        contentContext->GetContext()->TfLow(
                                         *UTF_8_VALUE(args[0]->TO_STRING()),
                                         TO_NUMBER(args[1])->Value());
    else
        contentContext->GetContext()->Tf(
                                     ObjectWrap::Unwrap<UsedFontDriver>(args[0]->TO_OBJECT())->UsedFont,
                                     TO_NUMBER(args[1])->Value());
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Tj(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if ((args.Length() != 1 && args.Length() != 2 ) ||
        (!args[0]->IsString() && !args[0]->IsArray()) ||
        (args.Length() == 2 && !args[1]->IsObject()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 1 argument, the string that you wish to display or a glyphs IDs array, and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
        
    
    if(args[0]->IsArray())
    {
        contentContext->GetContext()->Tj(ArrayToGlyphsList(args[0]));
    }
    else
    {
        TextPlacingOptions options;
        if(args.Length() == 2)
            options = ObjectToOptions(args[1]->TO_OBJECT());

        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->TjLow(*UTF_8_VALUE(args[0]->TO_STRING()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->TjHexLow(*UTF_8_VALUE(args[0]->TO_STRING()));
                break;
            default:
                contentContext->GetContext()->Tj(*UTF_8_VALUE(args[0]->TO_STRING()));
        }
    }
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::Quote(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if ((args.Length() != 1 && args.Length() != 2 ) ||
        (!args[0]->IsString() && !args[0]->IsArray()) ||
        (args.Length() == 2 && !args[1]->IsObject()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 1 argument, the string that you wish to display or a glyphs IDs array, and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    if(args[0]->IsArray())
    {
        contentContext->GetContext()->Quote(ArrayToGlyphsList(args[0]));
    }
    else
    {
    
        TextPlacingOptions options;
        if(args.Length() == 2)
            options = ObjectToOptions(args[1]->TO_OBJECT());
    
        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->QuoteLow(*UTF_8_VALUE(args[0]->TO_STRING()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->QuoteHexLow(*UTF_8_VALUE(args[0]->TO_STRING()));
                break;
            default:
                contentContext->GetContext()->Quote(*UTF_8_VALUE(args[0]->TO_STRING()));
        }
    }
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::DoubleQuote(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if ((args.Length() != 3 && args.Length() != 4)||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        (!args[2]->IsString() && !args[2]->IsArray()) ||
        (args.Length() == 4 && !args[3]->IsObject()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide 3 arguments, word spacing, character spacing and text, and optionally an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    if(args[2]->IsArray())
    {
        contentContext->GetContext()->DoubleQuote(TO_NUMBER(args[0])->Value(),
                                                  TO_NUMBER(args[1])->Value(),
                                                  ArrayToGlyphsList(args[2]));
    }
    else
    {
    
        TextPlacingOptions options;
        if(args.Length() == 4)
            options = ObjectToOptions(args[3]->TO_OBJECT());
        
        switch(options.encoding)
        {
            case TextPlacingOptions::EEncodingCode:
                contentContext->GetContext()->DoubleQuoteLow(TO_NUMBER(args[0])->Value(),
                                                          TO_NUMBER(args[1])->Value(),
                                                          *UTF_8_VALUE(args[2]->TO_STRING()));
                break;
            case TextPlacingOptions::EEncodingHex:
                contentContext->GetContext()->DoubleQuoteHexLow(TO_NUMBER(args[0])->Value(),
                                                          TO_NUMBER(args[1])->Value(),
                                                          *UTF_8_VALUE(args[2]->TO_STRING()));
                break;
            default:
                contentContext->GetContext()->DoubleQuote(TO_NUMBER(args[0])->Value(),
                                                          TO_NUMBER(args[1])->Value(),
                                                          *UTF_8_VALUE(args[2]->TO_STRING()));
        }
    }
    
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}

METHOD_RETURN_TYPE AbstractContentContextDriver::TJ(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    // first, scan args to see if has arrays or lists, to realize which form of TJ to run
    bool hasStrings = false;
    bool hasOptions = (args.Length()>0) && (!args[args.Length()-1]->IsString())
                                        && (!args[args.Length()-1]->IsNumber())
                                        && (args[args.Length()-1]->IsObject());
    for(int i=0;i<args.Length() && !hasStrings;++i)
        hasStrings = args[i]->IsString();
    
    if(hasStrings)
    {
        StringOrDoubleList params;

        TextPlacingOptions options;
        if(hasOptions)
            options = ObjectToOptions(args[args.Length()-1]->TO_OBJECT());
        
        bool status = true;
        int lengthButOptions = hasOptions ? (args.Length()-1) : args.Length();
        for(int i=0; i < lengthButOptions && status; ++i)
        {
            if(args[i]->IsString())
                params.push_back(StringOrDouble(*UTF_8_VALUE(args[i]->TO_STRING())));
            else if(args[i]->IsNumber())
                params.push_back(StringOrDouble(TO_NUMBER(args[i])->Value()));
            else
                status = false;
        }
        
        if(!status)
        {
            THROW_EXCEPTION("Wrong arguments. please provide a variable number of elements each either string/glyphs list or number, and an optional final options object");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
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
        for(int i=0; i < args.Length() && status; ++i)
        {
            if(args[i]->IsArray())
                params.push_back(GlyphUnicodeMappingListOrDouble(ArrayToGlyphsList(args[i])));
            else if(args[i]->IsNumber())
                params.push_back(GlyphUnicodeMappingListOrDouble(TO_NUMBER(args[i])->Value()));
            else
                status = false;
        }
        
        if(!status)
        {
            THROW_EXCEPTION("Wrong arguments. please provide a variable number of elements each either string/glyph list or number, and an optional final options object");
            SET_FUNCTION_RETURN_VALUE(UNDEFINED)
        }
        contentContext->GetContext()->TJ(params);
        
    }
    SET_FUNCTION_RETURN_VALUE(args.This())
}

TextPlacingOptions AbstractContentContextDriver::ObjectToOptions(const Local<Object>& inObject)
{
	CREATE_ISOLATE_CONTEXT;
	TextPlacingOptions options;
    
	if (inObject->Has(GET_CURRENT_CONTEXT, NEW_SYMBOL("encoding")).FromJust())
    {
        std::string value = *UTF_8_VALUE(inObject->Get(GET_CURRENT_CONTEXT, NEW_SYMBOL("encoding")).ToLocalChecked()->TO_STRING());
        if(value.compare("hex"))
            options.encoding = TextPlacingOptions::EEncodingHex;
        else if(value.compare("code"))
            options.encoding = TextPlacingOptions::EEncodingCode;
    
        // EEncodingText is the default
    }
    
    return options;
}

GlyphUnicodeMappingList AbstractContentContextDriver::ArrayToGlyphsList(const v8::Local<v8::Value>& inArray)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;
	
	GlyphUnicodeMappingList glyphList;

    int arrayLength =TO_INT32(inArray->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked())->Value();
    Local<Object> arrayObject = inArray->TO_OBJECT();

    for(int i=0; i < arrayLength; ++i)
    {
        if(!arrayObject->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->IsArray())
            continue;
        
        int itemLength = TO_INT32(arrayObject->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked())->Value();
        if(0 == itemLength)
            continue;
        
        GlyphUnicodeMapping mapping;
        
        mapping.mGlyphCode = TO_UINT32(arrayObject->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, 0).ToLocalChecked())->Value();
        for(int j=1; j < itemLength;++j)
            mapping.mUnicodeValues.push_back(TO_UINT32(arrayObject->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked()->TO_OBJECT()->Get(GET_CURRENT_CONTEXT, j).ToLocalChecked())->Value());
			
		glyphList.push_back(mapping);
    }
   
    return glyphList;
}

METHOD_RETURN_TYPE AbstractContentContextDriver::WriteFreeCode(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
	if (args.Length() != 1 ||
        !args[0]->IsString())
    {
		THROW_EXCEPTION("Wrong Arguments, please provide string to write");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
	}
    
    contentContext->GetContext()->WriteFreeCode(*UTF_8_VALUE(args[0]->TO_STRING()));
    SET_FUNCTION_RETURN_VALUE(args.This())
    
}

/* context.drawPath(x1,y1,x2,y2,x3,y3...{type:stroke, color:#FF00FF, width:3, close:true})
 */
METHOD_RETURN_TYPE AbstractContentContextDriver::DrawPath(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 2)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide path coordinates and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    
    contentContext->GetContext()->m(TO_NUMBER(args[0])->Value(),TO_NUMBER(args[1])->Value());
    for(int i=2;i<args.Length()-1;i+=2)
    {
        if(!args[i]->IsNumber()) // options object, stop
            break;
        contentContext->GetContext()->l(TO_NUMBER(args[i])->Value(),TO_NUMBER(args[i+1])->Value());
    }
    
    contentContext->FinishPath(args[args.Length() - 1]);
    SET_FUNCTION_RETURN_VALUE(args.This())
}

void AbstractContentContextDriver::SetupColorAndLineWidth(const Local<Value>& inMaybeOptions)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    if(!inMaybeOptions->IsObject())
        return;
    
    Local<Object> options = inMaybeOptions->TO_OBJECT();
    
    bool isStroke = !options->Has(GET_CURRENT_CONTEXT, NEW_STRING("type")).FromJust() ||
                    strcmp(*UTF_8_VALUE(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("type")).ToLocalChecked()),"stroke") == 0;
    SetColor(inMaybeOptions,isStroke);
    
    if(isStroke && options->Has(GET_CURRENT_CONTEXT, NEW_STRING("width")).FromJust())
        GetContext()->w(TO_NUMBER(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("width")).ToLocalChecked())->Value());
}

void AbstractContentContextDriver::SetColor(const Local<Value>& inMaybeOptions,bool inIsStroke)
{
	CREATE_ISOLATE_CONTEXT;
	
	if (!inMaybeOptions->IsObject())
        return;
    
    Local<Object> options = inMaybeOptions->TO_OBJECT();

    if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("color")).FromJust())
    {
        if(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("color")).ToLocalChecked()->IsString())
        {
            // string, named color. always RGB (for now)
            SetRGBColor(sColorMap.GetRGBForColorName(*UTF_8_VALUE(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("color")).ToLocalChecked()->TO_STRING())),inIsStroke);
        }
        else
        {
            // should be number
            unsigned long colorvalue = (unsigned long)(TO_INT32(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("color")).ToLocalChecked())->Value());
            std::string colorspace = options->Has(GET_CURRENT_CONTEXT, NEW_STRING("colorspace")).FromJust() ?
            *UTF_8_VALUE(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("colorspace")->TO_STRING()).ToLocalChecked()) :
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

void AbstractContentContextDriver::FinishPath(const Local<Value>& inMaybeOptions)
{
	CREATE_ISOLATE_CONTEXT;
	
	bool closePath = false;
    std::string type = "stroke";
    
    if(inMaybeOptions->IsObject())
    {
    
        Local<Object> options = inMaybeOptions->TO_OBJECT();
    
        if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("type")).FromJust())
            type = *UTF_8_VALUE(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("type")).ToLocalChecked());
        
        if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("close")).FromJust())
            closePath = options->Get(GET_CURRENT_CONTEXT, NEW_STRING("close")).ToLocalChecked()->TO_BOOLEAN()->Value();
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

METHOD_RETURN_TYPE AbstractContentContextDriver::DrawCircle(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 3)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide x and y coordinates for center, radius and an optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    
    const double magic = 0.551784;
    double x = TO_NUMBER(args[0])->Value();
    double y = TO_NUMBER(args[1])->Value();
    double r = TO_NUMBER(args[2])->Value();
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
    SET_FUNCTION_RETURN_VALUE(args.This())
}

/* context.drawSquare(x,y,l,[{type:stroke, color:#FF00FF, width:3, close:true}])
 */
METHOD_RETURN_TYPE AbstractContentContextDriver::DrawSquare(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 3)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide bottom left coordinates, an edge size and optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    contentContext->GetContext()->re(
        TO_NUMBER(args[0])->Value(),
        TO_NUMBER(args[1])->Value(),
        TO_NUMBER(args[2])->Value(),
        TO_NUMBER(args[2])->Value()
    );

    contentContext->FinishPath(args[args.Length() - 1]);
    SET_FUNCTION_RETURN_VALUE(args.This())
}

/* context.drawSquare(x,y,w,h,[{type:stroke, color:#FF00FF, width:3, close:true}])
 */
METHOD_RETURN_TYPE AbstractContentContextDriver::DrawRectangle(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 4)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide bottom left coordinates, width and height and optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->SetupColorAndLineWidth(args[args.Length() - 1]);
    contentContext->GetContext()->re(
                                     TO_NUMBER(args[0])->Value(),
                                     TO_NUMBER(args[1])->Value(),
                                     TO_NUMBER(args[2])->Value(),
                                     TO_NUMBER(args[3])->Value()
                                     );
    
    contentContext->FinishPath(args[args.Length() - 1]);
    SET_FUNCTION_RETURN_VALUE(args.This())
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
METHOD_RETURN_TYPE AbstractContentContextDriver::WriteText(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 3)
    {
		THROW_EXCEPTION("Wrong Arguments, please provide the text and x,y coordinate for text position. optionally also add an options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    contentContext->GetContext()->BT();
	if(args.Length() >= 4)
	{
		contentContext->SetColor(args[3],false);
		contentContext->SetFont(args[3]);
	}

	std::string text = *UTF_8_VALUE(args[0]->TO_STRING());
	double xPos = TO_NUMBER(args[1])->Value();
	double yPos = TO_NUMBER(args[2])->Value();

    contentContext->GetContext()->Tm(1,0,0,1,xPos,yPos);
    contentContext->GetContext()->Tj(text);
    contentContext->GetContext()->ET();


	// underline
	if(args.Length() >= 4 && args[3]->IsObject())
	{
		Local<Object> options = args[3]->TO_OBJECT();
		if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("underline")).FromJust() && 
				options->Get(GET_CURRENT_CONTEXT, NEW_STRING("underline")).ToLocalChecked()->TO_BOOLEAN()->Value() &&
				contentContext->holder->IsUsedFontInstance(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("font")).ToLocalChecked()))
		{
			// draw underline. use font data for position and thickness
			double fontSize = options->Has(GET_CURRENT_CONTEXT, NEW_STRING("size")).FromJust() ? TO_NUMBER(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("size")).ToLocalChecked())->Value():1;

			PDFUsedFont* font = ObjectWrap::Unwrap<UsedFontDriver>(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("font")).ToLocalChecked()->TO_OBJECT())->UsedFont;
			FreeTypeFaceWrapper*  ftWrapper = font->GetFreeTypeFont();

			contentContext->SetColor(args[3],true);
			contentContext->GetContext()->w(sGetUnderlineThicknessFactor(ftWrapper)*fontSize);
			double startLine = yPos+sGetUnderlinePositionFactor(ftWrapper)*fontSize;
			contentContext->GetContext()->m(xPos,startLine);
			contentContext->GetContext()->l(xPos + font->CalculateTextAdvance(text,fontSize),startLine);
			contentContext->GetContext()->S();
		}
	}

    SET_FUNCTION_RETURN_VALUE(args.This())
}

void AbstractContentContextDriver::SetFont(const v8::Local<v8::Value>& inMaybeOptions)
{
	CREATE_ISOLATE_CONTEXT;
	
	if (!inMaybeOptions->IsObject())
        return;
    
    Local<Object> options = inMaybeOptions->TO_OBJECT();
    
    if(options->Has(GET_CURRENT_CONTEXT, NEW_STRING("font")).FromJust() &&
       holder->IsUsedFontInstance(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("font")).ToLocalChecked()))
        GetContext()->Tf(ObjectWrap::Unwrap<UsedFontDriver>(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("font")).ToLocalChecked()->TO_OBJECT())->UsedFont,
                         options->Has(GET_CURRENT_CONTEXT, NEW_STRING("size")).FromJust() ? TO_NUMBER(options->Get(GET_CURRENT_CONTEXT, NEW_STRING("size")).ToLocalChecked())->Value():1);
}

METHOD_RETURN_TYPE AbstractContentContextDriver::DrawImage(const ARGS_TYPE& args)
{
	CREATE_ISOLATE_CONTEXT;
	CREATE_ESCAPABLE_SCOPE;

    AbstractContentContextDriver* contentContext = ObjectWrap::Unwrap<AbstractContentContextDriver>(args.This());
    if(!contentContext->GetContext())
    {
        THROW_EXCEPTION("Null content context. Please create a context");
        SET_FUNCTION_RETURN_VALUE(UNDEFINED)
    }
    
    if(args.Length() < 3 ||
       !args[0]->IsNumber() ||
       !args[1]->IsNumber() ||
       !args[2]->IsString() ||
       (args.Length() >= 4 && !args[3]->IsObject()))
    {
		THROW_EXCEPTION("Wrong Arguments, please provide bottom left coordinates, an edge size and optional options object");
		SET_FUNCTION_RETURN_VALUE(UNDEFINED)
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

    double x = TO_NUMBER(args[0])->Value();
    double y = TO_NUMBER(args[1])->Value();
    std::string imagePath = *(UTF_8_VALUE(args[2]->TO_STRING()));
    AbstractContentContext::ImageOptions imageOptions;
    
    if(args.Length() >= 4)
    {
        Local<Object> optionsObject = args[3]->TO_OBJECT();
        
        if(optionsObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("index")).FromJust())
            imageOptions.imageIndex = TO_UINT32(optionsObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("index")).ToLocalChecked())->Value();
        
        if(optionsObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("transformation")).FromJust())
        {
            Local<Value> transformationValue = optionsObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("transformation")).ToLocalChecked();
            
            if(transformationValue->IsArray() || transformationValue->IsObject())
            {
                Local<Object> transformationObject = transformationValue->TO_OBJECT();
                
                if(transformationValue->IsArray() && TO_NUMBER(transformationObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("length")).ToLocalChecked())->Value() == 6)
                {
                    imageOptions.transformationMethod = AbstractContentContext::eMatrix;
                    for(int i=0;i<6;++i)
                        imageOptions.matrix[i] = TO_NUMBER(transformationObject->Get(GET_CURRENT_CONTEXT, i).ToLocalChecked())->Value();
                }
                else if(transformationValue->IsObject())
                {
                    // fitting object, determine transformation according to image dimensions relation to width/height
                    imageOptions.transformationMethod = AbstractContentContext::eFit;
                    imageOptions.boundingBoxWidth = TO_NUMBER(transformationObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("width")).ToLocalChecked())->Value();
                    imageOptions.boundingBoxHeight = TO_NUMBER(transformationObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("height")).ToLocalChecked())->Value();
                    imageOptions.fitProportional = transformationObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("proportional")).FromJust() ?
                                            transformationObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("proportional")).ToLocalChecked()->TO_BOOLEAN()->Value() :
                                            false;
                    imageOptions.fitPolicy = transformationObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("fit")).FromJust() ?
                                    (strcmp("always",*UTF_8_VALUE(transformationObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("fit")).ToLocalChecked()->TO_STRING())) == 0 ? AbstractContentContext::eAlways : AbstractContentContext::eOverflow):
                                    AbstractContentContext::eOverflow;
                }
                
            }
        }

        if(optionsObject->Has(GET_CURRENT_CONTEXT, NEW_STRING("password")).FromJust() && optionsObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->IsString())
        {
            imageOptions.pdfParsingOptions.Password = *UTF_8_VALUE(optionsObject->Get(GET_CURRENT_CONTEXT, NEW_STRING("password")).ToLocalChecked()->TO_STRING());
        }        

    }
    
    contentContext->GetContext()->DrawImage(x,
                                            y,
                                            imagePath,
                                            imageOptions);    
    
    SET_FUNCTION_RETURN_VALUE(args.This())
}
