/*
   Source File : AbstractContentContext.h


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
#pragma once

/*
	abstract implementation for content context.
	writes functions, and function parameters.

	exculdes taking care of streams and the identity of the resource dictionary.

	use SetPDFStreamForWrite to setup the stream for write (may be using multiple times)
	Implement GetResourcesDictionary to return the relevant resources dictionary for the content
	Optionally Implement RenewStreamConnection, which is called before any operator is written, in order to make sure there's a stream there to write to

*/

#include "EStatusCode.h"
#include "PrimitiveObjectsWriter.h"
#include "GraphicStateStack.h"
#include "GlyphUnicodeMapping.h"
#include "ObjectsBasicTypes.h"
#include "PDFParsingOptions.h"
#include <string>
#include <list>
#include <set>
#include <utility>


namespace PDFHummus
{
    class DocumentContext;  
};
class ObjectsContext;
class PDFStream;
class ResourcesDictionary;
class PDFImageXObject;
class ITextCommand;
class IByteReader;
class IContentContextListener;
class IObjectEndWritingTask;

template <typename T>
struct SomethingOrDouble
{
	T SomeValue;
	double DoubleValue;

	bool IsDouble; // true - double, false - other

	SomethingOrDouble(T inSome){SomeValue = inSome;IsDouble = false;}
	SomethingOrDouble(double inDouble){DoubleValue = inDouble;IsDouble = true;}
};

typedef SomethingOrDouble<std::string> StringOrDouble;
typedef SomethingOrDouble<GlyphUnicodeMappingList> GlyphUnicodeMappingListOrDouble;

typedef std::list<StringOrDouble> StringOrDoubleList;
typedef std::list<GlyphUnicodeMappingListOrDouble> GlyphUnicodeMappingListOrDoubleList;

typedef std::set<IContentContextListener*> IContentContextListenerSet;
typedef std::pair<double,double> DoubleAndDoublePair;
typedef std::list<DoubleAndDoublePair> DoubleAndDoublePairList;

#define NO_OPACITY_VALUE 255.0

class AbstractContentContext
{
public:
	// graphic options struct for high level methods
	enum EDrawingType
	{
		eStroke,
		eFill,
		eClip
	};

	enum EColorSpace
	{
		eRGB,
		eCMYK,
		eGray
	};


	struct GraphicOptions
	{
		EDrawingType drawingType;
		EColorSpace colorSpace;
		unsigned long colorValue;
		double strokeWidth;
		bool close;
		double opacity;

		GraphicOptions(EDrawingType inDrawingType = eStroke,
						EColorSpace inColorSpace = eRGB,
						unsigned long inColorValue = 0,
						double inStrokeWidth = 1.0,
						bool inClose = false,
						double inOpacity = NO_OPACITY_VALUE)
		{
			drawingType = inDrawingType;
			colorSpace = inColorSpace;
			colorValue = inColorValue;
			strokeWidth = inStrokeWidth;
			close = inClose;
			opacity = inOpacity;
		}

	};

	struct TextOptions
	{
		EColorSpace colorSpace;
		unsigned long colorValue;
		double fontSize;
		PDFUsedFont* font;
		double opacity;

		TextOptions(PDFUsedFont* inFont,
					double inFontSize = 1,
					EColorSpace inColorSpace = eRGB,
					unsigned long inColorValue = 0,
					double inOpacity = NO_OPACITY_VALUE)
		{
			fontSize = inFontSize;
			colorSpace = inColorSpace;
			colorValue = inColorValue;
			font = inFont;
			opacity = inOpacity;
		}
	};
	

	enum EImageTransformation
	{
		eNone,
		eMatrix,
		eFit
	};

	enum EFitPolicy
	{
		eAlways,
		eOverflow
	};

	struct ImageOptions
	{
		EImageTransformation transformationMethod;
		unsigned long imageIndex;
		double matrix[6];
		double boundingBoxWidth;
		double boundingBoxHeight;
		bool fitProportional;
		EFitPolicy fitPolicy;
		PDFParsingOptions pdfParsingOptions;


		ImageOptions()
		{
			transformationMethod = eNone;
			imageIndex = 0;
			matrix[0] = matrix[3] = 1;
			matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0;
			boundingBoxWidth = 100;
			boundingBoxHeight = 100;
			fitProportional = false;
			fitPolicy = eOverflow;
		}

	};

	AbstractContentContext(PDFHummus::DocumentContext* inDocumentContext);
	virtual ~AbstractContentContext(void);

