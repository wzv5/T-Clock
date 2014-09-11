//Based on autorevision.cpp by Code::Blocks released as GPLv3 (should be enough modification to vanish GPL, MUHAHAHAHAW)
//	Rev: 7109 @ 2011-04-15T11:53:16.901970Z
//Now released as WTFPL
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <string>

#define MYGITPATH "E:/zZ-Progra/Git/bin"

#ifndef _MSC_VER
#	include <unistd.h>//unlink,getcwd
#	include <stdlib.h>//getenv/putenv
#	ifndef _WIN32
#		include <linux/limits.h>//PATH_MAX
#	endif // _WIN32
#else
#	include <direct.h>//unlink,getcwd
#	define PATH_MAX 260
#	define popen _popen
#	define pclose _pclose
#	define unlink _unlink
#	define putenv _putenv
#	define getcwd _getcwd
#	define chdir _chdir
#	define fopen fopenMShit
	FILE* fopenMShit(const char* filename, const char* mode){
		FILE* ret; fopen_s(&ret,filename,mode);
	return ret;
	}
#	define gmtime gmtimeMShit
	struct tm* gmtimeMShit(const time_t* time){
		static struct tm ret; gmtime_s(&ret,time);
		return &ret;
	}
#	define getenv getenvMShit
	char* getenvMShit(const char* varname){
		size_t wayne;
		static char ret[2048];
		getenv_s(&wayne,ret,sizeof(ret),varname);
		return ret;
	}
#endif
using namespace std;
bool g_verbose=false;
bool g_do_postbuild=true;
enum{
	REPO_NONE		=0x00,
	REPO_AUTOINC	=0x01, // fake repo, simple autoincrement
	REPO_GIT		=0x02,
	REPO_SVN		=0x04,
};
unsigned char g_repo=REPO_AUTOINC;
bool g_only_postbuild=false;

//major.minor[.build[.revision]]
//0.1a1
//1.0r1
//1.0.1 #2141
//    1.2.0.1 instead of 1.2-a1
//    1.2.1.2 instead of 1.2-b2 (beta with some bug fixes)
//    1.2.2.3 instead of 1.2-rc3 (release candidate)
//    1.2.3.0 instead of 1.2-r (commercial distribution)
//    1.2.3.5 instead of 1.2-r5 (commercial distribution with many bug fixes)
enum Status{
	STATUS_Alpha=0,
	STATUS_Beta,
	STATUS_ReleaseCandidate,
	STATUS_Release,
	STATUS_ReleaseMaintenance,
	STATUS_NUM_};
