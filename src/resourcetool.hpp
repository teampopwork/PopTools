#ifndef __RESOURCETOOL_HPP__
#define __RESOURCETOOL_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include "xmlparser.hpp"


namespace PopLib
{

const int MAX_ALIASES = 10;

class ResourceManager
{
    protected:
        enum ResType
        {
            ResType_Image,
            ResType_Sound,
            ResType_Font
        };

        struct BaseRes
        {
            ResType mType;
            std::string mId;
            std::string mAlias[MAX_ALIASES];
        };

        typedef std::map<std::string,BaseRes> ResMap;
        typedef std::map<std::string,ResMap> ResGroupMap;
        typedef std::set<std::string> StringSet;
        typedef std::set<std::string> StringList;
        typedef std::list<BaseRes> ResList;


        ResMap					*mCurResMap;
        ResGroupMap				mResGroupMap;
        std::string				mDefaultIdPrefix;
        ResList					mVariableList;
        std::string				mFunctionPrefix;
        std::string				mDirPrefix;
        StringSet				mDirSet;
        StringSet				mFileSet;

        XMLParser*				mXMLParser;
        std::string				mError;
        bool					mHasFailed;

        bool Fail(const std::string& theErrorText);

        void AddFile(const std::string &thePath);
        bool ParseCommonResource(XMLElement &theElement, ResType theType);
        bool ParseSetDefaults(XMLElement &theElement);
        bool ParseResources();
        bool DoParseResources();

        std::string GetResName(const BaseRes &theRes);
        std::string GetTypeName(const BaseRes &theRes);
        void WriteSourceFileVariables(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap);
        void WriteSourceFileGroup(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap);
        void WriteHeaderFileGroup(std::vector<std::string>& out, const std::string &theResGroup, ResMap &theMap);
        std::vector<std::string> WriteSourceFile(const std::string &theFileName);
        std::vector<std::string> WriteHeaderFile(const std::string &theName);

    public:
        ResourceManager();
        virtual ~ResourceManager();

        bool ParseResourcesFile(const std::string& theFilename);
        bool WriteSourceCode(const std::string &theFilename, const std::string &thePrefix);
        std::string GetErrorText();
        bool HadError();

    };

}

#endif