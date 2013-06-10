/*
   Source File : FSType.h


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

class FSType
{
public:
	FSType(unsigned short inFSTypeValue) {mFSTypeValue = inFSTypeValue;}

	// using Adobe applications policy as published in FontPolicies.pdf [note that if multiple bits are set the least restrictive takes over]
	// so there are just 3 options for not embedding
	bool CanEmbed(){return (mFSTypeValue != 0x2) && (mFSTypeValue != 0x0200) && (mFSTypeValue != 0x0202);}
private:

	unsigned short mFSTypeValue;
};