	// High level methods
	void DrawRectangle(double inLeft,double inBottom,double inWidth,double inHeight,const GraphicOptions& inOptions=GraphicOptions());
	void DrawSquare(double inLeft,double inBottom,double inEdge,const GraphicOptions& inOptions=GraphicOptions());
	void DrawCircle(double inCenterX,double inCenterY,double inRadius,const GraphicOptions& inOptions=GraphicOptions());
	void DrawPath(const DoubleAndDoublePairList& inPathPoints,const GraphicOptions& inOptions=GraphicOptions());
	void WriteText(double inX,double inY,const std::string& inText,const TextOptions& inOptions);
	static unsigned long ColorValueForName(const std::string& inColorName);
	void DrawImage(double inX,double inY,const std::string& inImagePath,const ImageOptions& inOptions=ImageOptions());


	// PDF Operators. For explanations on the meanings of each operator read Appendix A "Operator Summary" of the PDF Reference Manual (1.7)
	
	// path stroke/fill
	void b();
	void B();
	void bStar();
	void BStar();
	void s();
	void S();
	void f();
	void F();
	void fStar();
	void n();

	// path construction
	void m(double inX,double inY);
	void l(double inX,double inY);
	void c(	double inX1,double inY1, 
			double inX2, double inY2, 
			double inX3, double inY3);
	void v(	double inX2,double inY2, 
			double inX3, double inY3);
	void y(	double inX1,double inY1, 
			double inX3, double inY3);
	void h();
	void re(double inLeft,double inBottom, double inWidth,double inHeight);

	// graphic state
	void q();
	PDFHummus::EStatusCode Q(); // Status code returned, in case there's inbalance in "q-Q"s
	void cm(double inA, double inB, double inC, double inD, double inE, double inF);
	void w(double inLineWidth);
	void J(int inLineCapStyle);
	void j(int inLineJoinStyle);
	void M(double inMiterLimit);
	void d(double* inDashArray, int inDashArrayLength, double inDashPhase);
	void ri(const std::string& inRenderingIntentName);
	void i(int inFlatness);
	void gs(const std::string& inGraphicStateName);

	// color operators
	void CS(const std::string& inColorSpaceName);
	void cs(const std::string& inColorSpaceName);
	void SC(double* inColorComponents, int inColorComponentsLength);
	void SCN(double* inColorComponents, int inColorComponentsLength);
	void SCN(double* inColorComponents, int inColorComponentsLength,const std::string& inPatternName);
	void SCN(const std::string& inPatternName);
	void sc(double* inColorComponents, int inColorComponentsLength);
	void scn(double* inColorComponents, int inColorComponentsLength);
	void scn(double* inColorComponents, int inColorComponentsLength,const std::string& inPatternName);
	void scn(const std::string& inPatternName);
	void G(double inGray);
	void g(double inGray);
	void RG(double inR,double inG,double inB);
	void rg(double inR,double inG,double inB);
	void K(double inC,double inM,double inY,double inK);
	void k(double inC,double inM,double inY,double inK);

	// clip operators
	void W();
	void WStar();

	// XObject usage
	void Do(const std::string& inXObjectName);

	// Text state operators
	void Tc(double inCharacterSpace);
	void Tw(double inWordSpace);
	void Tz(int inHorizontalScaling);
	void TL(double inTextLeading);
	void Tr(int inRenderingMode);
	void Ts(double inFontRise);

	// Text object operators
	void BT();
	void ET();

	// Text positioning operators
	void Td(double inTx, double inTy);
	void TD(double inTx, double inTy);
	void Tm(double inA, double inB, double inC, double inD, double inE, double inF);
	void TStar();

	//
	// Text showing operators using the library handling of fonts with unicode text using UTF 16
	//

	// Set the font for later text writing command to the font in reference.
	// to create such a font use PDF Writer GetFontForFile.
	// Note that placing an actual Tf command (and including in resources dictionary) will
	// only occur when actually placing text.
	void Tf(PDFUsedFont* inFontReference,double inFontSize);

	// Tf is just setting current font and size, if you want to do those individually use 
	// either SetCurrentFont or SetCurrentFontSize
	void SetCurrentFont(PDFUsedFont* inFontReference);
	// you can also get the current font
	PDFUsedFont* GetCurrentFont();
	void SetCurrentFontSize(double inFontSize);

	

	// place text to the current set font with Tf
	// will return error if no font was set, or that one of the glyphs
	// didn't succeed in encoding.
	// input parameter is UTF-8 encoded
	PDFHummus::EStatusCode Tj(const std::string& inText);

	// The rest of the text operators, handled by the library handing of font. text is in UTF8
	PDFHummus::EStatusCode Quote(const std::string& inText);
	PDFHummus::EStatusCode DoubleQuote(double inWordSpacing, double inCharacterSpacing, const std::string& inText);
	PDFHummus::EStatusCode TJ(const StringOrDoubleList& inStringsAndSpacing); 

