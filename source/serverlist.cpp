#ifdef WIN32

#include "serverlist.h"

bool ServerList::load()
{
	FILE *fp = fopen("serverlist.ini","r");
	if(fp)
	{
		fscanf(fp, "%d", &mMaxIps);
		fscanf(fp, "%d", &mStoredIps);
		
		for(uint32_t x = 0; x < mStoredIps; x++)
		{
			Server *server = new Server();
			
			fscanf(fp, "%s", server->ip);
			fscanf(fp, "%d", &server->port);
			
			char tmp[8];
			ltoa(server->port, tmp, 10);
			server->text = server->ip;
			server->text += " - ";
			server->text += tmp;
			
			mServerList.push_back(server);
		}
		
		fclose(fp);
	}
	else
	{
		Server *server = new Server();
		sprintf(server->ip, "127.0.0.1");
		server->port = 7171;
		
		mMaxIps = 10;
		mStoredIps = 1;
		
		mServerList.clear();
		mServerList.push_back(server);
		
		if(!save())
			return false;
	}
	
	return true;
}

bool ServerList::save()
{
	FILE *fp = fopen("serverlist.ini","w");
	if(fp)
	{
		fprintf(fp, "%d\n", mMaxIps);
		fprintf(fp, "%d\n", mStoredIps);
		for(std::list<Server*>::iterator it = mServerList.begin(), end = mServerList.end(); it != end; it++)
			fprintf(fp, "%s %d\n", (*it)->ip, (*it)->port);
			
		fclose(fp);
		return true;
	}
	else
	{
		alert("Error", "Could save iplist.ini.");
		return false;
	}
}

uint8_t ServerList::add(Server *server)
{
	for(std::list<Server*>::iterator it = mServerList.begin(), end = mServerList.end(); it != end; it++)
	{
		if(strncmp((*it)->ip, server->ip, strlen(server->ip)) == 0 && (*it)->port == server->port)
		{
			delete server;
			return 0;
		}
	}

	char tmp[8];
	ltoa(server->port, tmp, 10);
	server->text = server->ip;
	server->text += " - ";
	server->text += tmp;
	
	if(mStoredIps < mMaxIps)
	{
		mStoredIps++;
		mServerList.push_front(server);
		save();
		return 1;
	}
	else
	{
		delete mServerList.back();
		mServerList.pop_back();
		mServerList.push_front(server);
		save();
		return 2;
	}
}

#endif
