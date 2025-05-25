#ifndef __POPPAKGUI_HPP__
#define __POPPAKGUI_HPP__
#ifdef _WIN32
#pragma once
#endif

#include <SDL3/SDL.h>

struct SDLInstance
{
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
};


#endif