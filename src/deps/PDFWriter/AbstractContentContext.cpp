/*
   Source File : AbstractContentContext.cpp


   Copyright 2011 Gal Kahana PDFWriter

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
#include "AbstractContentContext.h"
#include "PDFStream.h"
#include "ResourcesDictionary.h"
#include "PDFImageXObject.h"
#include "ProcsetResourcesConstants.h"
#include "PDFUsedFont.h"
#include "Trace.h"
#include "OutputStringBufferStream.h"
#include "SafeBufferMacrosDefs.h"
#include "OutputStreamTraits.h"
#include "IContentContextListener.h"
#include "DocumentContext.h"
#include "SimpleGlyphsDrawingContext.h"
#include "LayeredGlyphsDrawingContext.h"
#include "PaintedGlyphsDrawingContext.h"

#include <ctype.h>
#include <algorithm>

using namespace PDFHummus;

struct CSSColor
{
    const char* name;
    unsigned long rgbValue;
};

CSSColor kCSSColors[] =
{
    {"aliceblue",0xF0F8FF},
    {"antiquewhite",0xFAEBD7},
    {"aqua",0x00FFFF},
    {"aquamarine",0x7FFFD4},
    {"azure",0xF0FFFF},
    {"beige",0xF5F5DC},
    {"bisque",0xFFE4C4},
    {"black",0x000000},
    {"blanchedalmond",0xFFEBCD},
    {"blue",0x0000FF},
    {"blueviolet",0x8A2BE2},
    {"brown",0xA52A2A},
    {"burlywood",0xDEB887},
    {"cadetblue",0x5F9EA0},
    {"chartreuse",0x7FFF00},
    {"chocolate",0xD2691E},
    {"coral",0xFF7F50},
    {"cornflowerblue",0x6495ED},
    {"cornsilk",0xFFF8DC},
    {"crimson",0xDC143C},
    {"cyan",0x00FFFF},
    {"darkblue",0x00008B},
    {"darkcyan",0x008B8B},
    {"darkgoldenrod",0xB8860B},
    {"darkgray",0xA9A9A9},
    {"darkgreen",0x006400},
    {"darkkhaki",0xBDB76B},
    {"darkmagenta",0x8B008B},
    {"darkolivegreen",0x556B2F},
    {"darkorange",0xFF8C00},
    {"darkorchid",0x9932CC},
    {"darkred",0x8B0000},
    {"darksalmon",0xE9967A},
    {"darkseagreen",0x8FBC8F},
    {"darkslateblue",0x483D8B},
    {"darkslategray",0x2F4F4F},
    {"darkturquoise",0x00CED1},
    {"darkviolet",0x9400D3},
    {"deeppink",0xFF1493},
    {"deepskyblue",0x00BFFF},
    {"dimgray",0x696969},
    {"dodgerblue",0x1E90FF},
    {"firebrick",0xB22222},
    {"floralwhite",0xFFFAF0},
    {"forestgreen",0x228B22},
    {"fuchsia",0xFF00FF},
    {"gainsboro",0xDCDCDC},
    {"ghostwhite",0xF8F8FF},
    {"gold",0xFFD700},
    {"goldenrod",0xDAA520},
    {"gray",0x808080},
    {"green",0x008000},
    {"greenyellow",0xADFF2F},
    {"honeydew",0xF0FFF0},
    {"hotpink",0xFF69B4},
    {"indianred",0xCD5C5C},
    {"indigo",0x4B0082},
    {"ivory",0xFFFFF0},
    {"khaki",0xF0E68C},
    {"lavender",0xE6E6FA},
    {"lavenderblush",0xFFF0F5},
    {"lawngreen",0x7CFC00},
    {"lemonchiffon",0xFFFACD},
    {"lightblue",0xADD8E6},
    {"lightcoral",0xF08080},
    {"lightcyan",0xE0FFFF},
    {"lightgoldenrodyellow",0xFAFAD2},
    {"lightgray",0xD3D3D3},
    {"lightgreen",0x90EE90},
    {"lightpink",0xFFB6C1},
    {"lightsalmon",0xFFA07A},
    {"lightseagreen",0x20B2AA},
    {"lightskyblue",0x87CEFA},
    {"lightslategray",0x778899},
    {"lightsteelblue",0xB0C4DE},
    {"lightyellow",0xFFFFE0},
    {"lime",0x00FF00},
    {"limegreen",0x32CD32},
    {"linen",0xFAF0E6},
    {"magenta",0xFF00FF},
    {"maroon",0x800000},
    {"mediumaquamarine",0x66CDAA},
    {"mediumblue",0x0000CD},
    {"mediumorchid",0xBA55D3},
    {"mediumpurple",0x9370DB},
    {"mediumseagreen",0x3CB371},
    {"mediumslateblue",0x7B68EE},
    {"mediumspringgreen",0x00FA9A},
    {"mediumturquoise",0x48D1CC},
    {"mediumvioletred",0xC71585},
    {"midnightblue",0x191970},
    {"mintcream",0xF5FFFA},
    {"mistyrose",0xFFE4E1},
    {"moccasin",0xFFE4B5},
    {"navajowhite",0xFFDEAD},
    {"navy",0x000080},
    {"oldlace",0xFDF5E6},
    {"olive",0x808000},
    {"olivedrab",0x6B8E23},
    {"orange",0xFFA500},
    {"orangered",0xFF4500},
    {"orchid",0xDA70D6},
    {"palegoldenrod",0xEEE8AA},
    {"palegreen",0x98FB98},
    {"paleturquoise",0xAFEEEE},
    {"palevioletred",0xDB7093},
    {"papayawhip",0xFFEFD5},
    {"peachpuff",0xFFDAB9},
    {"peru",0xCD853F},
    {"pink",0xFFC0CB},
    {"plum",0xDDA0DD},
    {"powderblue",0xB0E0E6},
    {"purple",0x800080},
    {"red",0xFF0000},
    {"rosybrown",0xBC8F8F},
    {"royalblue",0x4169E1},
    {"saddlebrown",0x8B4513},
    {"salmon",0xFA8072},
    {"sandybrown",0xF4A460},
    {"seagreen",0x2E8B57},
    {"seashell",0xFFF5EE},
    {"sienna",0xA0522D},
    {"silver",0xC0C0C0},
    {"skyblue",0x87CEEB},
    {"slateblue",0x6A5ACD},
    {"slategray",0x708090},
    {"snow",0xFFFAFA},
    {"springgreen",0x00FF7F},
    {"steelblue",0x4682B4},
    {"tan",0xD2B48C},
    {"teal",0x008080},
    {"thistle",0xD8BFD8},
    {"tomato",0xFF6347},
    {"turquoise",0x40E0D0},
    {"violet",0xEE82EE},
    {"wheat",0xF5DEB3},
    {"white",0xFFFFFF},
    {"whitesmoke",0xF5F5F5},
    {"yellow",0xFFFF00},
    {"yellowgreen",0x9ACD32},
    {"",0}
};


typedef std::map<std::string,unsigned long> StringToULongMap;

class CSSColorMap
{
public:
    CSSColorMap();
    
    unsigned long GetRGBForColorName(const std::string& inColorName);
    
private:
    
    StringToULongMap mColorMap;
    
};

CSSColorMap::CSSColorMap()
{
    unsigned long i;

    for(i=0;strlen(kCSSColors[i].name) != 0; ++i)
        mColorMap.insert(StringToULongMap::value_type(kCSSColors[i].name,kCSSColors[i].rgbValue));
    
}


unsigned long CSSColorMap::GetRGBForColorName(const std::string& inColorName)
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
static CSSColorMap sColorMap;

unsigned long AbstractContentContext::ColorValueForName(const std::string& inColorName)
{
	return sColorMap.GetRGBForColorName(inColorName);
}

AbstractContentContext::AbstractContentContext(PDFHummus::DocumentContext* inDocumentContext)
{
	mDocumentContext = inDocumentContext;
}

AbstractContentContext::~AbstractContentContext(void)
{
}

void AbstractContentContext::SetPDFStreamForWrite(PDFStream* inStream)
{
	mPrimitiveWriter.SetStreamForWriting(inStream->GetWriteStream());	
}

void AbstractContentContext::AssertProcsetAvailable(const std::string& inProcsetName)
{
	GetResourcesDictionary()->AddProcsetResource(inProcsetName);	
}


static const std::string scAddRectangleToPath = "re";
void AbstractContentContext::re(double inLeft,double inBottom, double inWidth,double inHeight)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inLeft);
	mPrimitiveWriter.WriteDouble(inBottom);
	mPrimitiveWriter.WriteDouble(inWidth);
	mPrimitiveWriter.WriteDouble(inHeight);
	mPrimitiveWriter.WriteKeyword(scAddRectangleToPath);
}

static const std::string scFill = "f";
void AbstractContentContext::f()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword(scFill);
}

void AbstractContentContext::S()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("S");
}

void AbstractContentContext::s()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("s");
}

void AbstractContentContext::F()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("F");
}

void AbstractContentContext::fStar()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("f*");
}

void AbstractContentContext::B()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("B");
}

void AbstractContentContext::BStar()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("B*");
}

void AbstractContentContext::b()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("b");
}

void AbstractContentContext::bStar()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("b*");
}

void AbstractContentContext::n()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("n");
}

void AbstractContentContext::m(double inX,double inY)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inX);
	mPrimitiveWriter.WriteDouble(inY);
	mPrimitiveWriter.WriteKeyword("m");
}

void AbstractContentContext::l(double inX,double inY)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inX);
	mPrimitiveWriter.WriteDouble(inY);
	mPrimitiveWriter.WriteKeyword("l");
}

void AbstractContentContext::c(	double inX1,double inY1, 
							double inX2, double inY2, 
							double inX3, double inY3)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inX1);
	mPrimitiveWriter.WriteDouble(inY1);
	mPrimitiveWriter.WriteDouble(inX2);
	mPrimitiveWriter.WriteDouble(inY2);
	mPrimitiveWriter.WriteDouble(inX3);
	mPrimitiveWriter.WriteDouble(inY3);
	mPrimitiveWriter.WriteKeyword("c");
}

void AbstractContentContext::v(	double inX2,double inY2, 
							double inX3, double inY3)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inX2);
	mPrimitiveWriter.WriteDouble(inY2);
	mPrimitiveWriter.WriteDouble(inX3);
	mPrimitiveWriter.WriteDouble(inY3);
	mPrimitiveWriter.WriteKeyword("v");
}

void AbstractContentContext::y(	double inX1,double inY1, 
							double inX3, double inY3)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inX1);
	mPrimitiveWriter.WriteDouble(inY1);
	mPrimitiveWriter.WriteDouble(inX3);
	mPrimitiveWriter.WriteDouble(inY3);
	mPrimitiveWriter.WriteKeyword("y");
}

void AbstractContentContext::h()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("h");
}

void AbstractContentContext::q()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("q");
	mGraphicStack.Push();
    
    IContentContextListenerSet::iterator it = mListeners.begin();
    for(; it != mListeners.end();++it)
        (*it)->Onq(this);
}

EStatusCode AbstractContentContext::Q()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("Q");
	EStatusCode status = mGraphicStack.Pop();
    
    IContentContextListenerSet::iterator it = mListeners.begin();
    for(; it != mListeners.end();++it)
        (*it)->OnQ(this);
    
    return status;
}

void AbstractContentContext::cm(double inA, double inB, double inC, double inD, double inE, double inF)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inA);
	mPrimitiveWriter.WriteDouble(inB);
	mPrimitiveWriter.WriteDouble(inC);
	mPrimitiveWriter.WriteDouble(inD);
	mPrimitiveWriter.WriteDouble(inE);
	mPrimitiveWriter.WriteDouble(inF);
	mPrimitiveWriter.WriteKeyword("cm");
}

void AbstractContentContext::w(double inLineWidth)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inLineWidth);
	mPrimitiveWriter.WriteKeyword("w");
}

void AbstractContentContext::J(int inLineCapStyle)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteInteger(inLineCapStyle);
	mPrimitiveWriter.WriteKeyword("J");
}

void AbstractContentContext::j(int inLineJoinStyle)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteInteger(inLineJoinStyle);
	mPrimitiveWriter.WriteKeyword("j");
}

void AbstractContentContext::M(double inMiterLimit)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inMiterLimit);
	mPrimitiveWriter.WriteKeyword("M");
}

void AbstractContentContext::d(double* inDashArray, int inDashArrayLength, double inDashPhase)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.StartArray();
	for (int i = 0; i<inDashArrayLength; ++i)
		mPrimitiveWriter.WriteDouble(inDashArray[i]);
	mPrimitiveWriter.EndArray(eTokenSeparatorSpace);
	mPrimitiveWriter.WriteDouble(inDashPhase);
	mPrimitiveWriter.WriteKeyword("d");
}

void AbstractContentContext::ri(const std::string& inRenderingIntentName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inRenderingIntentName);
	mPrimitiveWriter.WriteKeyword("ri");
}

void AbstractContentContext::i(int inFlatness)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteInteger(inFlatness);
	mPrimitiveWriter.WriteKeyword("i");
}

void AbstractContentContext::gs(const std::string& inGraphicStateName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inGraphicStateName);
	mPrimitiveWriter.WriteKeyword("gs");
}


void AbstractContentContext::CS(const std::string& inColorSpaceName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inColorSpaceName);
	mPrimitiveWriter.WriteKeyword("CS");
}

void AbstractContentContext::cs(const std::string& inColorSpaceName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inColorSpaceName);
	mPrimitiveWriter.WriteKeyword("cs");
}

void AbstractContentContext::SC(double* inColorComponents, int inColorComponentsLength)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteKeyword("SC");
}

void AbstractContentContext::SCN(double* inColorComponents, int inColorComponentsLength)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteKeyword("SCN");
}

void AbstractContentContext::SCN(double* inColorComponents, int inColorComponentsLength,const std::string& inPatternName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteName(inPatternName);
	mPrimitiveWriter.WriteKeyword("SCN");
}

void AbstractContentContext::SCN(const std::string& inPatternName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inPatternName);
	mPrimitiveWriter.WriteKeyword("SCN");
}

void AbstractContentContext::sc(double* inColorComponents, int inColorComponentsLength)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteKeyword("sc");
}

void AbstractContentContext::scn(double* inColorComponents, int inColorComponentsLength)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteKeyword("scn");
}

void AbstractContentContext::scn(double* inColorComponents, int inColorComponentsLength,const std::string& inPatternName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	for(int i=0;i<inColorComponentsLength;++i)
		mPrimitiveWriter.WriteDouble(inColorComponents[i]);
	mPrimitiveWriter.WriteName(inPatternName);
	mPrimitiveWriter.WriteKeyword("scn");
}

void AbstractContentContext::scn(const std::string& inPatternName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inPatternName);
	mPrimitiveWriter.WriteKeyword("scn");
}

void AbstractContentContext::G(double inGray)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inGray);
	mPrimitiveWriter.WriteKeyword("G");
}

void AbstractContentContext::g(double inGray)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inGray);
	mPrimitiveWriter.WriteKeyword("g");
}

void AbstractContentContext::RG(double inR,double inG,double inB)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inR);
	mPrimitiveWriter.WriteDouble(inG);
	mPrimitiveWriter.WriteDouble(inB);
	mPrimitiveWriter.WriteKeyword("RG");
}

void AbstractContentContext::rg(double inR,double inG,double inB)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inR);
	mPrimitiveWriter.WriteDouble(inG);
	mPrimitiveWriter.WriteDouble(inB);
	mPrimitiveWriter.WriteKeyword("rg");
}

void AbstractContentContext::K(double inC,double inM,double inY,double inK)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inC);
	mPrimitiveWriter.WriteDouble(inM);
	mPrimitiveWriter.WriteDouble(inY);
	mPrimitiveWriter.WriteDouble(inK);
	mPrimitiveWriter.WriteKeyword("K");
}

void AbstractContentContext::k(double inC,double inM,double inY,double inK)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteDouble(inC);
	mPrimitiveWriter.WriteDouble(inM);
	mPrimitiveWriter.WriteDouble(inY);
	mPrimitiveWriter.WriteDouble(inK);
	mPrimitiveWriter.WriteKeyword("k");
}

void AbstractContentContext::W()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("W");
}

void AbstractContentContext::WStar()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteKeyword("W*");
}

void AbstractContentContext::Do(const std::string& inXObjectName)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);

	mPrimitiveWriter.WriteName(inXObjectName);	
	mPrimitiveWriter.WriteKeyword("Do");	
}

void AbstractContentContext::Tc(double inCharacterSpace)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inCharacterSpace);
	mPrimitiveWriter.WriteKeyword("Tc");
}

void AbstractContentContext::Tw(double inWordSpace)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inWordSpace);
	mPrimitiveWriter.WriteKeyword("Tw");
}

void AbstractContentContext::Tz(int inHorizontalScaling)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteInteger(inHorizontalScaling);
	mPrimitiveWriter.WriteKeyword("Tz");
}

void AbstractContentContext::TL(double inTextLeading)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inTextLeading);
	mPrimitiveWriter.WriteKeyword("TL");
}

void AbstractContentContext::TfLow(const std::string& inFontName,double inFontSize)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteName(inFontName);
	mPrimitiveWriter.WriteDouble(inFontSize);
	mPrimitiveWriter.WriteKeyword("Tf");

	mGraphicStack.GetCurrentState().mPlacedFontName = inFontName;
	mGraphicStack.GetCurrentState().mPlacedFontSize = inFontSize;
}

void AbstractContentContext::Tr(int inRenderingMode)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteInteger(inRenderingMode);
	mPrimitiveWriter.WriteKeyword("Tr");
}

void AbstractContentContext::Ts(double inFontRise)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inFontRise);
	mPrimitiveWriter.WriteKeyword("Ts");
}

void AbstractContentContext::BT()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteKeyword("BT");
}

void AbstractContentContext::ET()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteKeyword("ET");
}

void AbstractContentContext::Td(double inTx, double inTy)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inTx);
	mPrimitiveWriter.WriteDouble(inTy);
	mPrimitiveWriter.WriteKeyword("Td");
}

void AbstractContentContext::TD(double inTx, double inTy)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inTx);
	mPrimitiveWriter.WriteDouble(inTy);
	mPrimitiveWriter.WriteKeyword("TD");
}

void AbstractContentContext::Tm(double inA, double inB, double inC, double inD, double inE, double inF)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inA);
	mPrimitiveWriter.WriteDouble(inB);
	mPrimitiveWriter.WriteDouble(inC);
	mPrimitiveWriter.WriteDouble(inD);
	mPrimitiveWriter.WriteDouble(inE);
	mPrimitiveWriter.WriteDouble(inF);
	mPrimitiveWriter.WriteKeyword("Tm");
}

void AbstractContentContext::TStar()
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteKeyword("T*");
}


void AbstractContentContext::TjLow(const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteLiteralString(inText);
	mPrimitiveWriter.WriteKeyword("Tj");
}

void AbstractContentContext::TjHexLow(const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteHexString(inText);
	mPrimitiveWriter.WriteKeyword("Tj");
}

void AbstractContentContext::QuoteLow(const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteLiteralString(inText);
	mPrimitiveWriter.WriteKeyword("'");
}

void AbstractContentContext::QuoteHexLow(const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteHexString(inText);
	mPrimitiveWriter.WriteKeyword("Quote");
}

void AbstractContentContext::DoubleQuoteLow(	double inWordSpacing, 
											double inCharacterSpacing, 
											const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inWordSpacing);
	mPrimitiveWriter.WriteDouble(inCharacterSpacing);
	mPrimitiveWriter.WriteLiteralString(inText);
	mPrimitiveWriter.WriteKeyword("\"");
}

void AbstractContentContext::DoubleQuoteHexLow(double inWordSpacing, double inCharacterSpacing, const std::string& inText)
{
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.WriteDouble(inWordSpacing);
	mPrimitiveWriter.WriteDouble(inCharacterSpacing);
	mPrimitiveWriter.WriteHexString(inText);
	mPrimitiveWriter.WriteKeyword("\"");
}

void AbstractContentContext::TJLow(const StringOrDoubleList& inStringsAndSpacing)
{
	StringOrDoubleList::const_iterator it = inStringsAndSpacing.begin();
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.StartArray();

	for(; it != inStringsAndSpacing.end();++it)
	{
		if(it->IsDouble)
			mPrimitiveWriter.WriteDouble(it->DoubleValue);
		else
			mPrimitiveWriter.WriteLiteralString(it->SomeValue);
	}
	
	mPrimitiveWriter.EndArray(eTokenSeparatorSpace);

	mPrimitiveWriter.WriteKeyword("TJ");
}

void AbstractContentContext::TJHexLow(const StringOrDoubleList& inStringsAndSpacing)
{
	StringOrDoubleList::const_iterator it = inStringsAndSpacing.begin();
	RenewStreamConnection();
	AssertProcsetAvailable(KProcsetPDF);
	AssertProcsetAvailable(KProcsetText);

	mPrimitiveWriter.StartArray();

	for(; it != inStringsAndSpacing.end();++it)
	{
		if(it->IsDouble)
			mPrimitiveWriter.WriteDouble(it->DoubleValue);
		else
			mPrimitiveWriter.WriteHexString(it->SomeValue);
	}
	
	mPrimitiveWriter.EndArray(eTokenSeparatorSpace);

	mPrimitiveWriter.WriteKeyword("TJ");
}

void AbstractContentContext::SetCurrentFont(PDFUsedFont* inFontReference) 
{
	mGraphicStack.GetCurrentState().mFont = inFontReference;
}

PDFUsedFont* AbstractContentContext::GetCurrentFont() {
	return mGraphicStack.GetCurrentState().mFont;
}

void AbstractContentContext::SetCurrentFontSize(double inFontSize) 
{
	mGraphicStack.GetCurrentState().mFontSize = inFontSize;
}

void AbstractContentContext::Tf(PDFUsedFont* inFontReference,double inFontSize)
{
	SetCurrentFont(inFontReference);
	SetCurrentFontSize(inFontSize);
}

class ITextCommand
{
public:

	virtual void WriteHexStringCommand(const std::string& inStringToWrite) = 0;
	virtual void WriteLiteralStringCommand(const std::string& inStringToWrite) = 0;
};

EStatusCode AbstractContentContext::WriteTextCommandWithEncoding(const std::string& inUnicodeText,ITextCommand* inTextCommand)
{
	GlyphUnicodeMappingList glyphsAndUnicode;
	EncodeWithCurrentFont(inUnicodeText, glyphsAndUnicode);

	return WriteTextCommandWithDirectGlyphSelection(glyphsAndUnicode,inTextCommand);
}

class TjCommand : public ITextCommand
{
public:
	TjCommand(AbstractContentContext* inContext) {mContext = inContext;}

	virtual void WriteHexStringCommand(const std::string& inStringToWrite){mContext->TjHexLow(inStringToWrite);}
	virtual void WriteLiteralStringCommand(const std::string& inStringToWrite){mContext->TjLow(inStringToWrite);}
private:
	AbstractContentContext* mContext;
};

EStatusCode AbstractContentContext::Tj(const std::string& inText)
{
	TjCommand command(this);
	return WriteTextCommandWithEncoding(inText,&command);
}

class QuoteCommand : public ITextCommand
{
public:
	QuoteCommand(AbstractContentContext* inContext) {mContext = inContext;}

	virtual void WriteHexStringCommand(const std::string& inStringToWrite){mContext->QuoteHexLow(inStringToWrite);}
	virtual void WriteLiteralStringCommand(const std::string& inStringToWrite){mContext->QuoteLow(inStringToWrite);}
private:
	AbstractContentContext* mContext;
};

EStatusCode AbstractContentContext::Quote(const std::string& inText)
{
	QuoteCommand command(this);
	return WriteTextCommandWithEncoding(inText,&command);
}

class DoubleQuoteCommand : public ITextCommand
{
public:
	DoubleQuoteCommand(AbstractContentContext* inContext,
						double inWordSpacing,
						double inCharacterSpacing) {mContext = inContext;mWordSpacing = inWordSpacing;mCharacterSpacing = inCharacterSpacing;}

	virtual void WriteHexStringCommand(const std::string& inStringToWrite){mContext->DoubleQuoteHexLow(mWordSpacing,mCharacterSpacing,inStringToWrite);}
	virtual void WriteLiteralStringCommand(const std::string& inStringToWrite){mContext->DoubleQuoteLow(mWordSpacing,mCharacterSpacing,inStringToWrite);}
private:
	AbstractContentContext* mContext;
	double mWordSpacing;
	double mCharacterSpacing;
};

EStatusCode AbstractContentContext::DoubleQuote(double inWordSpacing, double inCharacterSpacing, const std::string& inText)
{
	DoubleQuoteCommand command(this,inWordSpacing,inCharacterSpacing);
	return WriteTextCommandWithEncoding(inText,&command);
}

EStatusCode AbstractContentContext::TJ(const StringOrDoubleList& inStringsAndSpacing)
{
	PDFUsedFont* currentFont = GetCurrentFont();
	if(!currentFont)
	{
		TRACE_LOG("AbstractContentContext::TJ, Cannot write text, no current font is defined");
		return PDFHummus::eFailure;
	}

	StringOrDoubleList::const_iterator it = inStringsAndSpacing.begin();
	GlyphUnicodeMappingListOrDoubleList parameters;
	EStatusCode encodingStatus;
	
	for(; it != inStringsAndSpacing.end();++it)
	{
		if(it->IsDouble)
		{
			parameters.push_back(GlyphUnicodeMappingListOrDouble(it->DoubleValue));
		}
		else
		{
			GlyphUnicodeMappingList glyphsAndUnicode;
			encodingStatus = currentFont->TranslateStringToGlyphs(it->SomeValue,glyphsAndUnicode);

			// encoding returns false if was unable to encode some of the glyphs. will display as missing characters
			if(encodingStatus != PDFHummus::eSuccess)
				TRACE_LOG("AbstractContextContext::TJ, was unable to find glyphs for all characters, some will appear as missing");
			parameters.push_back(GlyphUnicodeMappingListOrDouble(glyphsAndUnicode));
		}
	}

	return TJ(parameters);
}

EStatusCode AbstractContentContext::Tj(const GlyphUnicodeMappingList& inText)
{
	TjCommand command(this);
	return WriteTextCommandWithDirectGlyphSelection(inText,&command);
}

EStatusCode AbstractContentContext::WriteTextCommandWithDirectGlyphSelection(const GlyphUnicodeMappingList& inText,ITextCommand* inTextCommand)
{
	PDFUsedFont* currentFont = GetCurrentFont();
	if(!currentFont)
	{
		TRACE_LOG("AbstractContentContext::WriteTextCommandWithDirectGlyphSelection, Cannot write text, no current font is defined");
		return PDFHummus::eFailure;
	}

	ObjectIDType fontObjectID;
	UShortList encodedCharactersList;
	bool writeAsCID;	

	if(currentFont->EncodeStringForShowing(inText,fontObjectID,encodedCharactersList,writeAsCID) != PDFHummus::eSuccess)
	{
		TRACE_LOG("AbstractcontextContext::WriteTextCommandWithDirectGlyphSelection, Unexepcted failure, Cannot encode characters");
		return PDFHummus::eFailure;
	}
	
	// skip if there's no text going to be written (also means no font ID)
	if(encodedCharactersList.empty() || 0 == fontObjectID)
		return PDFHummus::eSuccess;

	// Write the font reference (only if required)
	std::string fontName = GetResourcesDictionary()->AddFontMapping(fontObjectID);

	if(mGraphicStack.GetCurrentState().mPlacedFontName != fontName ||
		mGraphicStack.GetCurrentState().mPlacedFontSize != mGraphicStack.GetCurrentState().mFontSize)
		TfLow(fontName,mGraphicStack.GetCurrentState().mFontSize);
	
	// Now write the string using the text command
	OutputStringBufferStream stringStream;
	char formattingBuffer[5];
	UShortList::iterator it = encodedCharactersList.begin();
	if(writeAsCID)
	{
		for(;it!= encodedCharactersList.end();++it)
		{
			formattingBuffer[0] = ((*it) >> 8) & 0x00ff;
			stringStream.Write((const Byte*)formattingBuffer, 1);
			formattingBuffer[0] = (*it) & 0x00ff;
			stringStream.Write((const Byte*)formattingBuffer, 1);
		}
		inTextCommand->WriteHexStringCommand(stringStream.ToString());
	}
	else
	{
		for(;it!= encodedCharactersList.end();++it)
		{
			formattingBuffer[0] = (*it) & 0x00ff;
			stringStream.Write((const Byte*)formattingBuffer,1);
		}
		inTextCommand->WriteLiteralStringCommand(stringStream.ToString());	
	}
	return PDFHummus::eSuccess;
}

EStatusCode AbstractContentContext::Quote(const GlyphUnicodeMappingList& inText)
{
	QuoteCommand command(this);
	return WriteTextCommandWithDirectGlyphSelection(inText,&command);
}

EStatusCode AbstractContentContext::DoubleQuote(double inWordSpacing, double inCharacterSpacing, const GlyphUnicodeMappingList& inText)
{
	DoubleQuoteCommand command(this,inWordSpacing,inCharacterSpacing);
	return WriteTextCommandWithDirectGlyphSelection(inText,&command);
}

EStatusCode AbstractContentContext::TJ(const GlyphUnicodeMappingListOrDoubleList& inStringsAndSpacing)
{
	PDFUsedFont* currentFont = GetCurrentFont();
	if(!currentFont)
	{
		TRACE_LOG("AbstractContentContext::TJ, Cannot write text, no current font is defined");
		return PDFHummus::eSuccess;
	}

	// TJ is a bit different. i want to encode all strings in the array to the same font, so that at most a single
	// Tf is used...and command may be written as is. for that we need to encode all strings at once...which requires
	// a slightly different algorithm

	// first, list all the strings, so that you can encode them
	GlyphUnicodeMappingListList stringsList;
	GlyphUnicodeMappingListOrDoubleList::const_iterator it = inStringsAndSpacing.begin();
	
	for(; it != inStringsAndSpacing.end(); ++it)
		if(!it->IsDouble)
			stringsList.push_back(it->SomeValue);

	// now, encode all strings at once

	ObjectIDType fontObjectID;
	UShortListList encodedCharachtersListsList;
	bool writeAsCID;	

	if(currentFont->EncodeStringsForShowing(stringsList,fontObjectID,encodedCharachtersListsList,writeAsCID)!= PDFHummus::eSuccess)
	{
		TRACE_LOG("AbstractContentContext::TJ, Unexepcted failure, cannot include characters for writing final representation");
		return PDFHummus::eFailure;
	}
	
	// skip if there's no text going to be written (also means no font ID)
	if(encodedCharachtersListsList.empty() || 0 == fontObjectID)
		return PDFHummus::eSuccess;

	// status only returns if strings can be coded or not. so continue with writing regardless

	// Write the font reference (only if required)
	std::string fontName = GetResourcesDictionary()->AddFontMapping(fontObjectID);

	if(mGraphicStack.GetCurrentState().mPlacedFontName != fontName ||
		mGraphicStack.GetCurrentState().mPlacedFontSize != mGraphicStack.GetCurrentState().mFontSize)
		TfLow(fontName,mGraphicStack.GetCurrentState().mFontSize);
	
	// Now write the string using the text command
	OutputStringBufferStream stringStream;
	char formattingBuffer[5];
	StringOrDoubleList stringOrDoubleList;
	UShortListList::iterator itEncodedList = encodedCharachtersListsList.begin();
	UShortList::iterator itEncoded;

	if(writeAsCID)
	{
		for(it = inStringsAndSpacing.begin(); it != inStringsAndSpacing.end(); ++it)
		{
			if(it->IsDouble)
			{
				stringOrDoubleList.push_back(StringOrDouble(it->DoubleValue));
			}
			else
			{
				for(itEncoded = itEncodedList->begin();itEncoded!= itEncodedList->end();++itEncoded)
				{
					formattingBuffer[0] = ((*itEncoded) >> 8) & 0x00ff;
					stringStream.Write((const Byte*)formattingBuffer, 1);
					formattingBuffer[0] = (*itEncoded) & 0x00ff;
					stringStream.Write((const Byte*)formattingBuffer, 1);					
				}
				stringOrDoubleList.push_back(StringOrDouble(stringStream.ToString()));
				stringStream.Reset();
				++itEncodedList;
			}
		}
		TJHexLow(stringOrDoubleList);
	}
	else
	{

		for(it = inStringsAndSpacing.begin(); it != inStringsAndSpacing.end(); ++it)
		{
			if(it->IsDouble)
			{
				stringOrDoubleList.push_back(StringOrDouble(it->DoubleValue));
			}
			else
			{
				for(itEncoded = itEncodedList->begin();itEncoded!= itEncodedList->end();++itEncoded)
				{
					formattingBuffer[0] = (*itEncoded) & 0x00ff;
					stringStream.Write((const Byte*)formattingBuffer,1);
				}
				stringOrDoubleList.push_back(StringOrDouble(stringStream.ToString()));
				stringStream.Reset();
				++itEncodedList;
			}
		}
		TJLow(stringOrDoubleList);
	}
	return PDFHummus::eSuccess;	
}

void AbstractContentContext::WriteFreeCode(const std::string& inFreeCode)
{
    RenewStreamConnection();
    mPrimitiveWriter.GetWritingStream()->Write((const Byte*)(inFreeCode.c_str()),inFreeCode.length());
}
void AbstractContentContext::WriteFreeCode(IByteReader* inFreeCodeSource)
{
	RenewStreamConnection();
    
    OutputStreamTraits traits(mPrimitiveWriter.GetWritingStream());
    traits.CopyToOutputStream(inFreeCodeSource);
}

void AbstractContentContext::AddContentContextListener(IContentContextListener* inExtender)
{
    mListeners.insert(inExtender);
}

void AbstractContentContext::RemoveContentContextListener(IContentContextListener* inExtender)
{
    mListeners.erase(inExtender);
}

void AbstractContentContext::DrawRectangle(double inLeft,double inBottom,double inWidth,double inHeight,const GraphicOptions& inOptions)
{
	SetupColor(inOptions);
	if(inOptions.drawingType == eStroke)
		w(inOptions.strokeWidth);
	re(inLeft,inBottom,inWidth,inHeight);
	FinishPath(inOptions);
}

void AbstractContentContext::DrawSquare(double inLeft,double inBottom,double inEdge,const GraphicOptions& inOptions)
{
	DrawRectangle(inLeft,inBottom,inEdge,inEdge,inOptions);
}

void AbstractContentContext::DrawCircle(double inCenterX,double inCenterY,double inRadius,const GraphicOptions& inOptions)
{
    const double magic = 0.551784;
    double x = inCenterX;
    double y = inCenterY;
    double r = inRadius;
    double rmagic = r * magic;
    
	SetupColor(inOptions);
	if(inOptions.drawingType == eStroke)
		w(inOptions.strokeWidth);
    m(x-r,y);
    c(x-r,y+rmagic,x-rmagic,y+r,x,y+r);
    c(x+rmagic,y+r,x+r,y+rmagic,x+r,y);
    c(x+r,y-rmagic,x+rmagic,y-r,x,y-r);
    c(x-rmagic,y-r,x-r,y-rmagic,x-r,y);
	FinishPath(inOptions);
}

void AbstractContentContext::DrawPath(const DoubleAndDoublePairList& inPathPoints,const GraphicOptions& inOptions)
{
	if(inPathPoints.size() == 0)
		return;

	SetupColor(inOptions);
	if(inOptions.drawingType == eStroke)
		w(inOptions.strokeWidth);

	DoubleAndDoublePairList::const_iterator it = inPathPoints.begin();
	m(it->first,it->second);
	++it;
	for(;it!=inPathPoints.end();++it)
		l(it->first,it->second);
	FinishPath(inOptions);
}

void AbstractContentContext::SetupColor(const GraphicOptions& inOptions)
{
	SetupColor(inOptions.drawingType,inOptions.colorValue,inOptions.colorSpace, inOptions.opacity);
}

void AbstractContentContext::SetupColor(const TextOptions& inOptions)
{
	SetupColor(eFill,inOptions.colorValue,inOptions.colorSpace, inOptions.opacity);
}


void AbstractContentContext::SetupColor(EDrawingType inDrawingType,unsigned long inColorValue,EColorSpace inColorSpace, double inOpacity)
{
	if(inDrawingType != eStroke &&
		inDrawingType != eFill)
		return;

	SetOpacity(inOpacity);
	switch(inColorSpace)
	{
		case eRGB:
			{
				double r = (unsigned char)((inColorValue >> 16) & 0xFF);
				double g = (unsigned char)((inColorValue >> 8) & 0xFF);
				double b = (unsigned char)(inColorValue & 0xFF);

				if(inDrawingType == eStroke)
					RG(r/255,g/255,b/255);
				else
					rg(r/255,g/255,b/255);
				break;
			}
		case eCMYK:
			{
                double c = (unsigned char)((inColorValue >> 24) & 0xFF);
                double m = (unsigned char)((inColorValue >> 16) & 0xFF);
                double y = (unsigned char)((inColorValue >> 8) & 0xFF);
                double kValue = (unsigned char)(inColorValue & 0xFF);
                
                if(inDrawingType == eStroke)
                    K(c/255,m/255,y/255,kValue/255);
                else
                    k(c/255,m/255,y/255,kValue/255);
				break;
			}
		case eGray:
			{
				double gValue = (unsigned char)(inColorValue & 0xFF);
                
                if(inDrawingType == eStroke)
                    G(gValue/255);
                else
                    g(gValue/255);
				break;
			}
	}
}

void AbstractContentContext::FinishPath(const GraphicOptions& inOptions)
{
	switch(inOptions.drawingType)
	{
		case eStroke:
			{
				if(inOptions.close)
					s();
				else
					S();
				break;
			}
		case eFill:
			{
				f();
				break;
			}
		case eClip:
			{
				if(inOptions.close)
					h();
				W();
				break;
			}

	}
}

EStatusCode AbstractContentContext::EncodeWithCurrentFont(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping) {
	PDFUsedFont* currentFont = GetCurrentFont();
	if(!currentFont)
	{
		TRACE_LOG("AbstractContentContext::EncodeWithCurrentFont, Cannot write text, no current font is defined");
		return PDFHummus::eFailure;
	}

	EStatusCode encodingStatus = currentFont->TranslateStringToGlyphs(inText,outGlyphsUnicodeMapping);

	// encoding returns false if was unable to encode some of the glyphs. will display as missing characters
	if(encodingStatus != PDFHummus::eSuccess)
		TRACE_LOG("AbstractContextContext::EncodeWithCurrentFont, was unable to find glyphs for all characters, some will appear as missing");	

	return encodingStatus;
}

void AbstractContentContext::WriteText(double inX,double inY,const std::string& inText,const TextOptions& inOptions)
{
	if(inOptions.font)
	{
		SetCurrentFont(inOptions.font);
	}

	GlyphUnicodeMappingList glyphsAndUnicode;	
	EncodeWithCurrentFont(inText, glyphsAndUnicode);

	SimpleGlyphsDrawingContext sharedDrawingContext(this, inOptions);
	LayeredGlyphsDrawingContext layeredGlyphDrawing(this, inOptions);
	PaintedGlyphsDrawingContext paintedGlyphDrawing(this, inOptions);

	double x = inX;

	/**
	 * The following draws text using simple Bt..tj..Et sequances while
	 * stopping for glyphs that require special drawing (colorful emojis are an example).
	 * the code uses a shared context for simple text drawing to join such simple
	 * glyphs togather for compacting the drawing code.
	 */

	sharedDrawingContext.StartWriting(x,inY);

	GlyphUnicodeMappingList::iterator it = glyphsAndUnicode.begin();
	for(; it!=glyphsAndUnicode.end(); ++it) {

		// attempt to draw with ColrV1 method
		paintedGlyphDrawing.SetGlyph(*it);
		if(paintedGlyphDrawing.CanDraw()) {
			// stop simple shared context
			sharedDrawingContext.Flush(true);
			x+=sharedDrawingContext.GetLatestAdvance();

			// draw with specialized method
			paintedGlyphDrawing.Draw(x,inY, true);
			x+=paintedGlyphDrawing.GetLatestAdvance();

			// continue simple shared context
			sharedDrawingContext.StartWriting(x,inY);		

			continue;	
		}

		// attempt to draw with ColrV0 method
		layeredGlyphDrawing.SetGlyph(*it);
		if(layeredGlyphDrawing.CanDraw()) {
			// stop simple shared context
			sharedDrawingContext.Flush(true);
			x+=sharedDrawingContext.GetLatestAdvance();

			// draw with specialized method
			layeredGlyphDrawing.Draw(x,inY, true);
			x+=layeredGlyphDrawing.GetLatestAdvance();

			// continue simple shared context
			sharedDrawingContext.StartWriting(x,inY);		

			continue;	
		}

		// ok. not a special glyph. just add to shared context
		sharedDrawingContext.AddGlyphMapping(*it);
	}

	// flush what glyphs are remaining in shared context
	sharedDrawingContext.Flush();
}

