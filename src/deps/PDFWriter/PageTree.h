/*
   Source File : PageTree.h


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
	see comment in Catalog Information for details on the algorithm to build the page tree
*/


#include "ObjectsBasicTypes.h"

class IndirectObjectsReferenceRegistry;

#define PAGE_TREE_LEVEL_SIZE 10

class PageTree
{
public:
	PageTree(ObjectIDType inObjectID);
	PageTree(IndirectObjectsReferenceRegistry& inObjectsRegistry);
	~PageTree(void);

	ObjectIDType GetID();
	PageTree* GetParent();
	bool IsLeafParent();
	int GetNodesCount();
	// will return null for improper indexes or if has page IDs as children
	PageTree* GetPageTreeChild(int i);

	// will return 0 for improper indexes or if has page nodes as children
	ObjectIDType GetPageIDChild(int i);

	PageTree* AddNodeToTree(ObjectIDType inNodeID,IndirectObjectsReferenceRegistry& inObjectsRegistry);

	PageTree* CreateBrotherOrCousin(IndirectObjectsReferenceRegistry& inObjectsRegistry);
	PageTree* AddNodeToTree(PageTree* inPageTreeNode,IndirectObjectsReferenceRegistry& inObjectsRegistry);

	void SetParent(PageTree* inParent);

private:
	PageTree* mParent;
	ObjectIDType mPageTreeID;
	bool mIsLeafParent;
	int mKidsIndex;

	PageTree* mKidsNodes[PAGE_TREE_LEVEL_SIZE];
	ObjectIDType mKidsIDs[PAGE_TREE_LEVEL_SIZE];

};
