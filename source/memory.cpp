#include "memory.h"

Memory::Memory(VersionManager *versionManager, ConfigManager *configManager)
{
    mVersionManager = versionManager;
    mConfigManager = configManager;

    mProcessId = 0;
    mProcess = 0;
}

Version *Memory::searchMemory()
{
    uint32_t pos;
    Version *version = new Version();
    bool found = false;
    
    // Read version text and address ( its at .rsrc section)
    char versionText[16];
    
    // Version 7.6 address is different from 8.4 so check them all.
    for(uint8_t x = 0; x < mCodeSections.size(); x++)
    {
    	for(pos = 0x400000 + mCodeSections[x]->RVALocation; pos < 0x400000 + mCodeSections[x]->RVALocation + mCodeSections[x]->size; pos++)
    	{
        	readProcessMemory(pos, 8, versionText, false);
        	if(strncmp(versionText, "Version ", 8) == 0)
        	{
            	readProcessMemory(pos, 12, version->versionText, false);
            	if(version->versionText[8] == '%' || version->versionText[8] == '=')
                	continue;
            	version->versionAddress = pos;
            	found = true;
            	break;
        	}
		}
		if(found)
			break;
    }
    if(!found)
    {
        delete version;
        alert("Error", "Strange error while reading memory.");
        return NULL;
    }
    printf("Debug: Found version name.\n");
    found = false;
    
    // Check all configurations, if we have more than one.
    for(uint32_t config = 0; config < mConfigManager->getConfigsCount(); config++)
    {
        // Check IP
        char ipText[128], hostText[128];
        sprintf(hostText,"%s",mConfigManager->get(config)->host);
        uint8_t length = strlen(hostText);
        for(pos = 0x400000 + mCodeSections[2]->RVALocation; pos < 0xFFFFFF; pos++)
        {
            readProcessMemory(pos, length, ipText, false);
            if(strncmp(ipText, hostText, length) == 0)
            {
                version->ipAddress = pos;
                found = true;
                break;
            }
        }
        // If not found ip address, go to another config.
        if(!found)
            continue;
            
        found = false;
        printf("Debug: Found ip address.\n");
        
        // Check port
        uint32_t port, realPort = mConfigManager->get(config)->port;
        for(pos = 0x400000 + mCodeSections[2]->RVALocation; pos < 0xFFFFFF; pos++)
        {
            readProcessMemory(pos, 4, &port, false);
            if(port == realPort)
            {
                version->portAddress = pos;
                found = true;
                break;
            }
        }
        if(!found)
            continue;
            
        found = false;
        printf("Debug: Found port address.\n");
        
        // Check RSA ( its at .rdata section )
        if(mConfigManager->get(config)->useRSA)
        {
            char rsaText[309], rsaCip[309];
            sprintf(rsaCip,"%s",mConfigManager->get(config)->rsaCip);
            for(pos = 0x400000 + mCodeSections[1]->RVALocation; pos < 0x400000 + mCodeSections[1]->RVALocation + mCodeSections[1]->size; pos++)
            {
                readProcessMemory(pos, 10, rsaText, true);
                if(strncmp(rsaText, rsaCip, 10) == 0)
                {
                    version->rsaAddress = pos;
                    found = true;
                    break;
                }
            }
            if(!found)
                continue;
                
            printf("Debug: Found RSA address.\n");
        }
        
        version->quantity = mConfigManager->get(config)->quantity;
        version->distance = mConfigManager->get(config)->distance;
        version->useRSA = mConfigManager->get(config)->useRSA;
        version->configId = config;
        
        mVersionManager->add(version);
        
        return version;
    }
    
    alert("Error", "Update your config.ini.");
    delete version;
    return NULL;
}

void Memory::reallyChangeIp(Version *version, const char *ip, const uint16_t &port)
{
    // Change IP and Port.
    for(uint8_t x = 0; x < version->quantity; x++)
    {
        writeProcessMemory((version->ipAddress + version->distance * x), strlen(ip) + 1, ip, false);
        writeProcessMemory((version->portAddress + version->distance * x), 4, &port, false);
    }
            
    // Change RSA
    if(version->useRSA)
    {
        Config *config = mConfigManager->get(version->configId);
        writeProcessMemory(version->rsaAddress, strlen(config->rsaOts), config->rsaOts, true);   
    }
    
#ifdef WIN32
    char windowTitle[256];
    sprintf(windowTitle,"Tibia - %s:%d",ip,port);
    SetWindowText(mTibiaWnd, windowTitle);
#endif
}

void Memory::readHeader()
{
#ifdef WIN32
    
    uint32_t pos = 0x400000; // Offset to PE header
    
    uint32_t peHeaderOffset = 0;
    readProcessMemory(pos + 0x3C, 4, &peHeaderOffset, false);
    
    pos += peHeaderOffset;
    
    pos += 4; // PE signature
    pos += 2; // Machine
    
    uint16_t numberOfSections = 4;
    readProcessMemory(pos, 2, &numberOfSections, false);
    pos += 2; // Number of Sections
    
    pos += 0xF0; // Get to sections

    for(uint8_t x = 0; x < numberOfSections; x++)
    {
        CodeSection *codeSection = new CodeSection();
        mCodeSections.push_back(codeSection);
        
        readProcessMemory(pos, 8, mCodeSections[x]->name, false);
        pos += 8;
        
        readProcessMemory(pos, 4, &mCodeSections[x]->size, false);
        pos += 4;
        
        readProcessMemory(pos, 4, &mCodeSections[x]->RVALocation, false);
        pos += 4;
        
        readProcessMemory(pos, 4, &mCodeSections[x]->physicalSize, false);
        pos += 4;
        
        readProcessMemory(pos, 4, &mCodeSections[x]->physicalLocation, false);
        pos += 4;

        readProcessMemory(pos, 12, mCodeSections[x]->reserved, false);
        pos += 12;
        
        readProcessMemory(pos, 4, &mCodeSections[x]->flags, false);
        pos += 4;
        
        if(x > 0)
            mCodeSections[x-1]->size = mCodeSections[x]->RVALocation - mCodeSections[x-1]->RVALocation;
    }

#endif
}

