#include "config.h"

bool ConfigManager::load()
{
    FILE *fp = fopen("config.ini", "r");
    if(fp)
    {
        fscanf(fp, "%d", &mConfigsCount);
        
        for(uint8_t x = 0; x < mConfigsCount; x++)
        {
            Config *cfg = new Config();
            
            fscanf(fp, "%s", cfg->host);
		    fscanf(fp, "%d", &cfg->port);
		    fscanf(fp, "%d", &cfg->quantity);
		    fscanf(fp, "%d", &cfg->distance);
		    fscanf(fp, "%d", &cfg->useRSA);
		    if(cfg->useRSA)
		    {
		      fscanf(fp, "%s", cfg->rsaCip);
		      fscanf(fp, "%s", cfg->rsaOts);
            }
            
		    mConfigs.push_back(cfg);
        }
        
        fclose(fp);
        return true;
    }
    else
    {
        alert("Error", "Could not load config.ini.");
        return false;
    }
}

Config *ConfigManager::get(uint8_t position)
{
    return mConfigs[position];
}
