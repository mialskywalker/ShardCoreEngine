#pragma once

class Module
{
public:
	
	Module() {}
	~Module() {}

	virtual bool Init() = 0;
	virtual void Update();
	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();
};