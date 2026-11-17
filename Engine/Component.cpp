#include "Component.h"
#include "GameObject.h"

Component::Component(GameObject* owner, ComponentType type)
	: m_Owner(owner), m_Type(type)
{
}

bool Component::Init()
{
	return true;
}

void Component::Update(float deltaTime)
{

}