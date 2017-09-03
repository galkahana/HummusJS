///////////////////////////////////////////////////////////////////////////////
// PDFCosDict.h     Stefan Woerthmueller    2014       StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////
/*  Usage Examples

    1) Add a Simple Dictionary
    PDFCosDict  FunctionDict(mPdf, "Function");
        FunctionDict.Add("FunctionType", 2); 
        FunctionDict.Add("N", 1);
    FunctionDict.End();


    2) Add a Dictionary Containing a Array
    PDFCosDict  ShadingDict(mPdf, std::string(), myid);
        ShadingDict.AddName("ColorSpace", "DeviceRGB");
        
        // You can use a unnamed C++ auto Object to insert a "inline" array
        PDFCosArray(ShadingDict, "Coords").
                WriteInteger(0).WriteInteger(y1 + 10).
                WriteInteger(0).WriteInteger(y2 - 10);

        ShadingDict += FunctionDict;            // See Above
        ShadingDict.Add("ShadingType", 2); 
    ShadingDict.End();


    Then i.e. in the DocumentContextExtenderAdapter you write
	inPageResourcesDictionaryContext->WriteKey("myKey");
	inPageResourcesDictionaryContext->WriteObjectReferenceValue(ShadingDict.ID());

*/

#pragma once
#include "PDFWriter.h"
#include "DictionaryContext.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class PDFCosArray;

class PDFCosDict

{
public:
    PDFCosDict(PDFWriter &parentDoc, std::string name = std::string(), ObjectIDType newID = 0) : 
      m_Name(name), 
      m_DocumentContext(parentDoc.GetObjectsContext()),
      m_DidEnd(false)
    {
        if(newID)
            m_DocumentContext.StartNewIndirectObject(m_ObjID = newID);
        else
           m_ObjID              = m_DocumentContext.StartNewIndirectObject();
	    m_DictonaryContext      = m_DocumentContext.StartDictionary();

    }
    PDFCosDict(ObjectsContext &docContect, std::string name = std::string()) : 
      m_Name(name), 
      m_DocumentContext(docContect),
      m_DidEnd(false)
    {
        m_ObjID                 = m_DocumentContext.StartNewIndirectObject();
	    m_DictonaryContext      = m_DocumentContext.StartDictionary();

    }
    

    PDFCosDict(PDFCosDict &parentDict, std::string name);
    PDFCosDict(PDFCosArray &parentArr);

   ~PDFCosDict()
    {
        if(! m_DidEnd)
            End();
    }

    void End()
    {
		m_DocumentContext.EndDictionary(m_DictonaryContext);
        if(m_ObjID)
		    m_DocumentContext.EndIndirectObject();
        m_DictonaryContext = NULL;
        m_DidEnd = true;
    }

    void SkipEnd()
    {
        m_DidEnd = true;
    }

    PDFCosDict& operator+=(const PDFCosDict &rhs)
    {
	    m_DictonaryContext->WriteKey(rhs.m_Name);
        m_DictonaryContext->WriteObjectReferenceValue(rhs.m_ObjID);
        return *this;
    }
    PDFCosDict& operator+=(const PDFCosArray &rhs);

    PDFCosDict& Add(std::string key, const PDFCosDict &rhs)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteObjectReferenceValue(rhs.m_ObjID);
        return *this;
    }
    PDFCosDict& AddName(std::string key, std::string value)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteNameValue(value);
        return *this;
    }
    PDFCosDict& AddString(std::string key, std::string value)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteLiteralStringValue(value);
        return *this;
    }

    PDFCosDict& AddObjectRef(std::string key, const ObjectIDType &ID)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteObjectReferenceValue(ID);
        return *this;
    }
    PDFCosDict& AddNewObjectRef(std::string key, const ObjectIDType &ID)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteNewObjectReferenceValue(ID);
        return *this;
    }
    PDFCosDict& Add(std::string key, int i)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteIntegerValue(i);
        return *this;
    }
    PDFCosDict& Add(std::string key, double i)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteDoubleValue(i);
        return *this;
    }

    PDFCosDict& AddBool(std::string key, bool i)
    {
	    m_DictonaryContext->WriteKey(key);
        m_DictonaryContext->WriteBooleanValue(i);
        return *this;
    }


    PDFCosDict& WriteKey(std::string key)
    {
	    m_DictonaryContext->WriteKey(key);
        return *this;
    }
        

    ObjectIDType       ID() {return m_ObjID;}
    DictionaryContext* DictonaryContext() {return m_DictonaryContext;}


private:
    friend class PDFCosArray;
    std::string        m_Name;
    ObjectsContext&    m_DocumentContext;
    bool               m_DidEnd;
    DictionaryContext* m_DictonaryContext;
    ObjectIDType       m_ObjID;

};


