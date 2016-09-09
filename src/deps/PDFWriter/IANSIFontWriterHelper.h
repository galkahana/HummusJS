/*
   Source File : IANSIFontWriterHelper.h


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

class DictionaryContext;
class IFontDescriptorHelper;

class IANSIFontWriterHelper
{
public:
	virtual ~IANSIFontWriterHelper(){}

	// write the sub type value (only, key is already written at this point) for the particular font into the dictionary.
	virtual void WriteSubTypeValue(DictionaryContext* inDictionary) = 0;

	// Get the relevant CharSet entry writer for the descriptor writing. 
	// always return something - even if doing nothing
	virtual IFontDescriptorHelper* GetCharsetWriter() = 0;

	// Flag for possibility of having differences from win ansi.
	// basically true type can't, and others - type 1 and CFF can. true type has the pacularty
	// of not having dependable glyph names, but using index always...so can't have ansi if there are differences anyways
	virtual bool CanWriteDifferencesFromWinAnsi() = 0;
};