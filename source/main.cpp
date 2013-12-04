#include "config.h"
#include "headers.h"
#include "memory.h"
#include "version.h"

Memory *memory;

#ifdef WIN32
#include "window.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
    ConfigManager *config = new ConfigManager();
    if(!config->load())
        return -1;
        
    VersionManager *versionManager = new VersionManager();
    if(!versionManager->load())
        return -1;
        
    memory = new Memory(versionManager, config);
    
#ifdef WIN32
	initializeWindow(hInstance, hPrevInstance, szCmdLine, iCmdShow);
#else

	// TODO: linux support to memory search
	if(argc < 2)
		std::cout << "Usage:\nIPChanger ip port\n\nExample: IPChanger name.servegame.com 7171\n\n";
	else
	{
        memory->injectTibia();
		memory->changeIp(argv[1], atol(argv[2]));
		memory->desinjectTibia();
    }
#endif
	
	delete versionManager;
	delete config;
	delete memory;
	return 0;
}


