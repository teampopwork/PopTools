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

void OnFolderSelected(void* userdata, const char* const* selections, int count) {
	if (selections[0] != NULL)
    	inputFolder_str = selections[0];
	if (inputFolder_str.empty())
		inputFolder_str = "";
	gPopPak->SetInputFolderPath(inputFolder_str);
}

static struct RegisterDebugWindow
{
	RegisterDebugWindow()
	{
		RegisterImGuiWindow("MainWindow", &doToolWindow, [] {
			ImGuiIO& io = ImGui::GetIO();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); //Make sure there is NO padding

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(io.DisplaySize);
			ImGui::Begin("Window", &doToolWindow,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBringToFrontOnFocus);

			ImVec2 titleBarSize = ImVec2(ImGui::GetWindowWidth(), 30.0f);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
			ImGui::BeginChild("TitleBar", ImVec2(0, 30), false, ImGuiWindowFlags_NoScrollbar);
			ImGui::SetCursorPos(ImVec2(10, 10));
			ImGui::Text("PopPak");
			ImGui::SetCursorPos(ImVec2(775, 5));
			if (ImGui::Button("X"))
			{
				doToolWindow = false;
			}
			ImGui::SetCursorPos(ImVec2(750, 5));
			if (ImGui::Button("-"))
			{
				SDL_MinimizeWindow(gImGuiManager->mSDLInstance.mWindow);
			}
			ImGui::EndChild();
			ImGui::PopStyleColor(); // Restore background color
			ImGui::PopStyleVar();//Now we can have padding and more.

	    	ImGui::InputText("Encryption Password",
			&pw_str,
			ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int {
        	gPopPak->SetPassword(pw_str);
        	return 0;
    		});

	    	ImGui::InputText("Pak output name",
			&pakName_str,
			ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int {
        	gPopPak->SetPakName(pakName_str);
        	return 0;
    		});

			if (ImGui::Button("Open File Dialog"))
			{
				SDL_ShowOpenFolderDialog(OnFolderSelected, nullptr, gImGuiManager->mSDLInstance.mWindow, nullptr, false);
			}

	    	ImGui::InputText("Folder to package",
			&inputFolder_str,
			ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int {
        	gPopPak->SetInputFolderPath(inputFolder_str);
        	return 0;
    		});


			ImGui::SetCursorPos(ImVec2(680, 560));
			if (ImGui::Button("Create GPAK"))
			{
				//Ensure the values are set.
				gPopPak->SetPassword(pw_str);
				gPopPak->SetPakName(pakName_str);
				gPopPak->SetInputFolderPath(inputFolder_str);
				gPopPak->Package();
			}

			ImGui::End();
			if (gPopPak->mDoProgressBar)
			{
				ImGui::Begin("Packing");

				ImGui::Text("Packing in progress");

				ImGui::End();
			}
		});
	}
} registerDebugWindow;

#endif