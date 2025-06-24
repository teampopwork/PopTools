#include "resourcetool.hpp"
#include "common.hpp"
#include <fstream>

using namespace PopLib;

static void GetStandardFileName(std::string &theName)
{
	for (int i=0; i<(int)theName.length(); i++)
	{
		char aChar = tolower(theName[i]);
		if (aChar=='\\')
			aChar = '/';

		theName[i] = aChar;
	}
}

static void AddTrailingSlash(std::string &thePath)
{
	if (thePath.empty())
		return;

	if (thePath[thePath.length()-1]=='\\')
		thePath[thePath.length()-1]='/';
	else
		thePath += '/';	
}

static void RemoveExtensionAndUnderscores(std::string &thePath)
{
	if (thePath.empty())
		return;

	GetStandardFileName(thePath);
	int aSlashPos = ((int)thePath.find_last_of('/'))+1;

	if (thePath[aSlashPos]=='_')
		thePath = thePath.substr(0,aSlashPos) + thePath.substr(aSlashPos+1);

	if (thePath.empty())
		return;

	int aDotPos = (int)thePath.find_last_of('.');
	if (aDotPos != std::string::npos)
		thePath = thePath.substr(0,aDotPos);

	if (thePath[thePath.length()-1]=='_')
		thePath = thePath.substr(0,thePath.length()-1);
}


ResourceManager::ResourceManager()
{
	mHasFailed = false;
	mXMLParser = NULL;
}
    
ResourceManager::~ResourceManager()
{
}

std::string ResourceManager::GetErrorText()
{
	return mError;
}

bool ResourceManager::HadError()
{
	return !mError.empty();
}

bool ResourceManager::Fail(const std::string& theErrorText)
{
	if (!mHasFailed)
	{
		mHasFailed = true;
		int aLineNum = mXMLParser->GetCurrentLineNum();

		char aLineNumStr[16];
		sprintf(aLineNumStr, "%d", aLineNum);	

		mError = theErrorText;

		if (aLineNum > 0)
			mError += std::string(" on Line ") + aLineNumStr;

		if (mXMLParser->GetFileName().length() > 0)
			mError += " in File '" + mXMLParser->GetFileName() + "'";
	}

	return false;
}

void ResourceManager::AddFile(const std::string &thePath)
{
	if (thePath.empty())
		return;

	std::string aPath = mDirPrefix + thePath;
	GetStandardFileName(aPath);
	mDirSet.insert(GetFileDir(aPath));

	RemoveExtensionAndUnderscores(aPath);
	if (!aPath.empty() && aPath[0]!='!') // program supplied resource
		mFileSet.insert(aPath);
}

bool ResourceManager::ParseCommonResource(XMLElement &theElement, ResType theType)
{
	std::string aPath = theElement.mAttributes["path"];
	if (aPath.empty())
		return Fail("No path specified.");
	
	std::string anId;
	XMLParamMap::iterator anItr = theElement.mAttributes.find("id");
	if (anItr == theElement.mAttributes.end())
		anId = mDefaultIdPrefix + GetFileName(aPath,true);
	else
		anId = mDefaultIdPrefix + anItr->second;

	AddFile(aPath);
	if (theType==ResType_Image)
	{
		anItr = theElement.mAttributes.find("alphaimage");
		if (anItr != theElement.mAttributes.end())
			AddFile(anItr->second);

		anItr = theElement.mAttributes.find("alphagrid");
		if (anItr != theElement.mAttributes.end())
			AddFile(anItr->second);
	}



	BaseRes aRes;
	aRes.mId = anId;
	aRes.mType = theType;

	anItr = theElement.mAttributes.find("alias");
	if (anItr != theElement.mAttributes.end())
		aRes.mAlias[0] = mDefaultIdPrefix + anItr->second;

	for (int i=0; i<MAX_ALIASES; i++)
	{
		anItr = theElement.mAttributes.find(StrFormat("alias%d",i+1));
		if (anItr != theElement.mAttributes.end())
			aRes.mAlias[i] = mDefaultIdPrefix + anItr->second;
	}


	std::pair<ResMap::iterator,bool> aRet = mCurResMap->insert(ResMap::value_type(anId,aRes));
	if (!aRet.second)
		return Fail("Resource already defined.");

	mVariableList.push_back(aRes);
	return true;
}

