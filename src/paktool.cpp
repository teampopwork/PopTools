#include "paktool.hpp"
#include <SDL3/SDL.h>

using namespace PopWork;

PopPak *PopWork::gPopPak = nullptr;
    
int StartPacking(void *theArg)
{
    PopPak *aPopPak = (PopPak *)theArg;

	aPopPak->mGPAK = new GPAK();
	aPopPak->mGPAK->SetPassword(aPopPak->mPassword);
    aPopPak->mDoProgressBar = true;
	aPopPak->mGPAK->Create(aPopPak->mPakName,{aPopPak->mInputFolderPath});
    gPopPak->mDoProgressBar = false;
    return 0;
}


PopPak::PopPak()
{
    gPopPak = this;
    mDoProgressBar = true;
    mPassword = "";
    mPakName = "";
    mInputFolderPath = "";
    mGPAK = nullptr;
}
    
PopPak::~PopPak()
{
}

void PopPak::Package()
{
    SDL_Thread *aPackThread = SDL_CreateThread(StartPacking, "PackingThread", (void *)this);
    SDL_DetachThread(aPackThread);
}
    

void PopPak::Unpackage()
{

}
    

void PopPak::SetPassword(const std::string &theNewPass)
{
    mPassword = theNewPass;
}
    

void PopPak::SetPakName(const std::string &theNewName)
{
    mPakName = theNewName;
}
    

void PopPak::SetInputFolderPath(const std::string &theNewFolder)
{
    mInputFolderPath = theNewFolder;
}
    