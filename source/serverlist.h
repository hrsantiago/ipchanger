#ifdef WIN32

#ifndef SERVERLIST_H
#define SERVERLIST_H

#include "headers.h"

struct Server
{
	char ip[256];
	uint32_t port;
	
	std::string text;
};

class ServerList
{
	public:
		bool load();
		bool save();
		uint32_t getMaxIps() { return mMaxIps; }
		uint32_t getStoredIps() { return mStoredIps; }
		std::list<Server*> getServerList() { return mServerList; }
		uint8_t add(Server *server);
		
	private:
		uint32_t mMaxIps, mStoredIps;
		std::list<Server*> mServerList;
};

#endif

#endif
