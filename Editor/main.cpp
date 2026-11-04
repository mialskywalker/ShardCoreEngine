#include "EditorApplication.h"

int main()
{
	EditorApplication app;
	
	if (!app.Init())
	{
		return 1;
	}

	app.Run();

	return 0;
}