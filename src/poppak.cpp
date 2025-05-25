#include <iostream>
#include <string>
#include <list>
#include <cstdarg>

#include "gpak.hpp"
#include "common.hpp" // get MkDir and some other helper functions.

using namespace PopWork;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

bool package = false;
bool unpackage = false;

//////////////////////////////////////////////////////////////////////////
// This is the default password for the PakInterface.
// The idea is to be able to change the password  on the command prompt.
std::string gEncryptPassword = "PopCapPopWorkFramework";
//////////////////////////////////////////////////////////////////////////
/*
int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 1;
	}

	int anArgPos = 1;

	while (argc >= anArgPos + 1)
	{
		if (argv[anArgPos][0] == '/')
		{
			if (stricmp(argv[anArgPos], "/U") == 0)
				unpackage = true;
			else if (stricmp(argv[anArgPos], "/P") == 0)
				package = true;
			else if (stricmp(argv[anArgPos], "/K") == 0)
			{
				anArgPos++;
				gEncryptPassword = argv[anArgPos];

				if (gEncryptPassword == "")
				{
					std::cerr << "Invalid Password:" << argv[anArgPos] << std::endl;
					return 106;
				}
			}
			else
			{
				std::cerr << "Invalid option" << std::endl;
				return 101;
			}

			anArgPos++;
		}
		else
			break;
	}

	////////////////////////////////////////////////////////////////////////////
	// PopCapPWE should be an internal tool.  In any event, the default Password
	// is actually really easy to guess.
	std::cout << StrFormat("Password: %s", gEncryptPassword.c_str()).c_str() << std::endl;
	if (gEncryptPassword == "PopCapPopWorkFramework")
		std::cerr << StrFormat("WARNING: %s is the Default Password and is NOT allowed for Distribution!",
							   gEncryptPassword.c_str())
						 .c_str()
				  << std::endl;

	if (argc < anArgPos + 2)
	{
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 102;
	}

	if (package && unpackage)
	{
		std::cerr << "/U and /P may not be specified together." << std::endl;
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 103;
	}

	std::string aPackName = argv[anArgPos++];
	std::string aDirName = argv[anArgPos++];

	if ((aDirName[aDirName.length() - 1] != '\\') && (aDirName[aDirName.length() - 1] != '/'))
		aDirName += "\\";

	GPAK *aGPAK = new GPAK();
	aGPAK->SetPassword(gEncryptPassword);

	if (package)
	{
		aGPAK->Create(aPackName,{aDirName});
	}

	
	if (unpackage)
	{
		aGPAK->Extract(aPackName, {aDirName});
	}

	delete aGPAK;

	return 0;
}
*/