#ifndef __PAKTOOLWINDOW_HPP__
#define __PAKTOOLWINDOW_HPP__
#ifdef _WIN32
#pragma once
#endif

#include "core/imgui.h"
#include "core/imgui_impl_sdl3.h"
#include "core/imgui_impl_sdlrenderer3.h"
#include "../paktool.hpp"
#include "../poptoolsgui.hpp"
#include "core/imgui_stdlib.h"

using namespace PopLib;

bool doToolWindow = false;

bool &GetDebugWindowToggle()
{
    return doToolWindow;
}

std::string pw_str{"PopCapPopLibFramework"};
std::string pakName_str{"main.gpak"};
std::string inputFolder_str{""};

std::string extractPak_str{""};
std::string extractOutputFolder_str{""};

void OnFolderSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        inputFolder_str = selections[0];
    gPopPak->mPakFileOutputFolder = inputFolder_str;
}

void OnExtractFolderSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        extractOutputFolder_str = selections[0];
    gPopPak->mUnpackOutputFolder =  extractOutputFolder_str;
}

void OnPakFileSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        extractPak_str = selections[0];
    gPopPak->mPathToPack = extractPak_str;
}

void DrawPoppakTab()
    {
        ImGui::BeginTabBar("Poppak", ImGuiTabBarFlags_None);
        {
            if (ImGui::BeginTabItem("Packing"))
            {
                ImGui::InputText("Encryption Password", &pw_str, ImGuiInputTextFlags_CallbackAlways,
                    [](ImGuiInputTextCallbackData* data) {
                        gPopPak->mPakName = pakName_str;
                        return 0;
                    });
                ImGui::InputText("GPAK output name", &pakName_str, ImGuiInputTextFlags_CallbackAlways,
                    [](ImGuiInputTextCallbackData* data) {
                        gPopPak->mPakName = pakName_str;
                        return 0;
                    });
                ImGui::InputText("Folder to package", &inputFolder_str);
                if (ImGui::Button("Open Folder..."))
                    SDL_ShowOpenFolderDialog(OnFolderSelected, nullptr, PopToolsGUI::mWindow, nullptr, false);
                ImGui::SetCursorPos(ImVec2(680, 560));
                if (ImGui::Button("Create GPAK"))
                {
                    gPopPak->mPassword = pw_str;
                    gPopPak->mPakName = pakName_str;
                    gPopPak->mPakFileOutputFolder = inputFolder_str;
                    gPopPak->Package();
                }
                ImGui::EndTabItem();
                }
            if (ImGui::BeginTabItem("Extract"))
            {     
                ImGui::InputText("GPAK to extract", &extractPak_str);
                if (ImGui::Button("Open Pak File..."))
                    SDL_ShowOpenFileDialog(OnPakFileSelected, nullptr, PopToolsGUI::mWindow, nullptr, 0, nullptr, false);
                ImGui::InputText("Output folder", &extractOutputFolder_str);
                if (ImGui::Button("Choose Output Folder..."))
                    SDL_ShowOpenFolderDialog(OnExtractFolderSelected, nullptr, PopToolsGUI::mWindow, nullptr, false);
                ImGui::SetCursorPos(ImVec2(680, 560));
                if (ImGui::Button("Extract GPAK"))
                {
                    gPopPak->mPassword = pw_str;
                    gPopPak->mUnpackOutputFolder = extractOutputFolder_str;
                    gPopPak->mPathToPack = extractPak_str;
                    gPopPak->Extract();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (gPopPak->mDoProgressBar)
        {
            ImGui::Begin("Progress", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Operation in progress...");
            ImGui::Text((gPopPak->mCurrentOperation + ": " + gPopPak->mCurrentProccesedFile).c_str());
            if (gPopPak->mGPAK->mTotalFiles > 0)
                ImGui::ProgressBar((float)gPopPak->mGPAK->mProcessedFiles / (float)gPopPak->mGPAK->mTotalFiles);
            ImGui::End();
        }
    }

#endif // __PAKTOOLWINDOW_HPP__