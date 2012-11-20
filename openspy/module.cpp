#include "module.h"
CModule::CModule(char *name,modLoadOptions *options) {
	running = false;
	this->options = (modLoadOptions *)malloc(sizeof(modLoadOptions));
	memcpy(this->options,options,sizeof(modLoadOptions));
#ifdef _WIN32
	modpointer = LoadLibrary(name);
	if(modpointer) {
		infoProc = (modInfo *(*)())GetProcAddress(modpointer,"openspy_modInfo");
		if(infoProc) {
			moduleInfo = infoProc();
		}
		entryPoint = (void (*)(void *))GetProcAddress(modpointer,"openspy_mod_run");		
		modQuery = (bool (*)(char *, void *,int))GetProcAddress(modpointer,"openspy_mod_query");	
	}
#else
	modpointer = dlopen(name,RTLD_LAZY);
	if(modpointer) {
		infoProc = (modInfo *(*)())dlsym(modpointer,"openspy_modInfo");
		if(infoProc) {
			moduleInfo = infoProc();
		}
		entryPoint = (void (*)(void *))dlsym(modpointer,"openspy_mod_run");		
		modQuery = (bool (*)(char *, void *,int))dlsym(modpointer,"openspy_mod_query");
	}
#endif
}
CModule::~CModule() {
	if(modpointer) {
		#ifdef _WIN32
		FreeLibrary(modpointer);
		#else
		dlclose(modpointer);
		#endif
	}
	if(options) {
		free((void *)options);
	}
}
void CModule::start() {
#ifndef _WIN32
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(!pthread_create(&threadID, &attr, (void* (*)(void*))entryPoint, ((void *)options))) {
	running = true;
    }
#else
	threadHandle = CreateThread(0,0,(LPTHREAD_START_ROUTINE)entryPoint,(LPVOID)options,0,&threadID);
	if(threadHandle) running = true;
#endif
}
void CModule::stop() {
	running = false;
#ifdef _WIN32
	TerminateThread(threadHandle,0);
#else
	pthread_kill(threadID,SIGKILL);
#endif
}
#ifdef _WIN32
DWORD
#else
pthread_t
#endif
CModule::getThreadID() {
	return threadID;
}
modInfo *CModule::getModInfo() {
	return moduleInfo;
}
bool CModule::sendModuleMessage(char *sendername, void *data, int len) {
	if(modQuery) {
		return modQuery(sendername,data,len);
	}
	return false;
}