void AbstractContentContext::DrawImage(double inX,double inY,const std::string& inImagePath,const ImageOptions& inOptions)
{
	double transformation[6] = {1,0,0,1,0,0};

	if(inOptions.transformationMethod == eMatrix)
	{
		for(int i=0;i<6;++i)
			transformation[i] = inOptions.matrix[i];
	}
	else if(inOptions.transformationMethod == eFit)
	{
		DoubleAndDoublePair imageDimensions = mDocumentContext->GetImageDimensions(inImagePath,inOptions.imageIndex,inOptions.pdfParsingOptions);

        double scaleX = 1;
        double scaleY = 1;
                    
        if(inOptions.fitPolicy == eAlways)
        {
            scaleX = inOptions.boundingBoxWidth / imageDimensions.first;
            scaleY = inOptions.boundingBoxHeight / imageDimensions.second;
                        

        }
        else if(imageDimensions.first > inOptions.boundingBoxWidth || imageDimensions.second > inOptions.boundingBoxHeight) // overflow
        {
            scaleX = imageDimensions.first > inOptions.boundingBoxWidth ? inOptions.boundingBoxWidth / imageDimensions.first : 1;
            scaleY = imageDimensions.second > inOptions.boundingBoxHeight ? inOptions.boundingBoxHeight / imageDimensions.second : 1;
        }
                    
        if(inOptions.fitProportional)
        {
            scaleX = std::min(scaleX,scaleY);
            scaleY = scaleX;
        }
                    
        transformation[0] = scaleX;
        transformation[3] = scaleY;
	}

	transformation[4]+=inX;
	transformation[5]+=inY;

    // registering the images at pdfwriter to allow optimization on image writes
    ObjectIDTypeAndBool result = mDocumentContext->RegisterImageForDrawing(inImagePath,inOptions.imageIndex);
    if(result.second)
    {
        // if first usage, write the image
        ScheduleImageWrite(inImagePath,inOptions.imageIndex,result.first,inOptions.pdfParsingOptions);
    }
    
    q();
    cm(transformation[0],transformation[1],transformation[2],transformation[3],transformation[4],transformation[5]);
    Do(GetResourcesDictionary()->AddFormXObjectMapping(result.first));
    Q();

}

void AbstractContentContext::SetOpacity(double inAlpha) {
	if(inAlpha == NO_OPACITY_VALUE) // special value allowing to fallback on the current graphic state opacity value. this allows a default behavior for high level text commands
		return;

    // registering the images at pdfwriter to allow optimization on image writes
    ObjectIDTypeAndBool result = mDocumentContext->GetExtGStateRegistry().RegisterExtGStateForOpacity(inAlpha);
    if(result.second)
    {
        // if first usage, write the extgstate
		ScheduleObjectEndWriteTask(mDocumentContext->GetExtGStateRegistry().CreateExtGStateForOpacityWritingTask(result.first, inAlpha));
    }
	gs(GetResourcesDictionary()->AddExtGStateMapping(result.first));
}
