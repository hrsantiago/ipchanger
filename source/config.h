#ifndef CONFIG_H
#define CONFIG_H

#include "headers.h"

struct Config
{
    char host[128];
    char rsaOts[310], rsaCip[310];
    uint32_t port, quantity, distance, useRSA;
};

class ConfigManager
{
    public:
        bool load();
        Config *get(uint8_t position);
        uint8_t getConfigsCount() { return mConfigsCount; }
        
    private:
        uint8_t mConfigsCount;
        std::vector<Config*> mConfigs;
};

#endif
