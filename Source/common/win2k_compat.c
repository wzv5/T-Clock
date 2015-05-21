#include "win2k_compat.h"
#include <errno.h>

#ifdef WIN2K_COMPAT

errno_t win2k_strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count){
	(void)count;
	size_t len_src,len_dst;
	if(!strDest || !numberOfElements) return EINVAL;
	strDest[0] = '\0';
	if(!strSource) return EINVAL;
	--numberOfElements;
	
	len_src = strlen(strSource);
	if(len_src > numberOfElements)
		len_src = numberOfElements;
	len_dst = strlen(strDest);
	memcpy(strDest+len_dst, strSource, numberOfElements);
	strDest[numberOfElements] = '\0';
	return 0;
}
errno_t win2k_wcsncpy_s(wchar_t* strDest, size_t numberOfElements, const wchar_t* strSource, size_t count){
	(void)count;
	size_t len_src,len_dst;
	if(!strDest || !numberOfElements) return EINVAL;
	strDest[0] = '\0';
	if(!strSource) return EINVAL;
	--numberOfElements;
	
	len_src = wcslen(strSource);
	if(len_src > numberOfElements)
		len_src = numberOfElements;
	len_dst = wcslen(strDest);
	memcpy(strDest+len_dst, strSource, numberOfElements*sizeof(wchar_t));
	strDest[numberOfElements] = '\0';
	return 0;
}
char* win2k_strtok_s(char* strToken, const char* strDelimit, char** context){
	char* ret;
	char* pos;
	const char* delim;
	if(!strDelimit || !context || (!*context&&!strToken)){
		_set_errno(EINVAL);
		return NULL;
	}
	if(strToken)
		*context = strToken;
	for(pos=*context; *pos; ++pos){
		for(delim=strDelimit; *delim && *pos!=*delim; ++delim);
		if(*delim) break;
	}
	_set_errno(0);
	if(!*pos)
		return NULL;
	*pos = '\0';
	ret = *context;
	*context = pos+1;
	return ret;
}

#endif // WIN2K_COMPAT
