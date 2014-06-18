///////////////////////////////////////////////////////////////////////////////
// PDFArrayIterator.h       Stefan Woerthmueller 2014  StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "PDFWriter.h"
#include "PDFObjectCast.h"
#include "PDFArray.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class PDFDictionaryIterator;

class PDFArrayIterator
{
public:
    PDFArrayIterator(PDFParser &parser) :
                        mParser(parser)
    {
    }
    PDFArrayIterator(PDFParser &parser, PDFObjectCastPtr<PDFArray> &inArray) :
                        mArray(inArray),
                        mParser(parser)
    {
    }

    PDFDictionaryIterator DictAt(int ndx);
    unsigned long         GetLength();

private:
    PDFObjectCastPtr<PDFArray> mArray;
    PDFParser &mParser;
};