const char* STATUS_S[STATUS_NUM_]={"Alpha","Beta","Release Candidate","Release","Release Maintenance"};
const char* STATUS_SS[STATUS_NUM_]={"a","b","rc","r","rm"};
const char* STATUS_SS2[STATUS_NUM_]={"α","β","гc","г","гm"};
struct Version{
	unsigned char major;
	unsigned short minor;
	unsigned short build;
	unsigned char status;
	unsigned int revision;
};
bool ReadHeader(const char* filepath,Version &ver);
bool QueryGit(const char* path,Version* ver,string* url,string* date,string* revhash);
bool QuerySVN(const char* path,Version* ver,string* url,string* date);
bool WriteHeader(const char* filepath,const Version &ver,const string &url,const string &date,const string &revhash);
int main(int argc, char** argv)
{
//	+svn +noincrement +noinc --minor=99
	const char* headerPath=NULL;
	char* Gitpath=NULL;
	char* SVNpath=NULL;
	for(int i=1; i<argc; ++i) {
		if(!strcmp("-v",argv[i])) {
			g_verbose=true;
		} else if(!strcmp("--help",argv[i]) || !strcmp("-h",argv[i])) {
			g_repo|=REPO_GIT;
			Gitpath=NULL;
			break;
		} else if(!strcmp("--git",argv[i])) {
			g_repo|=REPO_GIT;
			if(i+1<argc)
				Gitpath=argv[++i];
			else
				puts("no valid Git path given\n");
		} else if(!strcmp("--svn",argv[i])) {
			g_repo|=REPO_SVN;
			if(i+1<argc)
				SVNpath=argv[++i];
			else
				puts("no valid SVN path given\n");
		} else if(!strcmp("--post-build",argv[i]) || !strcmp("--post",argv[i])) {
			g_only_postbuild=true;
		} else if(!strcmp("+noincrement",argv[i]) || !strcmp("+noinc",argv[i])) {
			g_repo&=~REPO_AUTOINC;
		} else if(!strcmp("+nopost-build",argv[i]) || !strcmp("+nopost",argv[i])) {
			g_do_postbuild=false;
		} else if((argv[i][0]!='-'||argv[i][0]!='+') && !headerPath) {
			headerPath=argv[i];
		} else {
			printf("Unknown Option: %s\n\n",argv[i]);
		}
	}
	if(g_repo&(REPO_GIT|REPO_SVN) && (!Gitpath&&!SVNpath)) {
		puts("Usage: autoversion [options] [autoversion.h]\n"
			"Options:\n"
			"   --git <path>             use Git for 'revision', also add Git date & URL\n"
			"   --svn <path>             use SVN for 'revision', also add SVN date & URL\n"
			"   --post-build, --post     do post-build stuff (clean lockfile to re-enable auto increment)\n"
			"   +noincrement, +noinc     do not auto increment revision\n"
			"   +nopost-build, +nopost   do not do post-build stuff (create lockfile to disable auto increment)\n"
			"   -v                       be verbose");
		return 1;
	}
	if(!headerPath)
		headerPath="version.h";
	size_t hlen=strlen(headerPath);
/// @todo (White-Tiger#1#): aren't both files doin' "nearly" the same?
	char lockPath[PATH_MAX];
		memcpy(lockPath,headerPath,sizeof(char)*hlen);
		memcpy(lockPath+hlen,".lock",sizeof(char)*6);
	//char incPath[hlen+6];
	//	memcpy(incPath,headerPath,sizeof(char)*hlen);
	//	memcpy(incPath+hlen,".inc",sizeof(char)*5);
	if(g_only_postbuild) {
		FILE* flock = fopen(lockPath,"rb");
		if(flock){
			fclose(flock);
			unlink(lockPath);
		}
		//flock = fopen(incPath,"wb");
		//if(flock)
		//	fclose(flock);
		return 0;
	} else if(g_do_postbuild && g_repo<=REPO_AUTOINC) {
		FILE* flock = fopen(lockPath,"rb");
		if(flock){
			fclose(flock);
			return 0;
		}
		flock = fopen(lockPath,"wb");
		if(flock)
			fclose(flock);
		//flock = fopen(incPath,"rb");
		//if(flock){
		//	fclose(flock);
			//unlink(incPath);
		//}else
		//	do_autoinc=false;
	}
	Version ver={0};
	string url="";
	string date="unknown date";
	string revhash="";

//	printf("Version: %u.%hu.%hu.%hu #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
	ReadHeader(headerPath,ver);
//	printf("Version: %u.%hu.%hu.%hu #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
	unsigned int rev=ver.revision;
	if(g_repo&REPO_GIT){
		if(QueryGit(Gitpath,&ver,&url,&date,&revhash))
			g_repo=REPO_GIT;
		else
			g_repo&=~REPO_GIT;
	}
	if(g_repo&REPO_SVN){
		if(QuerySVN(SVNpath,&ver,&url,&date))
			g_repo=REPO_SVN;
		else
			g_repo&=~REPO_SVN;
	}
	if(g_repo&REPO_AUTOINC){
		g_repo=REPO_NONE;
		++ver.revision;
	}
//	if(g_repo){ // increase revision because on commit the revision increases :P
//		++ver.revision; // So we should use the "comming" revision and not the previous (grml... date and time is wrong though :/ )
//	}
//	printf("Version: %u.%hu.%hu.%hu #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
	if(rev!=ver.revision){
		WriteHeader(headerPath,ver,url,date,revhash);
		puts("	- increased revision");
	}

	return 0;
}
bool QueryGit(const char* path,Version* ver,string* url,string* date,string* revhash)
{
	bool found=false;
#	ifdef _WIN32
	string env="PATH=";env+=getenv("PATH");env+=";" MYGITPATH; putenv(const_cast<char*>(env.c_str()));
#	endif // _WIN32
	char cwd[PATH_MAX]; getcwd(cwd,sizeof(cwd));
	if(chdir(path))
		return false;
	char buf[4097],* pos,* data;
	FILE* git;
	git=popen("git rev-list HEAD --count","r");
	if(git){ /// revision count
		int error;
		size_t read=fread(buf,sizeof(char),4096,git); buf[read]='\0'; error=pclose(git);
		if(!error && *buf>='0' && *buf<='9'){ // simple error check on command failure
			ver->revision=atoi(buf);
			git=popen("git remote -v","r");
			if(git){ /// url
				read=fread(buf,sizeof(char),4096,git); buf[read]='\0'; error=pclose(git);
				for(pos=buf; *pos && (*pos!='\r'&&*pos!='\n'&&*pos!=' '&&*pos!='\t'); ++pos);
				for(data=pos; *data=='\r'||*data=='\n'||*data==' '||*data=='\t'; ++data);
				for(pos=data; *pos && (*pos!='\r'&&*pos!='\n'&&*pos!=' '&&*pos!='\t'); ++pos);
				if(!error && *data && pos>data){
					url->append(data,pos-data);
//					git=popen("git log -1 --pretty=%h%n%an%n%at","r"); // short hash, author, timestamp
					git=popen("git log -1 --pretty=%h%n%at","r"); // short hash, timestamp
					if(git){ /// shorthash,author,timestamp		SVN date example: 2014-07-01 21:31:24 +0200 (Tue, 01 Jul 2014)
						read=fread(buf,sizeof(char),4096,git); buf[read]='\0'; error=pclose(git);
						for(pos=buf; *pos && (*pos!='\r'&&*pos!='\n'); ++pos);
						if(!error && *pos){
							revhash->assign(buf,pos-buf); ++pos;
							time_t tt=atoi(pos);
							tm* ttm=gmtime(&tt);
							strftime(buf,64,"%Y-%m-%d %H:%M:%S +0000 (%a, %b %d %Y)",ttm);
							date->assign(buf);
							found=true;
						}
					}
				}
			}
		}
	}
	chdir(cwd);
	return found;
}
bool QuerySVN(const char* path,Version* ver,string* url,string* date)
{
	bool found=false;
	string svncmd("svn info --non-interactive ");
	svncmd.append(path);
	FILE* svn = popen(svncmd.c_str(), "r");
	if(svn){
		size_t attrib_len=0; char attrib[32]={};
		size_t value_len=0; char value[128]={};
		char buf[4097];
		size_t read;
		read=fread(buf,sizeof(char),4096,svn); buf[read]='\0';
		if(pclose(svn)==0){
			for(char* c=buf; *c; ++c) {
				nextloop:
				if(attrib_len>=31) goto nextline;
				switch(*c) {
				case ':':
					attrib[attrib_len]='\0';
					for(++c; *c==' ' || *c=='\t'; ++c);
					for(; *c && *c!='\n'; ++c) {
						if(value_len>=127) {
							value_len=0; *value='\0';
							break;
						}
						value[value_len++]=*c;
					}
					if(*c=='\n') {
						value[value_len]='\0';
						if(!strcmp(attrib,"Revision")) {
//							printf("Found: %s %s\n",attrib,value);
							ver->revision=atoi(value);
							found=true;
						} else if(!strcmp(attrib,"Last Changed Date")) {
//							printf("Found: %s @ %s\n",attrib,value);
							date->assign(value);
						} else if(!strcmp(attrib,"URL")) {
//							printf("Found: %s @ %s\n",attrib,value);
							url->assign(value);
						}
					}
					value_len=0; *value='\0';
				case '\n':
					goto nextline;
				default:
					attrib[attrib_len++]=*c;
				}
				continue;
				nextline:
				for(; *c && *c!='\n'; ++c);
				for(; *c=='\r'||*c=='\n'||*c==' '||*c=='\t'; ++c);
				attrib_len=0; *attrib='\0';
				if(!*c) break;
				goto nextloop;
			}
		}
	}
	return found;
}
bool ReadHeader(const char* filepath,Version &ver)
{
	FILE* fheader = fopen(filepath,"rb");
	if(!fheader) {
		printf("Error: Couldn't read version file '%s', creating\n",filepath);
		return false;
	}
	char buf[2048]={};
	fread(buf,2048,sizeof(char),fheader);
	fclose(fheader);
	size_t def_found, def_num=7;const char def[]="define ";
	size_t attrib_len=0; char attrib[32]={};
	size_t value_len=0; char value[64]={};
	for(char* c=buf; *c; ++c) {
		nextloop:
		if(attrib_len>=31) {attrib_len=0; *attrib='\0'; goto nextline;}
		switch(*c) {
		case '#':
			for(++c; *c==' '||*c=='\t'; ++c);
			for(def_found=0;*c&&*c==def[def_found];c++,def_found++);
			if(def_found!=def_num) goto nextline;
			attrib_len=0; *attrib='\0';
			for(; *c && *c!=' ' && *c!='\n'; attrib[attrib_len++]=*c++);
			if(*c=='\n') goto nextline;
			attrib[attrib_len]='\0';
			for(++c; *c==' '||*c=='\t'; ++c);
			value_len=0; *value='\0';
			for(; *c && *c!='\n'; ++c) {
				if(value_len>=63) break;
				value[value_len++]=*c;
			}
			if(*c=='\n') {
				value[value_len]='\0';
//				printf("Found: %s: %s\n",attrib,value);
				if(!strcmp(attrib,"VER_MAJOR")) {
					int tmp=atoi(value);
					if(tmp>0xFF)tmp=0xFF;
					else if(tmp<0)tmp=0;
					ver.major=tmp;
				} else if(!strcmp(attrib,"VER_MINOR")) {
					int tmp=atoi(value);
					if(tmp>0xFFFF)tmp=0xFFFF;
					else if(tmp<0)tmp=0;
					ver.minor=tmp;
				} else if(!strcmp(attrib,"VER_BUILD")) {
					int tmp=atoi(value);
					if(tmp>0xFFFF)tmp=0xFFFF;
					else if(tmp<0)tmp=0;
					ver.build=tmp;
				} else if(!strcmp(attrib,"VER_REVISION")) {
					ver.revision=atoi(value);
				} else if(!strcmp(attrib,"VER_STATUS")) {
					int tmp=atoi(value);
					if(tmp>STATUS_NUM_-1)tmp=STATUS_NUM_-1;
					else if(tmp<0)tmp=0;
					ver.status=tmp;
				}
			}
		default:
			goto nextline;
		}
		continue;
		nextline:
		for(; *c && *c!='\n'; ++c);
		for(; *c=='\r'||*c=='\n'||*c==' '||*c=='\t'; ++c);
		if(!*c) break;
		goto nextloop;
	}
	return true;
}
bool WriteHeader(const char* filepath,const Version &ver,const string &url,const string &date,const string &revhash)
{
	FILE* fheader = fopen(filepath,"wb");
	if(!fheader) {
		puts("Error: Couldn't open output file.");
		return false;
	}
	fputs("/* Note: to use integer defines as strings, use for example STR(VER_REVISION) */\n",fheader);
	fputs("#pragma once\n",fheader);
	fputs("#ifndef AUTOVERSION_H\n",fheader);
	fputs("#define AUTOVERSION_H\n",fheader);
	fputs("#	define XSTR(x) #x\n",fheader);
	fputs("#	define STR(x) XSTR(x)\n",fheader);
//	fputs("namespace Version{\n",fheader);
	char tmp[64];
	time_t tt=time(NULL);
	tm* ttm=gmtime(&tt);
	fputs("/** Version **/\n",fheader);
	fprintf(fheader,"#	define VER_MAJOR %hu\n"
					"#	define VER_MINOR %hu\n"
					"#	define VER_BUILD %hu\n",ver.major,ver.minor,ver.build);
	fprintf(fheader,"#	define VER_STATUS %hu\n"
					"#	define VER_STATUS_FULL \"%s\"\n"
					"#	define VER_STATUS_SHORT \"%s\"\n"
					"#	define VER_STATUS_GREEK \"%s\"\n",ver.status,STATUS_S[ver.status],STATUS_SS[ver.status],STATUS_SS2[ver.status]);
	fprintf(fheader,"#	define VER_REVISION %u\n",ver.revision);
	fprintf(fheader,"#	define VER_FULL \"%hu.%hu.%hu %s\"\n",ver.major,ver.minor,ver.build,STATUS_S[ver.status]);
	fprintf(fheader,"#	define VER_SHORT \"%hu.%hu%s%hu\"\n",ver.major,ver.minor,STATUS_SS[ver.status],ver.build);
	fprintf(fheader,"#	define VER_SHORT_DOTS \"%hu.%hu.%hu\"\n",ver.major,ver.minor,ver.build);
	fprintf(fheader,"#	define VER_SHORT_GREEK \"%hu.%hu%s%hu\"\n",ver.major,ver.minor,STATUS_SS2[ver.status],ver.build);
	fprintf(fheader,"#	define VER_RC_REVISION %hu, %u, %u, %u\n",ver.major,ver.minor,ver.build,ver.revision);
	fprintf(fheader,"#	define VER_RC_STATUS %hu, %u, %u, %u\n",ver.major,ver.minor,ver.status,ver.build);
	if(g_repo) {
		fputs("/** Subversion Information **/\n",fheader);
		fprintf(fheader, "#	define VER_REVISION_URL \"%s\"\n",url.c_str());
		fprintf(fheader, "#	define VER_REVISION_DATE \"%s\"\n",date.c_str());
		fprintf(fheader, "#	define VER_REVISION_HASH \"%s\"\n",revhash.c_str());
	}
	fputs("/** Date/Time **/\n",fheader);
	fprintf(fheader,"#	define VER_TIMESTAMP %lu\n",tt);
	fprintf(fheader,"#	define VER_TIME_SEC %i\n",ttm->tm_sec);
	fprintf(fheader,"#	define VER_TIME_MIN %i\n",ttm->tm_min);
	fprintf(fheader,"#	define VER_TIME_HOUR %i\n",ttm->tm_hour);
	fprintf(fheader,"#	define VER_TIME_DAY %i\n",ttm->tm_mday);
	fprintf(fheader,"#	define VER_TIME_MONTH %i\n",ttm->tm_mon+1);
	fprintf(fheader,"#	define VER_TIME_YEAR %i\n",1900+ttm->tm_year);
	fprintf(fheader,"#	define VER_TIME_WDAY %i\n",ttm->tm_wday);
	fprintf(fheader,"#	define VER_TIME_YDAY %i\n",ttm->tm_yday);
	strftime(tmp,64,"%a",ttm);
	fprintf(fheader,"#	define VER_TIME_WDAY_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%A",ttm);
	fprintf(fheader,"#	define VER_TIME_WDAY_FULL \"%s\"\n",tmp);
	strftime(tmp,64,"%b",ttm);
	fprintf(fheader,"#	define VER_TIME_MONTH_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%B",ttm);
	fprintf(fheader,"#	define VER_TIME_MONTH_FULL \"%s\"\n",tmp);
	strftime(tmp,64,"%H:%M:%S",ttm);
	fprintf(fheader,"#	define VER_TIME \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%d",ttm);
	fprintf(fheader,"#	define VER_DATE \"%s\"\n",tmp);
	strftime(tmp,64,"%a, %b %d, %Y %H:%M:%S UTC",ttm);
	fprintf(fheader,"#	define VER_DATE_LONG \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%d %H:%M:%S UTC",ttm);
	fprintf(fheader,"#	define VER_DATE_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%dT%H:%M:%SZ",ttm);
	fprintf(fheader,"#	define VER_DATE_ISO \"%s\"\n",tmp);

	fputs("#endif\n",fheader);
	fclose(fheader);
	return true;
}
