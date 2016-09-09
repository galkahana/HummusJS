/*
   Source File : CatalogInformation.h


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
	In building the page tree i'm using a balanced tree sort of algorithm.
	The tree lowest level is the only one holding actual pages ID. all levels above it are Page Tree containing Page Trees.
	The adding algorithm tries to add to the current held node. if unsuccesful asks from its parent to create a "brother" page tree.
	if brother page tree is created than the addition is made to it. if unsuccesful the task is carried out to a higher heirarchy, for a "cousin" to be created.
	then the addition is made on the cousin.

	this algorithm creates a balanced tree, and the mCurrentPageTreeNode of The CatalogInformation will always hold the "latest" low node.
	writing the page tree in the end simply goes up the page tree to the root and uses recursion to walk over the tree.

*/

#include "ObjectsBasicTypes.h"

class PageTree;
class IndirectObjectsReferenceRegistry;

class CatalogInformation
{
public:
	CatalogInformation(void);
	~CatalogInformation(void);

	ObjectIDType AddPageToPageTree(ObjectIDType inPageID,IndirectObjectsReferenceRegistry& inObjectsRegistry);

	// indirect objects registry is passed in case there's no page tree root...in which case it'll automatically create one
	PageTree* GetPageTreeRoot(IndirectObjectsReferenceRegistry& inObjectsRegistry);

	PageTree* GetCurrentPageTreeNode();
	
	void SetCurrentPageTreeNode(PageTree* inCurrentPageTreeNode);

	void Reset();
private:

	PageTree* mCurrentPageTreeNode;
};
