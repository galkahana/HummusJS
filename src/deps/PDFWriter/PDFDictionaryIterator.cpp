///////////////////////////////////////////////////////////////////////////////
// PDFDictionaryIterator.cpp Stefan Woerthmueller 2014  StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////

#include "PDFDictionaryIterator.h"
#include "PDFArrayIterator.h"

#include <iostream>
#include <ostream>
#include <fstream>


PDFArrayIterator PDFDictionaryIterator::Array(std::string s)
{
    if(! mDictonary)
        return PDFArrayIterator(mParser);

    PDFObjectCastPtr<PDFArray> foundArray(mDictonary->QueryDirectObject(s));
	if(foundArray != NULL)
        return PDFArrayIterator(mParser, foundArray);
    return PDFArrayIterator(mParser);
}

PDFDictionaryIterator PDFDictionaryIterator::Enter(std::string s)
{
    if(! mDictonary)
        return PDFDictionaryIterator(mParser);
    PDFObjectCastPtr<PDFIndirectObjectReference> foundReference(mDictonary->QueryDirectObject(s));
	if(!foundReference)
        return PDFDictionaryIterator(mParser);

	PDFObjectCastPtr<PDFDictionary> catalog(mParser.ParseNewObject(foundReference->mObjectID));
	if(!catalog)
        return PDFDictionaryIterator(mParser);

    return PDFDictionaryIterator(mParser, catalog);
}

std::string PDFDictionaryIterator::GetStrValue(std::string s)
{
    if(! mDictonary)
        return std::string();
    PDFObjectCastPtr<PDFLiteralString> foundReference(mDictonary->QueryDirectObject(s));
	if(!foundReference)
        return std::string();
    return foundReference->GetValue();
}

bool PDFDictionaryIterator::WriteStreamToFile(InputFile &pdfFile, std::string s, std::string filePath)
{
    if(! mDictonary)
        return false;
    PDFObjectCastPtr<PDFIndirectObjectReference> foundReference(mDictonary->QueryDirectObject(s));
	if(!foundReference)
        return false;

    PDFObjectCastPtr<PDFStreamInput> foundStreamInput(mParser.ParseNewObject(foundReference->mObjectID));
	if(!foundStreamInput)
        return false;


    std::ofstream myFile;
    myFile.open(filePath.c_str(), std::ios_base::trunc | std::ios_base::binary);
    if(! myFile.is_open())
        return false;
        
    IByteReader* streamReader = mParser.CreateInputStreamReader(foundStreamInput.GetPtr());
    if(! streamReader)
        return false;

    Byte buffer[0xffff];
    if(streamReader)
    {
        pdfFile.GetInputStream()->SetPosition(foundStreamInput->GetStreamContentStart());
        while(streamReader->NotEnded())
        {
            LongBufferSizeType readAmount = streamReader->Read(buffer,sizeof(buffer));
            myFile.write((const char*)buffer,readAmount);
        }
    }
    delete streamReader;

    return true;
}