bool ResourceManager::ParseSetDefaults(XMLElement &theElement)
{
	XMLParamMap::iterator anItr;

	anItr = theElement.mAttributes.find("idprefix");
	if (anItr != theElement.mAttributes.end())
		mDefaultIdPrefix = RemoveTrailingSlash(anItr->second);	

	std::string aPath = theElement.mAttributes["path"];
	GetStandardFileName(aPath);
	mDirPrefix = aPath;
	AddTrailingSlash(mDirPrefix);

	return true;
}

bool ResourceManager::ParseResources()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "Image")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Image))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "Sound")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Sound))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "Font")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Font))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "SetDefaults")
			{
				if (!ParseSetDefaults(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");		
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}		
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool ResourceManager::DoParseResources()
{
	if (!mXMLParser->HasFailed())
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == "Resources")
				{
					std::string aResGroup = aXMLElement.mAttributes["id"];
					if (aResGroup.empty())
						return Fail("No id specified.");

					mCurResMap = &mResGroupMap[aResGroup];

					if (!ParseResources())
						break;
				}
				else 
				{
					Fail("Invalid Section '" + aXMLElement.mValue + "'");
					break;
				}
			}
			else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
			{
				Fail("Element Not Expected '" + aXMLElement.mValue + "'");
				break;
			}
		}
	}

	if (mXMLParser->HasFailed())
		Fail(mXMLParser->GetErrorText());	

	delete mXMLParser;
	mXMLParser = NULL;

	return !mHasFailed;
}

bool ResourceManager::ParseResourcesFile(const std::string& theFilename)
{
	mXMLParser = new XMLParser();
	mXMLParser->OpenFile(theFilename);

	XMLElement aXMLElement;
	while (true)
	{
		if (!mXMLParser->NextElement(&aXMLElement))
			Fail(mXMLParser->GetErrorText());

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue != "ResourceManifest")
				break;
			else
				return DoParseResources();
		}
	}
		
	Fail("Expecting ResourceManifest tag");

	return DoParseResources();	
}

std::string ResourceManager::GetResName(const BaseRes &theRes)
{
	return theRes.mId;
}

std::string	ResourceManager::GetTypeName(const BaseRes &theRes)
{
	switch(theRes.mType)
	{
		case ResType_Image: return "Image*"; 
		case ResType_Sound: return "int"; 
		case ResType_Font: return "Font*"; 
	}

	return "";
}

void ResourceManager::WriteSourceFileGroup(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap)
{
	out.push_back(StrFormat("bool PopLib::%sExtract%sResources(ResourceManager *theManager)\n",mFunctionPrefix.c_str(),theResGroup.c_str()));
	out.push_back("{\n");
	out.push_back("\tgNeedRecalcVariableToIdMap = true;\n\n");
	out.push_back("\tResourceManager &aMgr = *theManager;\n");
	out.push_back("\ttry\n");
	out.push_back("\t{\n");

	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		const std::string &anId = anItr->first;
		BaseRes &aRes = anItr->second;

		std::string aVarName = GetResName(aRes);
		std::string aMethodName;
		switch(aRes.mType)
		{
			case ResType_Image: aMethodName = "GetImageThrow"; break;
			case ResType_Sound: aMethodName = "GetSoundThrow"; break;
			case ResType_Font: aMethodName = "GetFontThrow"; break;
		}

		out.push_back(StrFormat("\t\t%s = aMgr.%s(\"%s\");\n",aVarName.c_str(),aMethodName.c_str(),anId.c_str()));
	}

	out.push_back("\t}\n");
	out.push_back("\tcatch(ResourceManagerException&)\n");
	out.push_back("\t{\n");
