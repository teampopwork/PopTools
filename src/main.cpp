#include <iostream>
#include "poppakgui.hpp"
#include "imgui/imguimanager.hpp"
#include "imgui/paktoolwindow.hpp"

using namespace PopLib;

extern bool doToolWindow;
extern bool demoWind;

bool isBeingDragged = false;

int main(int, char**){
    SDL_Init(SDL_INIT_VIDEO);
    SDLInstance instance = {};
    instance.mWindow =  SDL_CreateWindow("PopPak", 800, 600, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE );

    instance.mRenderer = SDL_CreateRenderer(instance.mWindow, nullptr);

    PopPak* aPopPak = new PopPak();
    aPopPak->mDoProgressBar = false;
    ImGuiManager* aManager = new ImGuiManager(instance);
    RegisterImGuiWindows();

    doToolWindow = true;
    demoWind = false;

    int dragOffsetX = 0, dragOffsetY = 0;

    while (doToolWindow)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                doToolWindow = false;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    float mouseX, mouseY;
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    int winX, winY;
                    SDL_GetWindowPosition(instance.mWindow, &winX, &winY);

                    // Restrict the dragging to a 30 pixel zone
                    int localX = event.button.x;
                    int localY = event.button.y;
                    if (localY < 30) {
                        isBeingDragged = true;
                        dragOffsetX = mouseX - winX;
                        dragOffsetY = mouseY - winY;
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    isBeingDragged = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (isBeingDragged)
                {
                    float mouseX, mouseY;
                    SDL_GetGlobalMouseState(&mouseX, &mouseY); // Global mouse position
                    SDL_SetWindowPosition(instance.mWindow, mouseX - dragOffsetX, mouseY - dragOffsetY);
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode ==SDL_SCANCODE_F1)
                    demoWind = !demoWind;
            
            }
        }

        aManager->Frame();

        SDL_SetRenderTarget(instance.mRenderer, nullptr);

        SDL_SetRenderDrawColor(instance.mRenderer, 0, 0, 0, 0);
        SDL_RenderClear(instance.mRenderer);

        if (ImGui::GetDrawData() != nullptr)
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), instance.mRenderer);
        SDL_RenderPresent(instance.mRenderer);
    }

    SDL_DestroyRenderer(instance.mRenderer);
    SDL_DestroyWindow(instance.mWindow);
    SDL_Quit();

    delete aManager;

    return 0;
}
