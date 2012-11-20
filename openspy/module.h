#ifndef _CMOD_INC
#define _CMOD_INC
#include "main.h"
class CModule {
	public:
	CModule(char *name,modLoadOptions *options);
	~CModule();
	bool sendModuleMessage(char *sendername, void *data, int len);
	void start();
	void stop();
#ifdef _WIN32
		DWORD
#else
	pthread_t 
#endif
	getThreadID();
	modInfo *getModInfo();
	private:
#ifdef _WIN32
		HANDLE threadHandle;
		HINSTANCE modpointer;
#else
	void *modpointer;
#endif
	modLoadOptions *options;
	modInfo *(*infoProc)();
	void (*entryPoint)(void *);
	bool (*modQuery)(char *, void *,int);
	modInfo *moduleInfo;
#ifdef _WIN32
	DWORD
#else
pthread_t 
#endif
	threadID;
	bool running;
};
#endif