	//
	// Text showing operators using the library handling of fonts with direct glyph selection
	//
	
	// For Direct glyph selections (if you don't like my UTF16 encoding), use the following commands.
	// each command accepts a list of glyphs. each glyph is mapped to its matching unicode values. 
	// a glyph may have more than one unicode value in case it reperesents a series of Characters.

	PDFHummus::EStatusCode Tj(const GlyphUnicodeMappingList& inText);
	PDFHummus::EStatusCode Quote(const GlyphUnicodeMappingList& inText);
	PDFHummus::EStatusCode DoubleQuote(double inWordSpacing, double inCharacterSpacing, const GlyphUnicodeMappingList& inText);
	PDFHummus::EStatusCode TJ(const GlyphUnicodeMappingListOrDoubleList& inStringsAndSpacing); 

	//
	// Text showing operators overriding library behavior
	//

	// Low level text showing operators. use them only if you completly take chare of
	// font and text usage

	// Low level setting of font. for the high level version, see below
	void TfLow(const std::string& inFontName,double inFontSize); 

	// first version of Tj writes the string in literal string paranthesis, 
	// second version of Tj writes the string in hex string angle brackets
	void TjLow(const std::string& inText);
	void TjHexLow(const std::string& inText); 

	void QuoteLow(const std::string& inText); // matches the operator '
	void QuoteHexLow(const std::string& inText);

	void DoubleQuoteLow(double inWordSpacing, double inCharacterSpacing, const std::string& inText); // matches the operator "
	void DoubleQuoteHexLow(double inWordSpacing, double inCharacterSpacing, const std::string& inText); 

	// similar to the TJ PDF command, TJ() recieves an input an array of items which
	// can be either a string or a double
	void TJLow(const StringOrDoubleList& inStringsAndSpacing);
	void TJHexLow(const StringOrDoubleList& inStringsAndSpacing);

    // introduce free code
    void WriteFreeCode(const std::string& inFreeCode);
    void WriteFreeCode(IByteReader* inFreeCodeSource);

    // Extensibility
    void AddContentContextListener(IContentContextListener* inExtender);
    void RemoveContentContextListener(IContentContextListener* inExtender);

	// Simplified color setup
	void SetupColor(const GraphicOptions& inOptions);
	void SetupColor(const TextOptions& inOptions);
	void SetupColor(EDrawingType inDrawingType,unsigned long inColorValue,EColorSpace inColorSpace, double inOpacity);

	// Opacity. sets for both fill and stroke, for simplicity sake.
	// Alpha value is 0 to 1, where 1 is opaque and 0 is fully transparent.
	// 255.0 is a special value which skips setting an alpha value. can be used for calling always but with a default
	// behavior that falls back on current graphic state
	void SetOpacity(double inAlpha);

	// accessors for internal objects, for extending content context capabilities outside of content context
	PrimitiveObjectsWriter& GetPrimitiveWriter() {return mPrimitiveWriter;}
	PDFHummus::DocumentContext* GetDocumentContext() {return mDocumentContext;}


	// Derived classes should use this method to retrive the content resource dictionary, for updating procsets 'n such
	virtual ResourcesDictionary* GetResourcesDictionary() = 0;
	// Derived classes should implement this method for registering image writes
	virtual void ScheduleImageWrite(const std::string& inImagePath,unsigned long inImageIndex,ObjectIDType inObjectID,const PDFParsingOptions& inParsingOptions) = 0;
	// And a bit more generic version of the image write would be:
	virtual void ScheduleObjectEndWriteTask(IObjectEndWritingTask* inObjectEndWritingTask) = 0;

protected:

	PDFHummus::DocumentContext* mDocumentContext;

	// Derived classes should use this method to update the stream for writing
	void SetPDFStreamForWrite(PDFStream* inStream);

private:
	// Derived classes should optionally use this method if the stream needs updating (use calls to SetPDFStreamForWrite for this purpose)
	virtual void RenewStreamConnection() {};


	PrimitiveObjectsWriter mPrimitiveWriter;

	// graphic stack to monitor high-level graphic usage (now - fonts)
	GraphicStateStack mGraphicStack;
    
    // listeners
    IContentContextListenerSet mListeners;

	void AssertProcsetAvailable(const std::string& inProcsetName);

	PDFHummus::EStatusCode WriteTextCommandWithEncoding(const std::string& inUnicodeText,ITextCommand* inTextCommand);
	PDFHummus::EStatusCode WriteTextCommandWithDirectGlyphSelection(const GlyphUnicodeMappingList& inText,ITextCommand* inTextCommand);


	void FinishPath(const GraphicOptions& inOptions);

	PDFHummus::EStatusCode EncodeWithCurrentFont(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping);
};

