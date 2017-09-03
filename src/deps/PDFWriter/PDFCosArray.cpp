///////////////////////////////////////////////////////////////////////////////
// PDFCosArray.cpp  Stefan Woerthmueller    2014       StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////

#include "PDFCosArray.h"
#include "PDFCosDict.h"

PDFCosArray::PDFCosArray(PDFCosDict &parentDict, std::string name) : 
    m_Name(name), 
    m_DocumentContext(parentDict.m_DocumentContext),
    m_DidEnd(false),
    m_ObjID(0)
{
    if(! m_Name.empty())
        parentDict.m_DictonaryContext->WriteKey(m_Name);
    m_DocumentContext.StartArray();
}

PDFCosArray& PDFCosArray::operator+=(const PDFCosDict &rhs)
{
    m_DocumentContext.WriteNewIndirectObjectReference(rhs.m_ObjID);
    return *this;
}







