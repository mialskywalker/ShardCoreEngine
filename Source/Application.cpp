#include "Common.h"
#include "Application.h"
#include "InputModule.h"
#include "D3D12Module.h"
#include "EditorModule.h"
#include "Test1.h"

Application::Application(int argc, wchar_t** argv, void* hWnd)
{
	modules.push_back(new InputModule((HWND)hWnd));
	modules.push_back(d3d12 = new D3D12Module((HWND)hWnd));
	modules.push_back(editor = new EditorModule());
	modules.push_back(new Test1());
}

Application::~Application()
{
	cleanUp();

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
	{
		delete* it;
	}
}

bool Application::init()
{
	bool ok = true;

	for (auto it = modules.begin(); it != modules.end() && ok; ++it)
		ok = (*it)->init();

	lastMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	return ok;
}

void Application::update()
{
	using namespace std::chrono_literals;

	// Update ms
	uint64_t currentMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	elapsedMs = currentMs - lastMs;
	lastMs = currentMs;
	tickSum -= tickList[tickIndex];
	tickSum += elapsedMs;
	tickList[tickIndex] = elapsedMs;
	tickIndex = (tickIndex + 1) % MAX_FPS_TICKS;

	if (!app->paused)
	{
		for (auto it = modules.begin(); it != modules.end(); ++it)
			(*it)->update();

		for (auto it = modules.begin(); it != modules.end(); ++it)
			(*it)->preRender();

		for (auto it = modules.begin(); it != modules.end(); ++it)
			(*it)->render();

		for (auto it = modules.begin(); it != modules.end(); ++it)
			(*it)->postRender();
	}
}

bool Application::cleanUp()
{
	bool ok = true;

	for (auto it = modules.rbegin(); it != modules.rend() && ok; ++it)
		ok = (*it)->cleanUp();

	return ok;
}