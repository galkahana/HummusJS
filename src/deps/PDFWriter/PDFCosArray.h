///////////////////////////////////////////////////////////////////////////////
// PDFCosArray.h    Stefan Woerthmueller    2014       StefanWoe@googlemail.com
///////////////////////////////////////////////////////////////////////////////
/*  Usage Examples

    1) Named C++ Object 
    PDFCosArray ColorMaskArr(mPdf);
        ColorMaskArr.WriteInteger(0);
        ColorMaskArr.WriteInteger(0);
        ColorMaskArr.WriteInteger(0xff).WriteInteger(0xff);  
    ColorMaskArr.End();

    Reference this later with ColorMaskArr.ID();

    2) You can use a unnamed C++ auto Object to insert a "inline" array
    PDFCosArray(currentDictName, "Coords").
            WriteInteger(0).WriteInteger(y1 + 10).
            WriteInteger(0).WriteInteger(y2 - 10);


*/
#pragma once
#include "PDFWriter.h"
#include "DictionaryContext.h"

class PDFCosDict;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PDFCosArray

{
public:
    PDFCosArray(PDFWriter &parentDoc, std::string name = std::string()) : 
      m_Name(name), 
      m_DocumentContext(parentDoc.GetObjectsContext()),
      m_DidEnd(false)
    {
        m_ObjID           = m_DocumentContext.StartNewIndirectObject();
        m_DocumentContext.StartArray();
    }

    PDFCosArray(PDFCosDict &parentDict, std::string name = std::string());

    PDFCosArray(ObjectsContext &docContect, ObjectIDType newID = 0) : 
      m_DocumentContext(docContect),
      m_DidEnd(false)
    {
        if(newID)
            m_DocumentContext.StartNewIndirectObject(m_ObjID = newID);
        else
            m_ObjID  = m_DocumentContext.StartNewIndirectObject();

        m_DocumentContext.StartArray();
    }
    ~PDFCosArray()
    {
        if(! m_DidEnd)
            End();
    }
    
    void End()
    {
        m_DocumentContext.EndArray();
        m_DocumentContext.EndLine();
        if(m_ObjID)
            m_DocumentContext.EndIndirectObject();
        m_DidEnd = true;
    }

    PDFCosArray& AddObjectRef(const ObjectIDType &ID)
    {
        m_DocumentContext.WriteNewIndirectObjectReference(ID);
        return *this;
    }
    PDFCosArray& operator+=(const PDFCosDict &rhs);

    PDFCosArray& operator+=(float f)
    {
        m_DocumentContext.WriteDouble(f);
        return *this;
    }
    PDFCosArray& operator+=(int i)
    {
        m_DocumentContext.WriteInteger(i);
        return *this;
    }
    PDFCosArray& AddName(const std::string value)
    {
        m_DocumentContext.WriteName(value);
        return *this;
    }
    PDFCosArray& AddString(const std::string value)
    {
        m_DocumentContext.WriteLiteralString(value);
        return *this;
    }
    PDFCosArray& WriteDouble(float f)
    {
        m_DocumentContext.WriteDouble(f);
        return *this;
    }
    PDFCosArray& WriteInteger(int i)
    {
        m_DocumentContext.WriteInteger(i);
        return *this;
    }
    PDFCosArray& WriteBool(bool i)
    {
        m_DocumentContext.WriteBoolean(i);
        return *this;
    }

    ObjectIDType       ID() {return m_ObjID;}

private:
    friend class PDFCosDict;
    std::string        m_Name;
    ObjectsContext&    m_DocumentContext;
    bool               m_DidEnd;
    ObjectIDType       m_ObjID;

};


