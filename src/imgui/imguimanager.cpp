#include "imguimanager.hpp"

using namespace PopWork;

ImGuiManager *PopWork::gImGuiManager = nullptr;

bool demoWind = false;

////////////////////////////

static std::vector<ImGuiWindowEntry> gWindowList;

void RegisterImGuiWindow(const char *name, bool *enabled, const ImGuiManager::WindowFunction &func)
{
	gWindowList.push_back({name, enabled, func});
}

void RegisterImGuiWindows()
{
	for (auto &entry : gWindowList)
	{
		if (entry.enabled && *entry.enabled)
		{
			gImGuiManager->AddWindow(entry.func);
		}
	}
}

////////////////////////////

ImGuiManager::ImGuiManager(SDLInstance theInstance)
{
	gImGuiManager = this;
	
	mSDLInstance = theInstance;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io; // uhhhhhh
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLRenderer(mSDLInstance.mWindow, mSDLInstance.mRenderer);
	ImGui_ImplSDLRenderer3_Init(mSDLInstance.mRenderer);
}

void ImGuiManager::RenderAll(void)
{
	// simple yet effective
	for (const auto &entry : gWindowList)
	{
		if (entry.enabled && *entry.enabled)
			entry.func();
	}
}

void ImGuiManager::Frame(void)
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	if (demoWind)
		ImGui::ShowDemoWindow();

	RenderAll();

	ImGui::Render();
}

ImGuiManager::~ImGuiManager()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}