#include <iostream>
#include "poptoolsgui.hpp"
#include "imgui/core/imgui.h"
#include "imgui/core/imgui_impl_sdl3.h"
#include "imgui/core/imgui_impl_sdlrenderer3.h"
#include "imgui/paktoolwindow.hpp"
#include "imgui/resourcegenwindow.hpp"

using namespace PopLib;

extern bool doToolWindow;

bool isBeingDragged = false;

int main(int, char**){
    SDL_Init(SDL_INIT_VIDEO);

    PopToolsGUI gui;
    
    PopToolsGUI::mWindow =  SDL_CreateWindow("PopTools", 800, 600, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE );

    PopToolsGUI::mRenderer = SDL_CreateRenderer(PopToolsGUI::mWindow, nullptr);

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLRenderer(PopToolsGUI::mWindow, PopToolsGUI::mRenderer);
	ImGui_ImplSDLRenderer3_Init(PopToolsGUI::mRenderer);

    PopPak* aPopPak = new PopPak();
    aPopPak->mDoProgressBar = false;

    doToolWindow = true;
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
                    SDL_GetWindowPosition(PopToolsGUI::mWindow, &winX, &winY);

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
                    SDL_SetWindowPosition(PopToolsGUI::mWindow, mouseX - dragOffsetX, mouseY - dragOffsetY);
                }
                break;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
	    ImGui_ImplSDL3_NewFrame();
	    ImGui::NewFrame();

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
        ImGui::Text("PopTools");
        ImGui::SetCursorPos(ImVec2(775, 5));
        if (ImGui::Button("X")) doToolWindow = false;
        ImGui::SetCursorPos(ImVec2(750, 5));
        if (ImGui::Button("-")) SDL_MinimizeWindow(PopToolsGUI::mWindow);
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();


        if (ImGui::BeginTabBar("MainTabs")) {

                if (ImGui::BeginTabItem("PopPak")) {
                    DrawPoppakTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("ResourceGen")) {
                    DrawResourceGenTab();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        ImGui::End();

        ImGui::Render();

        SDL_SetRenderTarget(PopToolsGUI::mRenderer, nullptr);

        SDL_SetRenderDrawColor(PopToolsGUI::mRenderer, 0, 0, 0, 0);
        SDL_RenderClear(PopToolsGUI::mRenderer);

        if (ImGui::GetDrawData() != nullptr)
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), PopToolsGUI::mRenderer);
        SDL_RenderPresent(PopToolsGUI::mRenderer);
    }

    SDL_DestroyRenderer(PopToolsGUI::mRenderer);
    SDL_DestroyWindow(PopToolsGUI::mWindow);
    SDL_Quit();

    ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

    return 0;
}
