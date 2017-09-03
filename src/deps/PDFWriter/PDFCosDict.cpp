///////////////////////////////////////////////////////////////////////////////
// PDFCosDict.cpp   Stefan Woerthmueller    2014       StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////
#include "PDFCosDict.h"
#include "PDFCosArray.h"

                                               
PDFCosDict::PDFCosDict(PDFCosDict &parentDict, std::string name) : 
    m_Name(name), 
    m_DocumentContext(parentDict.m_DocumentContext),
    m_DidEnd(false),
    m_ObjID(0)
{
	parentDict.m_DictonaryContext->WriteKey(m_Name);
	m_DictonaryContext = m_DocumentContext.StartDictionary();
}
   
PDFCosDict::PDFCosDict(PDFCosArray &parentArr) : 
    m_DocumentContext(parentArr.m_DocumentContext), 
    m_DidEnd(false),
    m_ObjID(0)
{
	m_DictonaryContext = m_DocumentContext.StartDictionary();
}



PDFCosDict& PDFCosDict::operator+=(const PDFCosArray &rhs)
{
	m_DictonaryContext->WriteKey(rhs.m_Name);
    m_DictonaryContext->WriteObjectReferenceValue(rhs.m_ObjID);
    return *this;
}







