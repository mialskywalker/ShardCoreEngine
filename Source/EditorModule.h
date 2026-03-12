#pragma once

#include "Module.h"
#include "ImGuizmo.h"

class ImGuiPass;
class D3D12Module;

class EditorModule : public Module
{
public:

	EditorModule();
	~EditorModule();

public:

	bool init() override;
	void update() override;
	void preRender() override;
	void render() override;
	void mainSettings();

private:

	ImGuiPass* imGuiPass = nullptr;

	//ImGuizmo::OPERATION guizmoOp = ImGuizmo::TRANSLATE;
	//ImGuizmo::MODE guizmoMode = ImGuizmo::LOCAL;

	static const int HISTORY_SIZE = 100;
	float fpsHistory[HISTORY_SIZE] = {};
	float msHistory[HISTORY_SIZE] = {};
	int historyIndex = 0;
	bool vsync = false;
	//bool camera = true;

	//float fov = 1.0f;
	//int samplerIndex = 0;

public:

	ImGuiPass* getImGui() { return this->imGuiPass; }

	//const int& getSamplerIndex() const { return samplerIndex; }
	//void setSamplerIndex(int& index) { samplerIndex = index; }
};