bool Memory::changeIp(const char *ip, const uint16_t &port)
{
    if(mProcessId == 0 || !attachProcess())
        return false;  
    
    // Read the tibia's pe header
    readHeader();
    
    // Check if we already have this version on address.ini
    Version *version;
    char versionText[16];
    for(uint8_t x = 0; x < mVersionManager->getVersionsCount(); x++)
    {
        version = mVersionManager->get(x);
        readProcessMemory(version->versionAddress, 12, versionText, false);
        if(strncmp(version->versionText, versionText, 12) == 0)
        {
            reallyChangeIp(version, ip, port);
            detachProcess();
            return true;
        }
    }
    
    // Search for addresses of this version.
    alert("Alert", "You do not have this version in your address.ini file.\nIt'll take some time to search for it.\nPress OK and wait.\n\n"
    "Voce nao possui essa versao. O programa ira procurar para voce. Aperte OK e aguarde alguns minutos.");
    version = searchMemory();
    if(version)
    {
        reallyChangeIp(version, ip, port);
        detachProcess();
        return true;
    }
        
    detachProcess();
    return false;
}

bool Memory::injectTibia()
{
    if(!findTibiaProcess())
        return false;

    if(!attachProcess())
        return false;

    detachProcess();
    return true;
}

void Memory::desinjectTibia()
{
    if(mProcessId == 0)
        return;
        
    detachProcess();
    mProcessId = 0;
}

bool Memory::findTibiaProcess()
{
#ifdef WIN32
    mTibiaWnd = FindWindowA((LPCSTR)"TibiaClient",NULL);
    if(!mTibiaWnd)
    {
        alert("Error", "You must open your Tibia client first.");
        return false;
    }

    GetWindowThreadProcessId(mTibiaWnd, &mProcessId);
#else
    PidList pidList = findPidsByName("Tibia");
    if(pidList.size() == 0 || pidList.begin() == pidList.end())
    {
        alert("Error", "You must open your Tibia client first.");
        return false;
    }

    mProcessId = (*pidList.begin());
#endif
    return true;
}

bool Memory::attachProcess()
{
#ifdef WIN32
    mProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, mProcessId);
    if(!mProcess)
    {
        alert("Error", "Could not open Tibia process.");
        return false;
    }
#else
    if(ptrace(PTRACE_ATTACH, mProcessId, NULL, NULL))
    {
        alert("Error", "Could not open Tibia process.");
        return false;
    }

    wait(NULL);
    mProcess = 1;
#endif
    return true;
}

void Memory::detachProcess()
{
#ifdef WIN32
    if(!mProcess)
        return;

    CloseHandle(mProcess);
    mProcess = NULL;
#else
    if(!mProcess)
        return;

    ptrace(PTRACE_DETACH, mProcessId, NULL, NULL);
#endif
}

bool Memory::readProcessMemory(uint32_t address, uint32_t size, void *buffer, bool protection)
{
    if(!mProcess)
        return false;
#ifdef WIN32
    if(protection)
    {
        DWORD oldProtection;
        VirtualProtectEx(mProcess, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtection);
    }

    if((bool)!ReadProcessMemory(mProcess, (LPVOID)address, buffer, size, NULL))
        return false;
#else
    //read memory every 4 bytes and puts in the buffer
    long val;
    const static int long_size = sizeof(long);
    for(unsigned int i=0;i < size/long_size + (size % long_size > 0) ? 1 : 0; i++) {
        val = ptrace(PTRACE_PEEKDATA, mProcessId, (void *)(address+i*long_size), NULL);
        memcpy((void*)(((char*)buffer)+i*long_size),&val, std::min(long_size, (int)(size-i*4)));
    }
#endif
    return true;
}

bool Memory::writeProcessMemory(uint32_t address, uint32_t size, const void *buffer, bool protection)
{
    if(!mProcess)
        return false;
#ifdef WIN32
    if(protection)
    {
        DWORD oldProtection;
        VirtualProtectEx(mProcess, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtection);
    }
    
    if((bool)!WriteProcessMemory(mProcess, (LPVOID)address, buffer, size, NULL))
        return false;
#else
    //write the memory every 4 btyes from buffer
    //TODO: check for additional writes when writing memory with size < 4
    long val;
    const static int long_size = sizeof(long);
    for(unsigned int i=0;i < size/long_size + (size % long_size > 0) ? 1 : 0; i++) {
        memcpy(&val,(void*)(((char*)buffer)+i*long_size), std::min(long_size, (int)(size-i*4)));
        ptrace(PTRACE_POKEDATA, mProcessId, (void *)(address+i*long_size), val);
    }
#endif
    return true;
}
