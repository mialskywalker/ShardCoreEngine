#pragma once

class Camera;
class GameObject;

enum class GizmoOperation
{
	Translate,
	Rotate,
	Scale
};

class EditorGizmo
{
public:
	void Draw(Camera& camera, GameObject* selectedObject, float viewportX, float viewportY, float viewportWidth, float viewportHeight, bool viewportHovered);

private:
	GizmoOperation m_Operation = GizmoOperation::Translate;
};