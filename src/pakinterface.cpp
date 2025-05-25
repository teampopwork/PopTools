#include "pakinterface.hpp"
#include "common.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace std;

PakInterface *gPakInterface = new PakInterface();

template <typename T>
bool readFromBuffer(const uint8_t* buffer, size_t bufferSize, size_t& offset, T& outValue)
{
    if (offset + sizeof(T) > bufferSize)
        return false;
    memcpy(&outValue, buffer + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

//////////////////
static bool starts_with(const std::string &str, const std::string &prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static inline std::string toupper(const std::string &theString)
{
	std::string aString;
	for (unsigned i = 0; i < theString.length(); i++)
		aString += toupper(theString[i]);
	return aString;
}
//////////////////

// Helper to convert wildcard patterns to simple matching
static bool matchPattern(const string &pattern, const string &name)
{
	size_t pos = pattern.find('*');
	if (pos == string::npos)
		return _stricmp(pattern.c_str(), name.c_str()) == 0;
	string prefix = pattern.substr(0, pos);
	string suffix = pattern.substr(pos + 1);
	if (!starts_with(name, prefix))
		return false;
	if (suffix.empty())
		return true;
	if (name.length() < prefix.length() + suffix.length())
		return false;
	return _stricmp(name.substr(name.size() - suffix.size()).c_str(), suffix.c_str()) == 0;
}

PakInterface::PakInterface()
{
	// nothing to initialize beyond base
}

PakInterface::~PakInterface()
{
}

static void FixFileName(std::string theFileName, const char* theUpperName)
{
namespace fs = std::filesystem;

    try
    {
        fs::path cwd = fs::current_path();
        fs::path inputPath = fs::absolute(theFileName);

        std::string cwdStr = cwd.generic_string();
        std::string inputStr = inputPath.generic_string();

        // Remove cwd prefix if it exists
        if (inputStr.rfind(cwdStr, 0) == 0)
        {
            theFileName = inputStr.substr(cwdStr.size());
        }
    }
    catch (...)
    {
        // fallback: use raw theFileName if std::filesystem fails
    }

    std::string result;
    result.reserve(theFileName.size());

    bool lastWasSlash = false;

    for (size_t i = 0; i < theFileName.size(); ++i)
    {
        char c = theFileName[i];

        if (c == '\\' || c == '/')
        {
            if (!lastWasSlash)
                result += '/';
            lastWasSlash = true;
        }
        else if (c == '.' && lastWasSlash && i + 1 < theFileName.size() && theFileName[i + 1] == '.')
        {
            // handle /..
            if (!result.empty())
            {
                auto slashPos = result.find_last_of('/', result.length() - 2);
                if (slashPos != std::string::npos)
                    result.erase(slashPos);
                else
                    result.clear();
            }
            lastWasSlash = false;
            ++i;
        }
        else
        {
            result += std::toupper(static_cast<unsigned char>(c));
            lastWasSlash = false;
        }
    }

    theUpperName = result.c_str();
}

bool PakInterface::AddPakFile(const string &fileName)
{
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	size_t aFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Ima be honest.
	//This is the worst thing someone has ever wrote.

	mPakCollectionList.emplace_back(aFileSize);
	PakCollection *aPakCollection = &mPakCollectionList.back();

    if (fread(aPakCollection->data(), 1, aFileSize, fp) != aFileSize) {
        fclose(fp);
        return false;
    }

	auto* bytes = static_cast<uint8_t*>(aPakCollection->data());
	// decrypt that data using the password.
    if (!mDecryptPassword.empty()) {
        for (size_t i = 0; i < aFileSize; ++i)
            bytes[i] ^= static_cast<uint8_t>(mDecryptPassword[i % mDecryptPassword.size()]);
    }

	fclose(fp);


	// Parse header and entries from the now decrypted buffer
    size_t aOffset = 0;

    // Read magic number (4 bytes)
    if (aOffset + sizeof(uint32_t) > aFileSize)
        return false;
	
    uint32_t aMagic = *reinterpret_cast<const uint32_t*>(bytes + aOffset);
    if (!readFromBuffer(bytes, aFileSize, aOffset, aMagic))
    	return false;

    if (aMagic != 0xBAC04AC0)
        return false;

    // Read version (4 bytes)
    if (aOffset + sizeof(uint32_t) > aFileSize)
        return false;

    uint32_t aVersion = *reinterpret_cast<const uint32_t*>(bytes + aOffset);
	if (!readFromBuffer(bytes, aFileSize, aOffset, aVersion))
    	return false;

	if (aVersion > 0)
        return false;

    int aPos = 0;
    while (true) {
        if (aOffset + 1 > aFileSize) break;

        uint8_t flags = bytes[aOffset];
        aOffset += 1;

        if (flags & 0x80) // FILEFLAGS_END
            break;

        if (aOffset + 1 > aFileSize) break;

        uint8_t aNameWidth = bytes[aOffset];
        aOffset += 1;

        if (aOffset + aNameWidth > aFileSize) break;

        char aName[256] = {0};
        memcpy(aName, bytes + aOffset, aNameWidth);
        aOffset += aNameWidth;

        // Normalize slashes
        for (int i = 0; i < aNameWidth; ++i) {
            if (aName[i] == '\\')
                aName[i] = '/';
        }

        if (aOffset + sizeof(int) + sizeof(int64_t) > aFileSize) break;

        int aSrcSize = *reinterpret_cast<const int*>(bytes + aOffset);
        aOffset += sizeof(int);

        int64_t aFileTime = *reinterpret_cast<const int64_t*>(bytes + aOffset);
        aOffset += sizeof(int64_t);

        char anUpperName[256];
        FixFileName(aName, anUpperName);

        PakRecordMap::iterator recordItr = mPakRecordMap.insert(
            PakRecordMap::value_type(PopWork::StringToUpper(aName), PakRecord())
        ).first;

        PakRecord* aPakRecord = &recordItr->second;
        aPakRecord->mCollection = aPakCollection;
        aPakRecord->mFileName = aName;
        aPakRecord->mStartPos = aPos;
        aPakRecord->mSize = aSrcSize;
        aPakRecord->mFileTime = std::filesystem::file_time_type{std::chrono::seconds(aFileTime)};

        aPos += aSrcSize;
    }

	// Fix start positions with offset to the actual data (pos after header)
    for (auto& [key, record] : mPakRecordMap)
    {
        if (record.mCollection == aPakCollection)
            record.mStartPos += aPos;
    }

    // Also add the full file record
    PakRecord fullPakRecord;
    fullPakRecord.mCollection = aPakCollection;
    fullPakRecord.mFileName = fileName;
    fullPakRecord.mStartPos = 0;
    fullPakRecord.mSize = static_cast<int>(aFileSize);
    fullPakRecord.mFileTime = std::filesystem::file_time_type::min();

    mPakRecordMap[PopWork::StringToUpper(fileName)] = fullPakRecord;

	return true;
}

PFILE *PakInterface::FOpen(const char *fn, const char *mode)
{
	string name(fn);
	auto it = mPakRecordMap.find(toupper(name));
	if (it != mPakRecordMap.end())
	{
		PFILE *pf = new PFILE;
		pf->mRecord = &it->second;
		pf->mPos = 0;
		pf->mFP = nullptr;
		return pf;
	}
	FILE *real = fopen(fn, mode);
	if (!real)
		return nullptr;
	PFILE *pf = new PFILE;
	pf->mRecord = nullptr;
	pf->mFP = real;
	pf->mPos = 0;
	return pf;
}

int PakInterface::FClose(PFILE *pf)
{
	if (pf->mFP)
		fclose(pf->mFP);
	delete pf;
	return 0;
}

int PakInterface::FSeek(PFILE *pf, long offset, int whence)
{
	if (pf->mRecord)
	{
		auto &r = *pf->mRecord;
		switch (whence)
		{
		case SEEK_SET:
			pf->mPos = offset;
			break;
		case SEEK_CUR:
			pf->mPos += offset;
			break;
		case SEEK_END:
			pf->mPos = r.mSize + offset;
			break;
		}
		pf->mPos = std::min(std::max(pf->mPos, 0L), static_cast<long>(r.mSize));
		return 0;
	}
	return fseek(pf->mFP, offset, whence);
}

int PakInterface::FTell(PFILE *pf)
{
	return pf->mRecord ? static_cast<int>(pf->mPos) : ftell(pf->mFP);
}

size_t PakInterface::FRead(void *buf, int size, int count, PFILE *pf)
{
	if (pf->mRecord)
	{
		auto &r = *pf->mRecord;
		size_t toRead = min<size_t>(size * count, r.mSize - pf->mPos);
		unsigned char *src = reinterpret_cast<unsigned char *>(r.mCollection->data()) + r.mStartPos + pf->mPos;
		unsigned char *dst = reinterpret_cast<unsigned char *>(buf);
		for (size_t i = 0; i < toRead; ++i)
			dst[i] = src[i] ^ static_cast<unsigned char>(
								  mDecryptPassword[(r.mStartPos + pf->mPos + i) % mDecryptPassword.size()]);
		pf->mPos += toRead;
		return toRead / size;
	}
	return fread(buf, size, count, pf->mFP);
}

int PakInterface::FGetC(PFILE *pf)
{
	unsigned char c;
	return FRead(&c, 1, 1, pf) == 1 ? c : EOF;
}

int PakInterface::UnGetC(int c, PFILE *pf)
{
	if (pf->mRecord)
	{
		pf->mPos = std::max(0L, pf->mPos - 1);
		return c;
	}
	return ungetc(c, pf->mFP);
}

char *PakInterface::FGetS(char *str, int size, PFILE *pf)
{
	int i = 0;
	while (i + 1 < size)
	{
		int c = FGetC(pf);
		if (c == EOF || c == '\n')
			break;
		str[i++] = c;
	}
	str[i] = '\0';
	return i > 0 ? str : nullptr;
}

int PakInterface::FEof(PFILE *pf)
{
	return pf->mRecord ? pf->mPos >= pf->mRecord->mSize : feof(pf->mFP);
}

#ifdef _WIN32
#undef FindFirstFile
#undef FindNextFile
#endif

PFindData PakInterface::FindFirstFile(const string &pattern)
{
	PFindData fd;
	fd.it = filesystem::directory_iterator(".");
	fd.end = filesystem::directory_iterator();
	fd.pattern = pattern;
	return fd;
}

bool PakInterface::FindNextFile(PFindData &fd, string &outName)
{
	while (fd.it != fd.end)
	{
		auto entry = *fd.it++;
		string name = entry.path().filename().string();
		if (matchPattern(fd.pattern, name))
		{
			outName = name;
			return true;
		}
	}
	return false;
}

void PakInterface::FindClose(PFindData &fd)
{
	// nothing to clean
}
