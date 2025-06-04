#include "paktool.hpp"
#include <SDL3/SDL.h>

using namespace PopLib;

PopPak *PopLib::gPopPak = nullptr;
    
int StartPacking(void *theArg)
{
    PopPak *aPopPak = (PopPak *)theArg;

	aPopPak->mGPAK = new GPAK();
	aPopPak->mGPAK->SetPassword(aPopPak->mPassword);
    aPopPak->mDoProgressBar = true;
	aPopPak->mGPAK->Create(aPopPak->mPakName,{aPopPak->mPakFileOutputFolder});
    gPopPak->mDoProgressBar = false;
    return 0;
}

int StartUnpacking(void *theArg)
{
    PopPak *aPopPak = (PopPak *)theArg;

	aPopPak->mGPAK = new GPAK();
	aPopPak->mGPAK->SetPassword(aPopPak->mPassword);
    aPopPak->mDoProgressBar = true;
	aPopPak->mGPAK->Extract(aPopPak->mPathToPack,{aPopPak->mUnpackOutputFolder});
    gPopPak->mDoProgressBar = false;
    return 0;
}



PopPak::PopPak()
{
    gPopPak = this;
    mDoProgressBar = true;
    mPassword = "";
    mPakName = "";
    mPakFileOutputFolder = "";
    mPathToPack = "";
    mUnpackOutputFolder = "";
    mCurrentProccesedFile = "";
    mCurrentOperation = "";
    mGPAK = nullptr;
}
    
PopPak::~PopPak()
{
}

void PopPak::Package()
{
    mCurrentOperation = "Packing";
    SDL_Thread *aPackThread = SDL_CreateThread(StartPacking, "PackingThread", (void *)this);
    SDL_DetachThread(aPackThread);
}
    

void PopPak::Extract()
{
    mCurrentOperation = "Extracting";
    SDL_Thread *aPackThread = SDL_CreateThread(StartUnpacking, "ExtractThread", (void *)this);
    SDL_DetachThread(aPackThread);
}
