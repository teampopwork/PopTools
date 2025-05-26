#include "gpak.hpp"
#include "paktool.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <filesystem>
#include <zlib.h>
extern "C"
{
#include <aes.h>
}

namespace fs = std::filesystem;

static std::vector<std::string> collectFiles(const std::vector<std::string> &inputs)
{
	std::vector<std::string> result;
	for (const auto &input : inputs)
	{
		fs::path path(input);
		if (fs::is_regular_file(path))
		{
			result.push_back(path.string());
		}
		else if (fs::is_directory(path))
		{
			for (auto &p : fs::recursive_directory_iterator(path))
			{
				if (fs::is_regular_file(p))
					result.push_back(p.path().string());
			}
		}
	}
	return result;
}

void GPAK::Create(const std::string &pakPath, const std::vector<std::string> &inputs)
{
	std::ofstream out(pakPath, std::ios::binary);
	if (!out)
		throw std::runtime_error("Failed to create .gpak file");

	GPAKHeader header = {};
	std::memcpy(header.magic, "GPAK", 4);
	header.magic[4] = '\0';
	header.version = 1;

	out.write(reinterpret_cast<const char *>(&header), sizeof(header)); // placeholder

	std::vector<GPAKFileEntry> entries;
	auto files = collectFiles(inputs);
	mTotalFiles = files.size();
	mProcessedFiles = 0;
	for (const auto &fullPath : files)
	{
		PopWork::gPopPak->mCurrentProccesedFile = fullPath;

		std::ifstream in(fullPath, std::ios::binary | std::ios::ate);
		if (!in)
			throw std::runtime_error("Could not read: " + fullPath);

		size_t fileSize = in.tellg();
		in.seekg(0);
		std::vector<uint8_t> buffer(fileSize);
		in.read(reinterpret_cast<char *>(buffer.data()), fileSize);

		auto compressed = Compress(buffer);
		if (!password.empty())
			compressed = AESEncrypt(compressed);

		GPAKFileEntry entry = {};
		std::memset(entry.path, 0, sizeof(entry.path));

		std::string relPath;
		for (const auto &root_path : inputs)
		{
			if (fullPath.find(root_path) == 0)
			{
				fs::path relative = fs::relative(fullPath, root_path);
				relPath = relative.generic_string(); // always uses '/' regardless of platform
				break;
			}
		}
		std::replace(relPath.begin(), relPath.end(), '\\', '/');
		std::strncpy(entry.path, relPath.c_str(), sizeof(entry.path) - 1);

		entry.dataOffset = static_cast<uint64_t>(out.tellp());
		entry.originalSize = static_cast<uint32_t>(buffer.size());
		entry.compressedSize = static_cast<uint32_t>(compressed.size());

		out.write(reinterpret_cast<const char *>(compressed.data()), compressed.size());
		entries.push_back(entry);
		mProcessedFiles++;
	}

	header.fileTableOffset = static_cast<uint64_t>(out.tellp());
	header.fileCount = static_cast<uint32_t>(entries.size());

	for (const auto &entry : entries)
		out.write(reinterpret_cast<const char *>(&entry), sizeof(entry));

	out.seekp(0);
	out.write(reinterpret_cast<const char *>(&header), sizeof(header));
}

void GPAK::Extract(const std::string &pakPath, const std::string &outputFolder)
{
	std::ifstream in(pakPath, std::ios::binary);
	if (!in)
		throw std::runtime_error("Failed to open GPAK file");

	GPAKHeader header;
	in.read(reinterpret_cast<char *>(&header), sizeof(header));
	if (std::memcmp(header.magic, "GPAK", 4) != 0)
		throw std::runtime_error("Invalid GPAK file");

	in.seekg(header.fileTableOffset);
	std::vector<GPAKFileEntry> entries(header.fileCount);
	in.read(reinterpret_cast<char *>(entries.data()), entries.size() * sizeof(GPAKFileEntry));

	mProcessedFiles = 0;
	mTotalFiles = entries.size();
	
	for (const auto &entry : entries)
	{
		PopWork::gPopPak->mCurrentProccesedFile = entry.path;
		in.seekg(entry.dataOffset);

		std::vector<uint8_t> compressed(entry.compressedSize);
		in.read(reinterpret_cast<char *>(compressed.data()), compressed.size());

		if (!password.empty())
		{
			if (compressed.size() % 16 != 0)
				throw std::runtime_error("Encrypted data size is not a multiple of AES block size");

			compressed = AESDecrypt(compressed);
		}

		// Decompress
		auto decompressed = Decompress(compressed, entry.originalSize);

		if (decompressed.size() != entry.originalSize)
			throw std::runtime_error("Decompressed size mismatch for file: " + std::string(entry.path));

		fs::path outPath = fs::path(outputFolder) / entry.path;
		fs::create_directories(outPath.parent_path());

		std::ofstream out(outPath, std::ios::binary);
		if (!out)
			throw std::runtime_error("Failed to create output file: " + outPath.string());

		out.write(reinterpret_cast<char *>(decompressed.data()), decompressed.size());

		mProcessedFiles++;
	}
}

std::vector<uint8_t> GPAK::Compress(const std::vector<uint8_t> &input)
{
	uLongf destLen = compressBound(input.size());
	std::vector<uint8_t> output(destLen);

	int res = ::compress2(output.data(), &destLen, input.data(), input.size(), 6);
	if (res != Z_OK)
		throw std::runtime_error("zlib compression failed with code: " + std::to_string(res));

	output.resize(destLen);
	return output;
}

std::vector<uint8_t> GPAK::Decompress(const std::vector<uint8_t> &input, size_t originalSize)
{
	std::vector<uint8_t> output(originalSize);
	uLongf destLen = originalSize;

	int res = ::uncompress(output.data(), &destLen, input.data(), input.size());
	if (res != Z_OK)
		throw std::runtime_error("zlib decompression failed with code: " + std::to_string(res));

	return output;
}

std::vector<uint8_t> GPAK::AESEncrypt(const std::vector<uint8_t> &data)
{
	AES_ctx ctx;
	uint8_t key[32] = {};
	std::memcpy(key, password.data(), std::min(password.size(), sizeof(key)));
	AES_init_ctx(&ctx, key);

	std::vector<uint8_t> padded = data;
	size_t pad = 16 - (padded.size() % 16);
	padded.resize(padded.size() + pad, static_cast<uint8_t>(pad));

	for (size_t i = 0; i < padded.size(); i += 16)
	{
		AES_ECB_encrypt(&ctx, padded.data() + i);
	}
	return padded;
}

std::vector<uint8_t> GPAK::AESDecrypt(const std::vector<uint8_t> &data)
{
	AES_ctx ctx;
	uint8_t key[32] = {};
	std::memcpy(key, password.data(), std::min(password.size(), sizeof(key)));
	AES_init_ctx(&ctx, key);

	std::vector<uint8_t> decrypted = data;
	for (size_t i = 0; i < decrypted.size(); i += 16)
	{
		AES_ECB_decrypt(&ctx, decrypted.data() + i);
	}

	if (!decrypted.empty())
	{
		uint8_t pad = decrypted.back();
		if (pad <= 16)
			decrypted.resize(decrypted.size() - pad);
	}
	return decrypted;
}
