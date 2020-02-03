#include "GameApp.h"


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
				   _In_ LPSTR cmdLine, _In_ int showCmd) {
	
	GameApp theApp(hInstance);
	
	if (!theApp.Init()) {
		return 0;
	}
	
	return theApp.Run();
}




