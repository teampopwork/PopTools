#ifndef __POPPAK_HPP__
#define __POPPAK_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include "gpak.hpp"


namespace PopWork
{
    class PopPak
    {
    public:
        PopPak();
        ~PopPak();

        void Package();
        void Unpackage();

        void SetPassword(const std::string &theNewPass);
        
        void SetPakName(const std::string &theNewName);
        
        void SetInputFolderPath(const std::string &theNewFolder);

        ::GPAK *mGPAK; 
        std::string mPassword;

        std::string mPakName;
        std::string mInputFolderPath;

        bool mDoProgressBar;
    };


    extern PopPak *gPopPak;
}

#endif