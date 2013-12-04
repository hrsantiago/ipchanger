#ifndef ADDRESS_H
#define ADDRESS_H

#include "headers.h"

struct Version
{
    char versionText[13];
    uint32_t versionAddress;
    uint32_t rsaAddress;
    uint32_t ipAddress, portAddress;
    uint32_t quantity, distance;
    uint32_t configId, useRSA;
};

class VersionManager
{
    public:
        bool load();
        bool save();
        bool add(Version *version);
        Version *get(uint8_t position);
        uint8_t getVersionsCount() { return mVersionsCount; }
        
    private:
        uint8_t mVersionsCount;
        std::vector<Version*> mVersions;
};

#endif
