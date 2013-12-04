#ifndef MEMORY_H
#define MEMORY_H

#include "config.h"
#include "headers.h"
#include "version.h"
#include <vector>

#ifdef WIN32
struct CodeSection
{
    /*
 0x00   8 bytes Section Name - in the above example the names are .text .data .bss
 0x08   4 bytes Size of the section once it is loaded to memory
 0x0C   4 bytes RVA (location) of section once it is loaded to memory
 0x10   4 bytes Physical size of section on disk
 0x14   4 bytes Physical location of section on disk (from start of disk image)
 0x18  12 bytes Reserved (usually zero) (used in object formats)
 0x24   4 bytes Section flags
    */
    
    char name[8];
    uint32_t size;
    uint32_t RVALocation;
    uint32_t physicalSize, physicalLocation;
    uint8_t reserved[12];
    uint32_t flags;
};
#endif

class Memory
{
	public:
        Memory(VersionManager *versionManager, ConfigManager *config);
        
        bool injectTibia();
        void desinjectTibia();
        
    	bool changeIp(const char *ip, const uint16_t &port);
    	
    
	private:
		bool attachProcess();
        void detachProcess();
        
        bool findTibiaProcess();
        void reallyChangeIp(Version *version, const char *ip, const uint16_t &port);
        Version *searchMemory();
        void readHeader();

        bool readProcessMemory(uint32_t address, uint32_t size, void *buffer, bool protection);
        bool writeProcessMemory(uint32_t address, uint32_t size, const void *buffer, bool protection);
    
		VersionManager *mVersionManager;
		ConfigManager *mConfigManager;
		
#ifdef WIN32
        HWND mTibiaWnd;
        DWORD mProcessId;
        HANDLE mProcess;
        
        std::vector<CodeSection*> mCodeSections;
#else
        pid_t mProcessId;
        int mProcess;
#endif

};

#endif
