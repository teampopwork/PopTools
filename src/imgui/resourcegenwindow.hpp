#ifndef __RESGENWINDOW_HPP__
#define __RESGENWINDOW_HPP__
#ifdef _WIN32
#pragma once
#endif

using namespace PopLib;

#include "core/imgui.h"
#include "core/imgui_impl_sdl3.h"
#include "core/imgui_impl_sdlrenderer3.h"
#include "core/imgui_stdlib.h"
#include "SDL3/SDL.h"
#include "../poptoolsgui.hpp"
#include "../resourcetool.hpp"

std::string pathToXML{""};
std::string functionPrefix{""};

void OnXMLFileSelected(void* userdata, const char* const* selections, int count) {
    if (selections[0] != NULL)
        pathToXML = selections[0];
}



void DrawResourceGenTab()
{
    static ResourceManager* resManager = nullptr;
    if (!resManager)
        resManager = new ResourceManager();

    ImGui::InputText("Resource XML", &pathToXML);
    if (ImGui::Button("Choose XML file"))
        SDL_ShowOpenFileDialog(OnXMLFileSelected, nullptr, PopToolsGUI::mWindow, nullptr, 0, nullptr, false);
    ImGui::InputText("Function prefix", &functionPrefix);

    if (ImGui::Button("Generate Code"))
    {
        resManager->WriteSourceCode(pathToXML, functionPrefix);
    }
}


#endif // __RESGENWINDOW_HPP__