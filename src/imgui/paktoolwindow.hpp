#ifndef __DEBUGWINDOW_HPP__
#define __DEBUGWINDOW_HPP__
#ifdef _WIN32
#pragma once
#endif

using namespace PopWork;

#include "imguimanager.hpp"
#include "../paktool.hpp"
#include "core/imgui_stdlib.h"

bool doToolWindow = false;

bool &GetDebugWindowToggle()
{
    return doToolWindow;
}

std::string pw_str{"PopCapPopWorkFramework"};
std::string pakName_str{"main.gpak"};
std::string inputFolder_str{""};

std::string extractPak_str{""};
std::string extractOutputFolder_str{""};

void OnFolderSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        inputFolder_str = selections[0];
    gPopPak->SetInputFolderPath(inputFolder_str);
}

void OnExtractFolderSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        extractOutputFolder_str = selections[0];
}

void OnPakFileSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        extractPak_str = selections[0];
}

static struct RegisterDebugWindow
{
    RegisterDebugWindow()
    {
        RegisterImGuiWindow("MainWindow", &doToolWindow, [] {
            ImGuiIO& io = ImGui::GetIO();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("Window", &doToolWindow,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBringToFrontOnFocus);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
            ImGui::BeginChild("TitleBar", ImVec2(0, 30), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::SetCursorPos(ImVec2(10, 10));
            ImGui::Text("PopPak");
            ImGui::SetCursorPos(ImVec2(775, 5));
            if (ImGui::Button("X")) doToolWindow = false;
            ImGui::SetCursorPos(ImVec2(750, 5));
            if (ImGui::Button("-")) SDL_MinimizeWindow(gImGuiManager->mSDLInstance.mWindow);
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::InputText("Encryption Password", &pw_str, ImGuiInputTextFlags_CallbackAlways,
                [](ImGuiInputTextCallbackData* data) {
                    gPopPak->SetPassword(pw_str);
                    return 0;
                });

            ImGui::InputText("GPAK output name", &pakName_str, ImGuiInputTextFlags_CallbackAlways,
                [](ImGuiInputTextCallbackData* data) {
                    gPopPak->SetPakName(pakName_str);
                    return 0;
                });

            ImGui::InputText("Folder to package", &inputFolder_str, ImGuiInputTextFlags_ReadOnly);
            if (ImGui::Button("Open Folder..."))
                SDL_ShowOpenFolderDialog(OnFolderSelected, nullptr, gImGuiManager->mSDLInstance.mWindow, nullptr, false);

            ImGui::SetCursorPos(ImVec2(680, 300));
            if (ImGui::Button("Create GPAK"))
            {
                gPopPak->SetPassword(pw_str);
                gPopPak->SetPakName(pakName_str);
                gPopPak->SetInputFolderPath(inputFolder_str);
                gPopPak->Package();
            }

            ImGui::Separator();
            ImGui::Text("Extraction");

            ImGui::InputText("GPAK to extract", &extractPak_str, ImGuiInputTextFlags_ReadOnly);
            if (ImGui::Button("Open Pak File..."))
                SDL_ShowOpenFileDialog(OnPakFileSelected, nullptr, gImGuiManager->mSDLInstance.mWindow, nullptr, 0, nullptr, false);

            ImGui::InputText("Output folder", &extractOutputFolder_str, ImGuiInputTextFlags_ReadOnly);
            if (ImGui::Button("Choose Output Folder..."))
                SDL_ShowOpenFolderDialog(OnExtractFolderSelected, nullptr, gImGuiManager->mSDLInstance.mWindow, nullptr, false);

            ImGui::SetCursorPos(ImVec2(680, 560));
            if (ImGui::Button("Extract GPAK"))
            {
                if (!extractPak_str.empty() && !extractOutputFolder_str.empty())
                    gPopPak->Extract(extractPak_str, extractOutputFolder_str);
            }

            ImGui::End();

            if (gPopPak->mDoProgressBar)
            {
                ImGui::Begin("Progress");
                ImGui::Text("Operation in progress...");
                ImGui::End();
            }
        });
    }
} registerDebugWindow;

#endif // __DEBUGWINDOW_HPP__