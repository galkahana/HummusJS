/*
   Source File : PDFUsedFont.h


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

#include "FreeTypeFaceWrapper.h"
#include "ObjectsBasicTypes.h"
#include "EStatusCode.h"
#include "GlyphUnicodeMapping.h"
#include <string>
#include <list>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H



typedef std::list<unsigned short> UShortList;
typedef std::list<UShortList> UShortListList;
typedef std::list<std::string> StringList;
typedef std::list<GlyphUnicodeMappingList> GlyphUnicodeMappingListList;
typedef std::list<unsigned int> UIntList;

class IWrittenFont;
class ObjectsContext;
class PDFParser;

class PDFUsedFont
{
public:

	struct TextMeasures
	{
		double xMin;
		double yMin;
		double xMax;
		double yMax;
		double width;
		double height;
	};

	class IOutlineEnumerator : private FreeTypeFaceWrapper::IOutlineEnumerator {
		friend class PDFUsedFont;
	public:
		IOutlineEnumerator(double base_x, double base_y);
		virtual ~IOutlineEnumerator(){};

	protected:
		virtual bool Moveto(double x, double y)=0;
		virtual bool Lineto(double x, double y)=0;
		virtual bool Curveto(double x1, double y1, double x2, double y2, double x3, double y3)=0;
		virtual bool Closepath()=0;

	private:
		void BeginEnum(double scale);
		void MoveBasepoint(double dx, double dy);

		virtual bool Moveto(FT_Short x, FT_Short y);
		virtual bool Lineto(FT_Short x, FT_Short y);
		virtual bool Curveto(FT_Short x1, FT_Short y1, FT_Short x2, FT_Short y2, FT_Short x3, FT_Short y3);
		virtual bool Close();

		double mBase_x, mBase_y;
		double mFontScale;
	};

	PDFUsedFont(FT_Face inInputFace,
				const std::string& inFontFilePath,
				const std::string& inAdditionalMetricsFontFilePath,
                long inFontIndex,
				ObjectsContext* inObjectsContext,
				bool inEmbedFont);
	virtual ~PDFUsedFont(void);

	bool IsValid();

	// a String in the following two implementations is represented here by a list of glyphs, with each mapped to the matching
	// unicode values. to move from a wide string to such a structure, use other class methods

	/*
		This function does the work of encoding a text string to a matching font instance name, and an encoded array
		of characters. the encoded array is a list of short values (double byte for CID, one byte for regular), and an extra
		boolean value indicates whether they are CID or regular.
	*/
	PDFHummus::EStatusCode EncodeStringForShowing(const GlyphUnicodeMappingList& inText,
										ObjectIDType &outFontObjectToUse,
										UShortList& outCharactersToUse,
										bool& outTreatCharactersAsCID);

	// encode all strings. make sure that they will use the same font.
	PDFHummus::EStatusCode EncodeStringsForShowing(const GlyphUnicodeMappingListList& inText,
										ObjectIDType &outFontObjectToUse,
										UShortListList& outCharactersToUse,
										bool& outTreatCharactersAsCID);

	PDFHummus::EStatusCode WriteFontDefinition();

	// use this method to translate text to glyphs and unicode mapping, to be later used for EncodeStringForShowing
	PDFHummus::EStatusCode TranslateStringToGlyphs(const std::string& inText,GlyphUnicodeMappingList& outGlyphsUnicodeMapping);

	PDFHummus::EStatusCode WriteState(ObjectsContext* inStateWriter,ObjectIDType inObjectID);
	PDFHummus::EStatusCode ReadState(PDFParser* inStateReader,ObjectIDType inObjectID);
    
    FreeTypeFaceWrapper* GetFreeTypeFont();

	// text measurements, either pass unicode text or glyphs list
	PDFUsedFont::TextMeasures CalculateTextDimensions(const std::string& inText,long inFontSize=1);
	PDFUsedFont::TextMeasures CalculateTextDimensions(const UIntList& inGlyphsList,long inFontSize=1);
	double CalculateTextAdvance(const std::string& inText,double inFontSize=1);
	double CalculateTextAdvance(const UIntList& inGlyphsList,double inFontSize=1);

	// character path enumeration, pass unicode text or glyph list
	bool EnumeratePaths(IOutlineEnumerator& target, const std::string& inText,double inFontSize=1);
	bool EnumeratePaths(IOutlineEnumerator& target, const UIntList& inGlyphsList,double inFontSize=1);

protected:
	void GetUnicodeGlyphs(const std::string& inText, UIntList& glyphs);

private:
	static const unsigned int AdvanceCacheLimit = 200;
	FreeTypeFaceWrapper mFaceWrapper;
    IWrittenFont* mWrittenFont;
	ObjectsContext* mObjectsContext;
	std::map<unsigned int, FT_Pos> mAdvanceCache;
	bool mEmbedFont;


};
