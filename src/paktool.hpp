#ifndef __POPPAK_HPP__
#define __POPPAK_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include "gpak.hpp"


namespace PopLib
{
    class PopPak
    {
    public:
        PopPak();
        ~PopPak();

        void Package();
        void Extract();

        ::GPAK *mGPAK; 
        std::string mPassword;

        std::string mPakName;
        std::string mPathToPack;
        std::string mPakFileOutputFolder;
        std::string mUnpackOutputFolder;
        std::string mCurrentProccesedFile;
        std::string mCurrentOperation;

        bool mDoProgressBar;
    };


    extern PopPak *gPopPak;
}

#endif