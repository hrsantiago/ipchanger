#include "version.h"
#include "headers.h"

bool VersionManager::load()
{
    FILE *fp = fopen("address.ini", "r");
    if(fp)
    {
        fscanf(fp, "%d\n", &mVersionsCount);
        
        for(uint8_t x = 0; x < mVersionsCount; x++)
        {
            Version *version = new Version();
            
            fscanf(fp, "\n%[^\n]", version->versionText);
            fscanf(fp, "%x", &version->versionAddress);
            fscanf(fp, "%d", &version->configId);
            fscanf(fp, "%d", &version->useRSA);
            
            if(version->useRSA)
                fscanf(fp, "%x", &version->rsaAddress);
                
            fscanf(fp, "%x", &version->ipAddress);
            fscanf(fp, "%x", &version->portAddress);
            fscanf(fp, "%d", &version->quantity);
            fscanf(fp, "%d", &version->distance);
            
            mVersions.push_back(version);
        }
        
        fclose(fp);
        return true;
    }
    else
    {
        alert("Error", "Could not open address.ini.");
        return false;
    }
}

bool VersionManager::save()
{
    FILE *fp = fopen("address.ini", "w");
    if(fp)
    {
        fprintf(fp, "%d\n", mVersions.size());
        
        for(std::vector<Version*>::iterator it = mVersions.begin(), end = mVersions.end(); it != end; it++)
        {
            fprintf(fp, "%s\n", (*it)->versionText);
            fprintf(fp, "%x\n", (*it)->versionAddress);
            fprintf(fp, "%d\n", (*it)->configId);
            fprintf(fp, "%d\n", (*it)->useRSA);
            
            if((*it)->useRSA)
                fprintf(fp, "%x\n", (*it)->rsaAddress);
                
            fprintf(fp, "%x\n", (*it)->ipAddress);
            fprintf(fp, "%x\n", (*it)->portAddress);
            fprintf(fp, "%d\n", (*it)->quantity);
            fprintf(fp, "%d\n", (*it)->distance);
        }
        
        fclose(fp);
        return true;
    }
    else
    {
        alert("Error", "Could not save address.ini.");
        return false;
    }
}

bool VersionManager::add(Version *version)
{
    mVersionsCount++;
    mVersions.push_back(version);
    if(!save())
        return false;
}

Version *VersionManager::get(uint8_t position)
{
    return mVersions[position];
}
