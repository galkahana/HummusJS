/*
   Source File : DocumentContextExtenderAdapter.h


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

#include "IDocumentContextExtender.h"

// adapter class for IDocumentContextExtender interface.
// note that you can't create it, just derive from it.

class DocumentContextExtenderAdapter: public IDocumentContextExtender
{
public:

	// add items to the page dictionary while it's written
	virtual PDFHummus::EStatusCode OnPageWrite(
							PDFPage* inPage,
							DictionaryContext* inPageDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext){
          (void) inPage;
	  (void) inPageDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  return PDFHummus::eSuccess;}

	// add items to the resources dictionary while it's written (can be either page or xobject resources dictionary)
	virtual PDFHummus::EStatusCode OnResourcesWrite(
							ResourcesDictionary* inResources,
							DictionaryContext* inPageResourcesDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext){
	  (void) inResources;
	  (void) inPageResourcesDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  return PDFHummus::eSuccess;}

	// add items to a particular resource dictionary (will be called from all but procset array and xobjects dict)
	virtual PDFHummus::EStatusCode OnResourceDictionaryWrite(
							DictionaryContext* inResourceDictionary,
							const std::string& inResourceDictionaryName,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext){
	  (void) inResourceDictionary;
	  (void) inResourceDictionaryName;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  return PDFHummus::eSuccess;}

	// add items to the form dictionary while it's written
	virtual PDFHummus::EStatusCode OnFormXObjectWrite(
							ObjectIDType inFormXObjectID,
							ObjectIDType inFormXObjectResourcesDictionaryID,
							DictionaryContext* inFormDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext){
	  (void) inFormXObjectID;
	  (void) inFormXObjectResourcesDictionaryID;
	  (void) inFormDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  return PDFHummus::eSuccess;}

	// add items to the image dictionary while it's written
	virtual PDFHummus::EStatusCode OnJPEGImageXObjectWrite(
							ObjectIDType inImageXObjectID,
							DictionaryContext* inImageDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							JPEGImageHandler* inJPGImageHandler){
	  (void) inImageXObjectID;
	  (void) inImageDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inJPGImageHandler;
	  return PDFHummus::eSuccess;}

	// add items to the image dictionary while it's writtern for a TIFF image (for tile images there are multiple such images)
	virtual PDFHummus::EStatusCode OnTIFFImageXObjectWrite(
							ObjectIDType inImageXObjectID,
							DictionaryContext* inImageDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							TIFFImageHandler* inTIFFImageHandler){
	  (void) inImageXObjectID;
	  (void) inImageDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inTIFFImageHandler;
	  return PDFHummus::eSuccess;}


	// add items to catalog dictionary while it's written
	virtual PDFHummus::EStatusCode OnCatalogWrite(
							CatalogInformation* inCatalogInformation,
							DictionaryContext* inCatalogDictionaryContext,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext){
	  (void) inCatalogInformation;
	  (void) inCatalogDictionaryContext;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  return PDFHummus::eSuccess;}

	// pdf copying events

	// When using any embedding method - Parsing of PDF to merge is not complete, before starting any merging
	virtual PDFHummus::EStatusCode OnPDFParsingComplete(
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When creating XObjects from pages - before creating a particular page xobject
	virtual PDFHummus::EStatusCode OnBeforeCreateXObjectFromPage(
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When creating XObjects from pages - after creating a particular page xobject
	virtual PDFHummus::EStatusCode OnAfterCreateXObjectFromPage(
							PDFFormXObject* iPageObjectResultXObject,
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) iPageObjectResultXObject;
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When appending pages from PDF - before appending a particular page
	virtual PDFHummus::EStatusCode OnBeforeCreatePageFromPage(
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}


	// When appending pages from PDF - after appending a particular page
	virtual PDFHummus::EStatusCode OnAfterCreatePageFromPage(
							PDFPage* iPageObjectResultPage,
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) iPageObjectResultPage;
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When merging pages from PDF - before merging a particular page
	virtual PDFHummus::EStatusCode OnBeforeMergePageFromPage(
							PDFPage* inTargetPage,
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inTargetPage;
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When merging pages from PDF - after merging a particular page
	virtual PDFHummus::EStatusCode OnAfterMergePageFromPage(
							PDFPage* inTargetPage,
							PDFDictionary* inPageObjectDictionary,
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inTargetPage;
	  (void) inPageObjectDictionary;
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}

	// When using any embedding method - right after embedding of the PDF is complete
	virtual PDFHummus::EStatusCode OnPDFCopyingComplete(
							ObjectsContext* inPDFWriterObjectContext,
							PDFHummus::DocumentContext* inDocumentContext,
							PDFDocumentHandler* inPDFDocumentHandler){
	  (void) inPDFWriterObjectContext;
	  (void) inDocumentContext;
	  (void) inPDFDocumentHandler;
	  return PDFHummus::eSuccess;}
    
    // When modifying a PDF, asking extender whether catalog update is desirable
    virtual bool IsCatalogUpdateRequiredForModifiedFile(PDFParser* inModifiderFileParser){
      (void) inModifiderFileParser;
      return false;}


protected:
	DocumentContextExtenderAdapter(){}

};
