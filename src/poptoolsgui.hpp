#ifndef __POPTOOLSGUI_HPP__
#define __POPTOOLSGUI_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <SDL3/SDL.h>

class PopToolsGUI
{
public:
    PopToolsGUI() = default;
    ~PopToolsGUI() = default;

    static SDL_Window* mWindow;
    static SDL_Renderer* mRenderer;
};

inline SDL_Window* PopToolsGUI::mWindow = nullptr;
inline SDL_Renderer* PopToolsGUI::mRenderer = nullptr;

#endif