//	fprintf(theFile,"\t\tif (exitOnFailure)\n\t\t{\n");
//	fprintf(theFile,"\t\t\tMessageBox(NULL,ex.what.c_str(),\"Resource Error\",MB_OK);\n");
//	fprintf(theFile,"\t\t\texit(0);\n");
//	fprintf(theFile,"\t\t}\n");
	out.push_back("\t\treturn false;\n");
	out.push_back("\t}\n");
	out.push_back("\treturn true;\n");
	out.push_back("}\n\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteSourceFileVariables(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap)
{
	out.push_back(StrFormat("// %s Resources\n",theResGroup.c_str()));
	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		BaseRes &aRes = anItr->second;
		std::string aVarName = GetResName(aRes);
		std::string aTypeName = GetTypeName(aRes);

		out.push_back(StrFormat("%s PopLib::%s;\n",aTypeName.c_str(), aVarName.c_str()));
	}
	out.push_back("\n");
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteHeaderFileGroup(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap)
{
	out.push_back(StrFormat("\t// %s Resources\n",theResGroup.c_str()));
	out.push_back(StrFormat("\tbool %sExtract%sResources(ResourceManager *theMgr);\n",mFunctionPrefix.c_str(),theResGroup.c_str()));

	StringSet aSet;

	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		BaseRes &aRes = anItr->second;
		std::string aVarName = GetResName(aRes);
		std::string aTypeName = GetTypeName(aRes);

		aSet.insert(StrFormat("\textern %s %s;\n",aTypeName.c_str(), aVarName.c_str()));
	}

	for (StringSet::iterator aStrItr = aSet.begin(); aStrItr!= aSet.end(); ++aStrItr)
		out.push_back(StrFormat("%s",aStrItr->c_str()));

	out.push_back("\n");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> ResourceManager::WriteSourceFile(const std::string &theFileName)
{
    std::vector<std::string> lines;

    lines.push_back(StrFormat("#include \"%s.h\"\n",theFileName.c_str()));
    lines.push_back("#include \"PopLib/resources/resourcemanager.hpp\"\n");
	lines.push_back("\nusing namespace PopLib;\n\n");

	lines.push_back("#pragma warning(disable:4311 4312)\n\n");
	lines.push_back("static bool gNeedRecalcVariableToIdMap = false;\n\n");

	// ExtractResourcesByName
	lines.push_back(StrFormat("bool PopLib::%sExtractResourcesByName(ResourceManager *theManager, const char *theName)\n{\n",mFunctionPrefix.c_str()));
	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
		lines.push_back(StrFormat("\tif (strcmp(theName,\"%s\")==0) return %sExtract%sResources(theManager);\n",anItr->first.c_str(),mFunctionPrefix.c_str(),anItr->first.c_str()));
	lines.push_back("\treturn false;\n}\n\n");

	// GetIdByStringId
	lines.push_back(StrFormat("PopLib::%sResourceId PopLib::%sGetIdByStringId(const char *theStringId)\n{\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back("\ttypedef std::map<std::string,int> MyMap;\n");
	lines.push_back("\tstatic MyMap aMap;\n");
	lines.push_back("\tif(aMap.empty())\n\t{\n");
	lines.push_back(StrFormat("\t\tfor(int i=0; i<%sRESOURCE_ID_MAX; i++)\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\t\t\taMap[%sGetStringIdById(i)] = i;\n\t}\n\n",mFunctionPrefix.c_str()));
	lines.push_back("\tMyMap::iterator anItr = aMap.find(theStringId);\n");
	lines.push_back("\tif (anItr == aMap.end())\n");
	lines.push_back(StrFormat("\t\treturn %sRESOURCE_ID_MAX;\n",mFunctionPrefix.c_str()));
	lines.push_back("\telse\n");
	lines.push_back(StrFormat("\t\treturn (%sResourceId) anItr->second;\n",mFunctionPrefix.c_str()));
	lines.push_back("}\n\n");
	

	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
	{
		WriteSourceFileVariables(lines, anItr->first,anItr->second);
		WriteSourceFileGroup(lines, anItr->first,anItr->second);
	}

	lines.push_back("static void* gResources[] =\n");
	lines.push_back("{\n");

	ResList::iterator aVarItr;
	for (aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
		lines.push_back(StrFormat("\t&%s,\n",aVarItr->mId.c_str()));

	lines.push_back("\tNULL\n");
	lines.push_back("};\n\n");

	// LoadImageById
	lines.push_back(StrFormat("Image* PopLib::%sLoadImageById(ResourceManager *theManager, int theId)\n{\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\treturn (*((Image**)gResources[theId]) = theManager->LoadImage(%sGetStringIdById(theId)));\n}\n\n", mFunctionPrefix.c_str()));

	// ReplaceImageById
	lines.push_back(StrFormat("void PopLib::%sReplaceImageById(ResourceManager *theManager, int theId, Image *theImage)\n{\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\ttheManager->ReplaceImage(%sGetStringIdById(theId),theImage);\n", mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\t*(Image**)gResources[theId] = theImage;\n}\n\n", mFunctionPrefix.c_str()));

	// GetImageById
	lines.push_back(StrFormat("Image* PopLib::%sGetImageById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(Image**)gResources[theId];\n");
	lines.push_back("}\n\n");

	// GetFontById
	lines.push_back(StrFormat("Font* PopLib::%sGetFontById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(Font**)gResources[theId];\n");
	lines.push_back("}\n\n");

	// GetSoundById
	lines.push_back(StrFormat("int PopLib::%sGetSoundById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(int*)gResources[theId];\n");
	lines.push_back("}\n\n");

	// GetImageRefById
	lines.push_back(StrFormat("Image*& PopLib::%sGetImageRefById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(Image**)gResources[theId];\n");
	lines.push_back("}\n\n");

	// SetFontRefById
	lines.push_back(StrFormat("Font*& PopLib::%sGetFontRefById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(Font**)gResources[theId];\n");
	lines.push_back("}\n\n");

	// SetSoundRefById
	lines.push_back(StrFormat("int& PopLib::%sGetSoundRefById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\treturn *(int*)gResources[theId];\n");
	lines.push_back("}\n\n");

	// GetIdByVariable
	lines.push_back(StrFormat("static PopLib::%sResourceId %sGetIdByVariable(const void *theVariable)\n{\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back("\ttypedef std::map<int,int> MyMap;\n");
	lines.push_back("\tstatic MyMap aMap;\n");
	lines.push_back("\tif(gNeedRecalcVariableToIdMap)\n\t{\n");
	lines.push_back("\t\tgNeedRecalcVariableToIdMap = false;\n");
	lines.push_back("\t\taMap.clear();\n");
	lines.push_back(StrFormat("\t\tfor(int i=0; i<%sRESOURCE_ID_MAX; i++)\n",mFunctionPrefix.c_str()));
	lines.push_back("\t\t\taMap[*(int*)gResources[i]] = i;\n\t}\n\n");
	lines.push_back("\tMyMap::iterator anItr = aMap.find((int)theVariable);\n");
	lines.push_back("\tif (anItr == aMap.end())\n");
	lines.push_back(StrFormat("\t\treturn %sRESOURCE_ID_MAX;\n",mFunctionPrefix.c_str()));
	lines.push_back("\telse\n");
	lines.push_back(StrFormat("\t\treturn (%sResourceId) anItr->second;\n",mFunctionPrefix.c_str()));
	lines.push_back("}\n\n");

	// GetIdByImage
	lines.push_back(StrFormat("PopLib::%sResourceId PopLib::%sGetIdByImage(Image *theImage)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back(StrFormat("\treturn %sGetIdByVariable(theImage);\n",mFunctionPrefix.c_str()));
	lines.push_back("}\n\n");

	// GetIdByFont
	lines.push_back(StrFormat("PopLib::%sResourceId PopLib::%sGetIdByFont(Font *theFont)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back(StrFormat("\treturn %sGetIdByVariable(theFont);\n",mFunctionPrefix.c_str()));
	lines.push_back("}\n\n");

	// GetIdBySound
	lines.push_back(StrFormat("PopLib::%sResourceId PopLib::%sGetIdBySound(int theSound)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back(StrFormat("\treturn %sGetIdByVariable((void*)theSound);\n",mFunctionPrefix.c_str()));
	lines.push_back("}\n\n");

	// GetStringIdById
	lines.push_back(StrFormat("const char* PopLib::%sGetStringIdById(int theId)\n",mFunctionPrefix.c_str()));
	lines.push_back("{\n");
	lines.push_back("\tswitch(theId)\n");
	lines.push_back("\t{\n");
	for (aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
		lines.push_back(StrFormat("\t\tcase %s_ID: return \"%s\";\n",aVarItr->mId.c_str(),aVarItr->mId.c_str()));
	lines.push_back("\t\tdefault: return \"\";\n");
	lines.push_back("\t}\n");
	lines.push_back("}\n");
	lines.push_back("\n");

    return lines;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> ResourceManager::WriteHeaderFile(const std::string &theName)
{
    std::vector<std::string> lines;

	lines.push_back(StrFormat("#ifndef __%s_H__\n",theName.c_str()));
	lines.push_back(StrFormat("#define __%s_H__\n",theName.c_str()));
	lines.push_back("\n");
	lines.push_back("namespace PopLib\n");
	lines.push_back("{\n");
	lines.push_back("\tclass ResourceManager;\n");
	lines.push_back("\tclass Image;\n");
	lines.push_back("\tclass Font;\n\n");

	lines.push_back(StrFormat("\tImage* %sLoadImageById(ResourceManager *theManager, int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tvoid %sReplaceImageById(ResourceManager *theManager, int theId, Image *theImage);\n",mFunctionPrefix.c_str()));

	lines.push_back(StrFormat("\tbool %sExtractResourcesByName(ResourceManager *theManager, const char *theName);\n\n",mFunctionPrefix.c_str()));

	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
		WriteHeaderFileGroup(lines ,anItr->first,anItr->second);

	lines.push_back(StrFormat("\tenum %sResourceId\n",mFunctionPrefix.c_str()));
	lines.push_back("\t{\n");
	for (ResList::iterator aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
	{
		lines.push_back(StrFormat("\t\t%s_ID,\n",aVarItr->mId.c_str()));
		for (int i=0; i<MAX_ALIASES; i++)
		{
			if (!aVarItr->mAlias[i].empty())
				lines.push_back(StrFormat("\t\t%s_ID = %s_ID,\n",aVarItr->mAlias[i].c_str(),aVarItr->mId.c_str()));
		}
	}

	lines.push_back(StrFormat("\t\t%sRESOURCE_ID_MAX\n",mFunctionPrefix.c_str()));
	lines.push_back("\t};\n\n");
	lines.push_back(StrFormat("\tImage* %sGetImageById(int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tFont* %sGetFontById(int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tint %sGetSoundById(int theId);\n\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tImage*& %sGetImageRefById(int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tFont*& %sGetFontRefById(int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\tint& %sGetSoundRefById(int theId);\n\n",mFunctionPrefix.c_str()));

	lines.push_back(StrFormat("\t%sResourceId %sGetIdByImage(Image *theImage);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\t%sResourceId %sGetIdByFont(Font *theFont);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\t%sResourceId %sGetIdBySound(int theSound);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));

	lines.push_back(StrFormat("\tconst char* %sGetStringIdById(int theId);\n",mFunctionPrefix.c_str()));
	lines.push_back(StrFormat("\t%sResourceId %sGetIdByStringId(const char *theStringId);\n\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str()));

	lines.push_back("} // namespace PopLib\n");
	lines.push_back("\n\n#endif\n");

    return lines;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::WriteSourceCode(const std::string &theFilename, const std::string &thePrefix)
{
	ParseResourcesFile(theFilename);

	mFunctionPrefix = thePrefix;

	std::string aName = GetFileName(theFilename,true);
	std::string aPath = GetFileDir(theFilename);
	std::string aSourceName = aName + ".cpp";
	std::string aHeaderName = aName + ".h";

	std::vector<std::string> headerlines = WriteHeaderFile(aName);
	std::vector<std::string> srclines = WriteSourceFile(aName);

    std::ofstream srcFile(aSourceName);
    if (!srcFile) {
        Fail("Couldn't open src file.");
        return false;
    }

    for (const std::string& line : srclines) {
        srcFile << line;
    }

    std::ofstream headerFile(aHeaderName);
    if (!headerFile) {
        Fail("Couldn't open header file.");
        return false;
    }

    for (const std::string& line : headerlines) {
        headerFile << line;
    }
	
	return true;

}