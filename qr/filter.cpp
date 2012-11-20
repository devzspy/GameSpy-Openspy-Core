#include "Client.h"
#include "filter.h"
enum EOperatorType {
	EOperatorType_None,
	EOperatorType_Equals,
};
char *findfirstoperator(char *filter) {
	int len = strlen(filter);
	char *firstEquals;
	char *nextSpace;
	firstEquals = find_first(filter,'=',len);
	nextSpace = find_first(filter,' ',len);	
	return firstEquals>nextSpace?firstEquals:nextSpace;
}
bool findNextVariable(char **filter, char *dst, int dstlen) {
	char *p = findfirstoperator(*filter);
	if(p == NULL) return false;
	memset(dst,0,dstlen);
	strncpy(dst,*filter,p-(*filter));
	*filter = p;
	return true;
}
EOperatorType findNextOperator(char **filter) {
	char *p = findfirstoperator(*filter);
	if(p != NULL) *filter = p+1;
	else return EOperatorType_None;
	if(*p == '=') {
		return EOperatorType_Equals;
	}
	return EOperatorType_None;
}
bool findNextValue(char **filter, char *dst, int dstlen) {
	char *p = findfirstoperator(*filter);
	if(p == NULL) {
		p = *filter;
		memset(dst,0,dstlen);
		strcpy(dst,*filter);
		return true;
	}
	memset(dst,0,dstlen);
	strncpy(dst,*filter,p-(*filter));
	*filter = p+1;
	return true;
}
bool filterMatches(uint8_t *filter, Client *server) {
	char variable[256];
	char value[256];
	char *result;
	char *p = (char *)filter;
	if(filter == NULL) return true;
	if(findNextVariable((char **)&p,(char *)&variable,sizeof(variable))) {
		EOperatorType oper = findNextOperator((char **)&p);
		findNextValue((char **)&p,(char *)&value,sizeof(value));
		if(oper == EOperatorType_Equals) {
			//getValue(variable,result,sizeof(result));
			//printf("value: %s\n",value);
			if(strcmp(variable,"groupid") != 0) return true; //temp fix for games which use other filters
			result = server->findServerValue(variable);
			if(result == NULL) return false;
			if(strcmp(value,result) != 0) {
				return false;
			}
		}
			
	}
	return true;	
}
