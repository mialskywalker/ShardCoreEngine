#pragma once

#include "Module.h"
#include "ImGuiPass.h"

class Test1 : public Module
{
public:

	bool init();
	void render() override;

private:
	
	ImGuiPass* imgui = nullptr;
};