#ifndef __GPAK_HPP__
#define __GPAK_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <cstdint>
#include <string>
#include <vector>


/**
 * @brief GPAK header
 */
struct GPAKHeader
{
	char magic[5];			  // should be {'G','P','A','K','\0'}
	uint32_t version;		  // format version (start at 1)
	uint32_t fileCount;		  // how many entries in the file table
	uint64_t fileTableOffset; // byte offset in this .pak where the table begins
};

/**
 * @brief GPAK file entry
 */
struct GPAKFileEntry
{
	char path[256];			 // null-terminated relative path
	uint64_t dataOffset;	 // byte offset in .pak where compressed data lives
	uint32_t compressedSize; // size in bytes of the compressed blob
	uint32_t originalSize;	 // uncompressed size in bytes
};

class GPAK
{
  public:
	GPAK() = default;
	virtual ~GPAK() = default;

	virtual void Create(const std::string &pakPath, const std::vector<std::string> &files);
	virtual void Extract(const std::string &pakPath, const std::string &outputFolder);
	void SetPassword(const std::string &pw)
	{
		password = pw;
	}

	int mTotalFiles;
	int mProcessedFiles;

  private:
	std::string password;

	std::vector<uint8_t> Compress(const std::vector<uint8_t> &input);
	std::vector<uint8_t> Decompress(const std::vector<uint8_t> &input, size_t originalSize);
	std::vector<uint8_t> AESEncrypt(const std::vector<uint8_t> &data);
	std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t> &data);
};

#endif // __GPAK_HPP__