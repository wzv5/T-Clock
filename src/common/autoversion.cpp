// Based on autorevision.cpp by Code::Blocks released under GPLv3 (should be enough modification to vanish GPL, MUHAHAHAHAW)
//	Rev: 7109 @ 2011-04-15T11:53:16.901970Z
// Now released under WTFPL
#ifdef _MSC_VER // sucks == true
#	define _CRT_NONSTDC_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <sys/stat.h>//stat
#include <stdarg.h>//va_list,va_start,va_end
#include <stdint.h>
#include <time.h>
#include <string.h>
 // MinGW's C++ includes might force __USE_MINGW_ANSI_STDIO, but it bugs inttypes.h
#if __USE_MINGW_ANSI_STDIO != 0 // backup old value
#	define REAL_MINGW_ANSI_STDIO 1
#endif // __USE_MINGW_ANSI_STDIO
#include <string>
#undef __USE_MINGW_ANSI_STDIO // correct it
#define __USE_MINGW_ANSI_STDIO REAL_MINGW_ANSI_STDIO
using namespace std;

#define AV_VERSION "1.0.4"

#ifdef _MSC_VER
#	include <direct.h>//unlink,getcwd
#	define PATH_MAX 260
#	define popen _popen
#	define pclose _pclose
#	define strcasecmp _stricmp
#	define strtoll _strtoi64
#	define PRIi64 "I64i"
#else
#	include <unistd.h>//unlink,getcwd
#	include <stdlib.h>//getenv,setenv
#	ifdef __linux__
#		include <linux/limits.h>//PATH_MAX
#	endif // __linux__
#	define __STDC_FORMAT_MACROS
#	include <inttypes.h>//PRIi64
#endif

#ifdef _WIN32
#	include <windows.h>//ExpandEnvironmentStrings
#	define setenv(name,value,force) SetEnvironmentVariable(name,value)
#endif

#include "getopt_tools.c"

enum Repository{
	REPO_NONE     =0x00, ///< no repository, only synchronize defines (manual mode)
	REPO_AUTOINC  =0x01, ///< fake repository, simple auto-increment
	REPO_GIT      =0x02, ///< use Git for automated revision update
	REPO_SVN      =0x04, ///< use SVN revision
};
enum Flag{
	FLAG_NONE     =0x00,
	FLAG_PRE      =0x01, ///< pre-build, create a build lock to prevent further version changes (eg. failed build)
	FLAG_POST     =0x02, ///< post-build, remove build lock to allow new version increments
	FLAG_E_IF     =0x04, ///< allow version update if condition is \c true
	FLAG_E_IFNOT  =0x08, ///< allow version update if condition is \c false
	FLAG_READ_ONLY=0x10, ///< prints defines and exists
	FLAG_GETOPT_SECOND_PASS=0x20, ///< sets defines to absolute or relative values
	FLAG_VERBOSE  =0x40, ///< enable verbose output (not implemented yet)
	FLAG_ERROR    =0x80, ///< exit with code 2 after argument parsing stage
};
unsigned char g_flag = FLAG_PRE; ///< behavior flags \sa Flag
unsigned char g_repo = REPO_AUTOINC; ///< repositories to use \sa Repository

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
	STATUS_Alpha = 0,
	STATUS_Beta,
	STATUS_ReleaseCandidate,
	STATUS_Release,
	STATUS_ReleaseMaintenance,
	STATUS_NUM_};
//const char* STATUS_S[STATUS_NUM_]   = {"Alpha","Beta","Release Candidate","Release","Release Maintenance"};
const char* STATUS_S[STATUS_NUM_]   = {"Alpha","Beta","RC","Release","Maintenance"};
const char* STATUS_SS[STATUS_NUM_]  = {"a","b","rc","r","rm"};
const char* STATUS_SS2[STATUS_NUM_] = {"α","β","гc","г","гm"};
enum VersionFlags{
	VER_TIME_SET   = 0x01, ///< time already set, don't use current
	VER_OFFSET_REV = 0x02, ///< use differential revision numbering (required by shallow Gits; allows manually adjusted revisions)
};

// time_t issues on 32bit systems
#ifdef __i386__
#	ifdef _WIN32
		typedef __time64_t timeX_t;
#		define timeX(tt) _time64(tt)
		inline struct tm* gmtimeX_r(timeX_t* tt, struct tm* tm) {return _gmtime64_s(tm, tt) ? NULL : tm;}
#	else
		typedef int64_t timeX_t;
		timeX_t timeX(timeX_t* tt) {
			timeX_t my_tt = (timeX_t)time(NULL);
			if(tt) *tt = my_tt;
			return my_tt;
		}
		struct tm* gmtimeX_r(timeX_t* tt, struct tm* tm) {
			time_t sys_tt = (time_t)*tt;
			return gmtime_r(&sys_tt, tm);
		}
#	endif
#else
	typedef time_t timeX_t;
#	define timeX(tt) time(tt)
#	ifdef _WIN32
		inline struct tm* gmtimeX_r(timeX_t* tt, struct tm* tm) {return gmtime_s(tm, tt) ? NULL : tm;}
#	else
#		define gmtimeX_r(tt,tm) gmtime_r(tt,tm)
#	endif
#endif // __i386__

struct Version{
	uint32_t flags; ///< any combination of \c VersionFlags \sa VersionFlags
	uint32_t major;
	uint32_t minor;
	uint32_t build;
	uint32_t status;
	uint32_t revision;
	timeX_t timestamp;
	string url;
	string date;
	string revhash;
};
Version ver_ = {0};

/**
 * \brief Sets a list of defines with ';' as the delimiter
 * \param[in] define_list string with one or more defines
 * \param[in,out] ver receives written version information
 * \return \c true on success */
bool SetDefineList(char* define_list, Version &ver);
/**
 * \brief Sets specified define to either absolute or relative value
 * \param[in] define variable name
 * \param[in] value target value which can be +/-N to increment or decrement value by N
 * \param[in,out] ver receives written version information
 * \return \c true on success */
bool SetDefine(const char* define, const char* value, Version &ver);
/**
 * \brief Read version header values
 * \param[in] filepath header's file path
 * \param[in,out] ver receives version information
 * \return \c true if header was read successfully */
bool ReadHeader(const char* filepath,Version &ver);
/**
 * \brief Query the Git repository to update revision (counts all commits)
 * \param[in] path repository path
 * \param[in,out] ver updated version information
 * \return \c true on success */
bool QueryGit(const char* path,Version* ver);
/**
 * \brief Query the SVN repository to update revision
 * \param[in] path repository path
 * \param[in,out] ver updated version information
 * \return \c true on success */
bool QuerySVN(const char* path,Version* ver);
/**
 * \brief Prints \p define to version file or stdout
 * \param[in] fp output target
 * \param[in] define which define to output
 * \param[in] ver version information
 * \return \c true on success */
bool PrintDefine(FILE* fp,const char* define,const Version &ver);
/**
 * \brief Writes version header if any value has changed
 * \param[in] filepath output file path
 * \param[in,out] ver version information */
bool WriteHeader(const char* filepath,Version &ver);

/**
 * \brief Small helper to ensure timely error outputs by flushing them
 * \param format fprintf format string
 * \param ... variable number of arguments for fprintf */
void printf_stderr_line(const char* format, ...) {
	va_list args;
	fflush(stdout);
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	putc('\n', stderr);
	fflush(stderr);
}

/**
 * \brief Ensures that Git/SVN tools are inside PATH
 * \param paths custom paths to add \e [optional]
 * \remark parses AUVER_PATH variable
 * \remark falls back to meaningful defaults if neither \p paths nor AUVER_PATH are set (Windows only) */
void SetupPath(const char* paths) {
#	ifdef _WIN32
#		define PATH_DELIM ";"
#	else
#		define PATH_DELIM ":"
#	endif // _WIN32
	string path;
	const char* envp;
	int len = 0;
	if((envp=getenv("PATH")))
		path += envp;
	if(paths && paths[0]){
		path += string(PATH_DELIM) + paths;
		++len;
	}
	if((envp=getenv("AUVER_PATH")) && envp[0]){
		path += string(PATH_DELIM) + envp;
		++len;
	}
#	ifdef _WIN32
	if(!len)
		path += ";%ProgramW6432%/Git/bin;%ProgramFiles(x86)%/Git/bin;%LocalAppData%/GitHub/PORTAB~1/bin;%ProgramW6432%/TortoiseSVN/bin;%ProgramFiles(x86)%/TortoiseSVN/bin";
	len = ExpandEnvironmentStrings(path.c_str(), NULL, 0);
	char* new_path = (char*)malloc(len);
	if(!new_path)
		return;
	ExpandEnvironmentStrings(path.c_str(), new_path, len);
	setenv("PATH", new_path, 1);
	//puts(new_path);
	free(new_path);
#	else
	if(len)
		setenv("PATH", path.c_str(), 1);
	//puts(path.c_str());
#	endif // _WIN32
}

/**
 * \brief reads \p var environment variable as a boolean
 * \param var variable name
 * \return \c -1 on error, otherwise boolean
 * \remark empty strings, 0, null, n, no, false are considered \c false */
int GetEnvBool(const char* var){
	const char* env = getenv(var);
	if(!env)
		return -1;
	switch(tolower(env[0])){
	case 0:
		return 0;
	case '0':
	case 'n':
		return !(!env[1] || !strcasecmp(env,"no") || !strcasecmp(env,"null"));
	case 'f':
		return strcasecmp(env,"false");
	}
	return 1;
}

int main(int argc, char** argv)
{
	const char* additional_paths = NULL;
	const char* headerPath;
	Version ver = {0};
	ver.date = "unknown date";
	char* Gitpath = NULL;
	char* SVNpath = NULL;
	static const char* short_options = "hvVa:g:s:od:D:e:E:pPI";
	static struct option long_options[] = {
		// basic
		{"help",           no_argument,       0, 'h'},
		{"version",        no_argument,       0, '1'},
		{"verbose",        no_argument,       0, 'v'},
		{"brief",          no_argument,       0, 'V'},
		// repo settings
		{"path",           required_argument, 0, 'a'},
		{"git",            required_argument, 0, 'g'},
		{"svn",            required_argument, 0, 's'},
		{"offset-revision",no_argument,       0, 'o'},
		// misc features
		{"get",            required_argument, 0, 'd'},
		{"set",            required_argument, 0, 'D'},
		{"if",             required_argument, 0, 'e'},
		{"if-not",         required_argument, 0, 'E'},
		// build features
		{"post-build",     no_argument,       0, 'p'},
		{"post",           no_argument,       0, 'p'},
		{"no-post-build",  no_argument,       0, 'P'},
		{"no-post",        no_argument,       0, 'P'},
//		{"increment",      no_argument,       0, 'i'},
//		{"inc",            no_argument,       0, 'i'},
		{"no-increment",   no_argument,       0, 'I'},
		{"no-inc",         no_argument,       0, 'I'},
		{0}
	};
	const struct help help_info[] = {
		{0,DH_ARGV_SHORT,"[option] ... [version.h]"},
		{'h',0,"this help message"},
		{'1',0,"displays version"},
		{'v',0,"be verbose"},
		//
		{'a',"<paths>","additional paths to append to PATH variable. Works like the AUVER_PATH environment variable"},
		{'g',"<repository>","use Git repo for 'revision', also add Git date & URL"},
		{'s',"<repository>","use SVN repo for 'revision', also add SVN date & URL"},
		{'o',0,"add differential Git/SVN 'revision' to VER_REVISION (automatically in use by shallow Gits)"},
		//
		{'d',"<define>","display <define> (eg. `REVISION`) and exit"},
		{'D',"<define>=<value>","sets a <define> to given value\nValue can be +/-# to increment/decrement value by #\neg. MINOR=+1"},
		{'e',"<ENV>","only update version.h if specified environment variable is true. 'true' means that the content is neither:\n   *empty*, 0, 'null', 'n', 'no' nor 'false'"},
		{'E',"<ENV>","inverse of -e, so don't update if 'true'."},
		//
		{'p',0,"execute post-build stuff now and exit\n(cleans lockfile to re-enable auto increment)\nNote: only needed if no repository was used"},
		{'P',0,"disable post-build\n(don't create lockfile to disable auto increment)"},
//		{'i',0,"-"},
		{'I',0,"do not auto increment revision"},
		{0}
	};
	for(;;){
		int opt = getopt_long(argc, argv, short_options, long_options, NULL);
		if (opt == -1)
			break;
		switch(opt){
		case 0: case 1:
			break;
		case '?': /*case ':':*/
			g_flag |= FLAG_ERROR;
			break;
		case 'h':{
			int line_length = DisplayHelp(argv[0], short_options, long_options, help_info, 80);
			printf("Environment variables:\n"
			       "  AUVER_PATH");
			PrintIndentedLine("see --path", 80, 12, line_length);
			printf("  AUVER_IF");
			PrintIndentedLine("see --if", 80, 10, line_length);
			printf("  AUVER_IF_NOT");
			PrintIndentedLine("see --if-not", 80, 14, line_length);
			printf("  AUVER_SET");
			PrintIndentedLine("';' separated list of defines to override; see --set", 80, 11, line_length);
			puts("");}
			/* fall through */
		case '1':
			puts("AutoVersion " AV_VERSION);
			return 1;
		case 'v': g_flag |= FLAG_VERBOSE; break;
		case 'V': g_flag &= ~FLAG_VERBOSE; break;
		//
		case 'a': additional_paths = optarg; break;
		case 'g':
			g_repo |= REPO_GIT;
			Gitpath = optarg;
			break;
		case 's':
			g_repo |= REPO_SVN;
			SVNpath = optarg;
			break;
		case 'o':
			ver.flags |= VER_OFFSET_REV;
			break;
		//
		case 'D': g_flag |= FLAG_GETOPT_SECOND_PASS; break;
		case 'd': g_flag |= (FLAG_GETOPT_SECOND_PASS | FLAG_READ_ONLY); break;
		case 'e': // if
			if(GetEnvBool(optarg) != 1){
				printf("if: '%s' is false\n", optarg);
				g_flag |= FLAG_E_IF;
			}
			break;
		case 'E': // if-not
			if(GetEnvBool(optarg) == 1){
				printf("if-not: '%s' is true\n", optarg);
				g_flag |= FLAG_E_IFNOT;
			}
			break;
		//
		case 'p': g_flag |= FLAG_POST; break;
		case 'P': g_flag &= ~FLAG_PRE; break;
		case 'I': g_repo &= ~REPO_AUTOINC; break;
		default:
			;
		}
	}
	if(GetEnvBool("AUVER_IF") == 0){
		fprintf(stderr, "AUVER_IF = %s\n", getenv("AUVER_IF"));
		g_flag |= FLAG_E_IF;
	}
	if(GetEnvBool("AUVER_IF_NOT") == 1){
		fprintf(stderr, "AUVER_IF_NOT = %s\n", getenv("AUVER_IF_NOT"));
		g_flag |= FLAG_E_IFNOT;
	}
	if(g_flag & (FLAG_E_IF|FLAG_E_IFNOT)){
		puts("	version unchanged");
		return 0;
	}
	if(g_flag & FLAG_ERROR)
		return 2;
	if(g_repo&(REPO_GIT|REPO_SVN) && (!Gitpath&&!SVNpath)) {
		DisplayHelp(argv[0], short_options, long_options, help_info, 80);
		return 1;
	}
	
	SetupPath(additional_paths);
	
	if(optind < argc)
		headerPath = argv[optind];
	else
		headerPath = "version.h";
	size_t hlen = strlen(headerPath);
	if(!(g_flag & FLAG_READ_ONLY)) {
/// @todo (White-Tiger#1#): aren't both files doin' "nearly" the same?
		char lockPath[PATH_MAX];
			memcpy(lockPath,headerPath,hlen);
			memcpy(lockPath+hlen,".lock",6);
		//char incPath[hlen+6];
		//	memcpy(incPath,headerPath,hlen);
		//	memcpy(incPath+hlen,".inc",5);
		if(g_flag&FLAG_POST) {
			FILE* flock = fopen(lockPath,"rb");
			if(flock){
				fclose(flock);
				unlink(lockPath);
			}
			//flock = fopen(incPath,"wb");
			//if(flock)
			//	fclose(flock);
			return 0;
		} else if(g_flag&FLAG_PRE && g_repo<=REPO_AUTOINC) {
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
	}

//	printf("Version: %u.%u.%u.%u #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
	if(!ReadHeader(headerPath, ver))
		printf_stderr_line("Error: Couldn't read version file '%s'", headerPath);
//	printf("Version: %u.%u.%u.%u #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
	if(g_repo&REPO_GIT){
		if(QueryGit(Gitpath,&ver))
			g_repo = REPO_GIT;
		else
			g_repo &= ~REPO_GIT;
	}
	if(g_repo&REPO_SVN){
		if(QuerySVN(SVNpath,&ver))
			g_repo = REPO_SVN;
		else
			g_repo &= ~REPO_SVN;
	}
	
	if(getenv("AUVER_SET")) {
		char* define_list = strdup(getenv("AUVER_SET"));
		fprintf(stderr, "AUVER_SET = %s\n", define_list);
		if(!SetDefineList(define_list, ver))
			g_flag |= FLAG_ERROR;
		free(define_list);
	}
	if(g_flag & FLAG_GETOPT_SECOND_PASS) {
		int printed = 0;
		optind = 1;
		for(;;) {
			int opt = getopt_long(argc, argv, short_options, long_options, NULL);
			if (opt == -1)
				break;
			switch(opt) {
			case 'D':
				if(!SetDefineList(optarg, ver))
					g_flag |= FLAG_ERROR;
				break;
			case 'd':
				if(printed++)
					putc('\n', stdout);
				if(!PrintDefine(stdout, optarg, ver))
					g_flag |= FLAG_ERROR;
				break;
			}
		}
	}
	if(!(g_flag & (FLAG_READ_ONLY|FLAG_ERROR))) {
		if(g_repo&REPO_AUTOINC){
			g_repo = REPO_NONE;
			++ver.revision;
		}
//		if(g_repo){ // increase revision because on commit the revision increases :P
//			++ver.revision; // So we should use the "comming" revision and not the previous (grml... date and time is wrong though :/ )
//		}
//		printf("Version: %u.%u.%u.%u #%u\n",ver.major,ver.minor,ver.build,ver.status,ver.revision);
		if(WriteHeader(headerPath,ver)){
			puts("	- version updated");
		}
	}
	return (g_flag & FLAG_ERROR);
}
bool QueryGit(const char* path,Version* ver)
{
	bool found = false;
	char cwd[PATH_MAX];
	if(!getcwd(cwd, sizeof(cwd))) {
		printf_stderr_line("failed to retrieve current directory!");
		return false;
	}
	if(chdir(path)){
		printf_stderr_line("invalid repository path!");
		return false;
	}
	char buf[4097],* pos,* data;
	FILE* git;
	git = popen("git rev-list HEAD --count","r");
	if(git){ /// revision count
		int error;
		size_t read = fread(buf,1,(sizeof(buf)-1),git); buf[read]='\0'; error=pclose(git);
		if(!error && *buf>='0' && *buf<='9'){ // simple error check on command failure
			int revision = atoi(buf);
			if(!(ver->flags & VER_OFFSET_REV)) {
				git = popen("git rev-parse --git-dir","r");
				if(git){ /// path to .git for shallow detection (Git 2.15 supports `git rev-parse --is-shallow-repository`)
					struct stat st;
					read = fread(buf,1,(sizeof(buf)-1),git); buf[read]='\0'; error=pclose(git);
					strcat(strtok(buf,"\r\n"), "/shallow");
					if(!error && stat(buf,&st) != -1) { /// is shallow repository
						ver->flags |= VER_OFFSET_REV;
						printf_stderr_line("warn: shallow repository");
					}
				}
			}
			if(ver->flags & VER_OFFSET_REV) {
				string revcount("git rev-list --count " + ver->revhash + "..HEAD --");
				git = popen(revcount.c_str(),"r");
				if(git){ /// revision count delta
					read = fread(buf,1,(sizeof(buf)-1),git); buf[read]='\0'; error=pclose(git);
					if(error) // fallback to all commits we see
						printf_stderr_line("shallow clone depth too low!");
					else
						revision = atoi(buf);
					revision += ver->revision;
				}
			}
			ver->revision = revision;
			git = popen("git remote -v","r");
			if(git){ /// url
				read = fread(buf,1,(sizeof(buf)-1),git); buf[read]='\0'; error=pclose(git);
				for(pos=buf; *pos && (*pos!='\r'&&*pos!='\n'&&*pos!=' '&&*pos!='\t'); ++pos);
				for(data=pos; *data=='\r'||*data=='\n'||*data==' '||*data=='\t'; ++data);
				for(pos=data; *pos && (*pos!='\r'&&*pos!='\n'&&*pos!=' '&&*pos!='\t'); ++pos);
				if(!error && *data && pos>data){
					ver->url.assign(data,pos-data);
//					git = popen("git log -1 --pretty=%h%n%an%n%at","r"); // short hash, author, timestamp
					git = popen("git log -1 --pretty=%h%n%at","r"); // short hash, timestamp
					if(git){ /// shorthash,author,timestamp		SVN date example: 2014-07-01 21:31:24 +0200 (Tue, 01 Jul 2014)
						read = fread(buf,1,(sizeof(buf)-1),git); buf[read]='\0'; error=pclose(git);
						for(pos=buf; *pos && (*pos!='\r'&&*pos!='\n'); ++pos);
						if(!error && *pos){
							ver->revhash.assign(buf,pos-buf); ++pos;
							timeX_t tt = (timeX_t)strtoll(pos, NULL, 0);
							tm ttm = {0};
							strftime(buf,64,"%Y-%m-%d %H:%M:%S +0000 (%a, %b %d %Y)", gmtimeX_r(&tt,&ttm));
							ver->date.assign(buf);
							found = true;
						}
					}
				}
			}
		}
	}
	if(chdir(cwd) != 0)
		printf_stderr_line("warn: change directory failed");
	return found;
}
bool QuerySVN(const char* path,Version* ver)
{
	bool found=false;
	string svncmd("svn info --non-interactive ");
	svncmd.append(path);
	FILE* svn = popen(svncmd.c_str(), "r");
	if(svn){
		size_t attrib_len = 0; char attrib[32];
		size_t value_len = 0; char value[128];
		char buf[4097];
		size_t read;
		read = fread(buf,1,(sizeof(buf)-1),svn); buf[read]='\0';
		if(pclose(svn) == 0){
			for(char* c=buf; *c; ++c) {
				nextloop:
				if(attrib_len >= 31) goto nextline;
				switch(*c) {
				case ':':
					attrib[attrib_len] = '\0';
					for(++c; *c==' ' || *c=='\t'; ++c);
					for(; *c && *c!='\n'; ++c) {
						if(value_len >= 127) {
							value_len = 0; value[0] = '\0';
							break;
						}
						value[value_len++] = *c;
					}
					if(*c == '\n') {
						value[value_len] = '\0';
						if(!strcmp(attrib,"Revision")) {
//							printf("Found: %s %s\n",attrib,value);
							if(ver->flags & VER_OFFSET_REV)
								ver->revision += atoi(value) - atoi(ver->revhash.c_str());
							else
								ver->revision = atoi(value);
							ver->revhash.assign(value);
							found = true;
						} else if(!strcmp(attrib,"Last Changed Date")) {
//							printf("Found: %s @ %s\n",attrib,value);
							ver->date.assign(value);
						} else if(!strcmp(attrib,"URL")) {
//							printf("Found: %s @ %s\n",attrib,value);
							ver->url.assign(value);
						}
					}
					value_len = 0; value[0] = '\0';
				case '\n':
					goto nextline;
				default:
					attrib[attrib_len++] = *c;
				}
				continue;
				nextline:
				for(; *c && *c!='\n'; ++c);
				for(; *c=='\r'||*c=='\n'||*c==' '||*c=='\t'; ++c);
				attrib_len = 0; attrib[0] = '\0';
				if(!*c) break;
				goto nextloop;
			}
		}
	}
	return found;
}
bool SetDefineList(char* define_list, Version &ver)
{
	bool success = true;
	char* define;
	char* value;
	define = strtok(define_list, "=");
	while(define) {
		value = strtok(NULL, ";");
		if(!value)
			value = strchr(define, '\0');
		if(!SetDefine(define, value, ver)) {
			success = false;
			printf_stderr_line("  unknown define: %s", define);
		}
		define = strtok(NULL, "=");
	}
	return success;
}
bool SetDefine(const char* define, const char* value, Version &ver)
{
	int additive = 0;
	if(value[0] == '+' || value[0] == '-')
		additive = 1;
	if(!strcasecmp(define,"MAJOR")) {
		int tmp = atoi(value) + (additive ? ver.major : 0);
		if(tmp > 0xFF) tmp = 0xFF;
		else if(tmp < 0) tmp = 0;
		ver.major = tmp;
	} else if(!strcasecmp(define,"MINOR")) {
		int tmp = atoi(value) + (additive ? ver.minor : 0);
		if(tmp > 0xFFFF) tmp = 0xFFFF;
		else if(tmp < 0) tmp = 0;
		ver.minor = tmp;
	} else if(!strcasecmp(define,"BUILD")) {
		int tmp = atoi(value) + (additive ? ver.build : 0);
		if(tmp > 0xFFFF) tmp = 0xFFFF;
		else if(tmp < 0) tmp = 0;
		ver.build = tmp;
	} else if(!strcasecmp(define,"REVISION")) {
		ver.revision = atoi(value) + (additive ? ver.revision : 0);
	} else if(!strcasecmp(define,"STATUS")) {
		int tmp = atoi(value) + (additive ? ver.status : 0);
		if(tmp > STATUS_NUM_-1) tmp = STATUS_NUM_-1;
		else if(tmp < 0) tmp = 0;
		ver.status = tmp;
	} else if(!strcasecmp(define,"REVISION_URL")) {
		ver.url = value;
	} else if(!strcasecmp(define,"REVISION_DATE")) {
		ver.date = value;
	} else if(!strcasecmp(define,"REVISION_HASH")) {
		ver.revhash = value;
	} else if(!strcasecmp(define,"TIMESTAMP")) {
		ver.flags |= VER_TIME_SET;
		ver.timestamp = strtoll(value,NULL,0) + (additive ? ver.timestamp : 0);
	} else
		return false;
	return true;
}
bool ReadHeader(const char* filepath,Version &ver)
{
	FILE* fheader = fopen(filepath,"rb");
	if(!fheader)
		return false;
	char buf[2048];
	size_t read = fread(buf,1,(sizeof(buf)-1),fheader);
	buf[read] = '\0';
	fclose(fheader);
	size_t def_found, def_num=7;const char def[]="define ";
	size_t attrib_len = 0; char attrib[32];
	size_t value_len = 0; char value[64];
	char* c = buf;
	while(*c) {
		if(attrib_len >= 31) {
			attrib_len = 0; attrib[0] = '\0';
			goto nextline;
		}
		if(*c == '#') {
			for(++c; *c==' '||*c=='\t'; ++c);
			for(def_found=0; *c&&*c==def[def_found]; c++,def_found++);
			if(def_found != def_num)
				goto nextline;
			attrib_len = 0; attrib[0] = '\0';
			for(; *c && *c!=' ' && *c!='\n'; attrib[attrib_len++]=*c++);
			if(*c == '\n')
				goto nextline;
			attrib[attrib_len] = '\0';
			for(++c; *c==' '||*c=='\t'; ++c);
			value_len = 0; *value = '\0';
			for(; *c && *c!='\n'; ++c) {
				if(value_len >= 63)
					break;
				value[value_len++] = *c;
			}
			if(*c == '\n') {
				char* value_ptr = value;
				if(value_ptr[0] == '"' && value_len >=2) {
					++value_ptr;
					value_len -= 2;
				}
				value_ptr[value_len] = '\0';
//				printf("Found: %s: %s\n", attrib, value_ptr);
				if(attrib_len > 4) {
					if(!SetDefine(attrib+4, value_ptr, ver) && !strcmp(attrib+4,"RC_STATUS"))
						sscanf(value_ptr, "%u, %u, %u, %u", &ver_.major, &ver_.minor, &ver_.build, &ver_.status);
				}
			}
		}
		nextline:
		for(; *c && *c!='\n'; ++c);
		for(; *c=='\r'||*c=='\n'||*c==' '||*c=='\t'; ++c);
	}
	ver.flags &= ~VER_TIME_SET;
	// major, minor, build, status set above in `RC_STATUS` parsing
	ver_.revision = ver.revision;
	ver_.timestamp = ver.timestamp;
	ver_.url = ver.url;
	ver_.date = ver.date;
	ver_.revhash = ver.revhash;
	return true;
}
/**
 * \brief writes non-ASCII characters as Universal Character Names to file pointer
 * \param utf8 string to write (UTF-8 encoded)
 * \param fp open file to write to */
void fputsUnicodeEscaped(const char* utf8, FILE* fp) {
	for(; *utf8; ++utf8) {
		if(*utf8 & 0x80) {
			char mask = 0x3f;
			for(char ch=*utf8<<1; ch & 0x80; ch<<=1)
				mask>>=1;
			unsigned codepoint = *utf8 & mask;
			for(++utf8; *utf8 & 0x80; ++utf8)
				codepoint = (codepoint << 6) | (*utf8 & 0x7f);
			--utf8;
			if(codepoint > 0xffff)
				fprintf(fp,"\\U%08X",codepoint);
			else
				fprintf(fp,"\\u%04X",codepoint);
		} else
			fputc(*utf8,fp);
	}
}
bool PrintDefine(FILE* fp,const char* define,const Version &ver)
{
	if(!strcasecmp("MAJOR",define)) {
		fprintf(fp,"%u",ver.major);
	}else if(!strcasecmp("MINOR",define)) {
		fprintf(fp,"%u",ver.minor);
	}else if(!strcasecmp("BUILD",define)) {
		fprintf(fp,"%u",ver.build);
	}else if(!strcasecmp("STATUS",define)) {
		fprintf(fp,"%u",ver.status);
	}else if(!strcasecmp("STATUS_FULL",define)) {
		fprintf(fp,"%s",STATUS_S[ver.status]);
	}else if(!strcasecmp("STATUS_SHORT",define)) {
		fprintf(fp,"%s",STATUS_SS[ver.status]);
	}else if(!strcasecmp("STATUS_GREEK",define)) {
		fputsUnicodeEscaped(STATUS_SS2[ver.status],fp);
	}else if(!strcasecmp("REVISION",define)) {
		fprintf(fp,"%u",ver.revision);
	
	// version strings
	}else if(!strcasecmp("FULL",define)) {
		if(STATUS_S[ver.status][0])
			fprintf(fp,"%u.%u.%u %s",ver.major,ver.minor,ver.build,STATUS_S[ver.status]);
		else
			fprintf(fp,"%u.%u.%u",ver.major,ver.minor,ver.build);
	}else if(!strcasecmp("SHORT",define)) {
		fprintf(fp,"%u.%u%s%u",ver.major,ver.minor,STATUS_SS[ver.status],ver.build);
	}else if(!strcasecmp("SHORT_DOTS",define)) {
		fprintf(fp,"%u.%u.%u",ver.major,ver.minor,ver.build);
	}else if(!strcasecmp("SHORT_GREEK",define)) {
		fprintf(fp,"%u.%u",ver.major,ver.minor);
		fputsUnicodeEscaped(STATUS_SS2[ver.status],fp);
		fprintf(fp,"%u",ver.build);
	}else if(!strcasecmp("RC_REVISION",define)) {
		fprintf(fp,"%u, %u, %u, %u",ver.major,ver.minor,ver.build,ver.revision);
	}else if(!strcasecmp("RC_STATUS",define)) {
		fprintf(fp,"%u, %u, %u, %u",ver.major,ver.minor,ver.build,ver.status);
	
	// repository
	}else if(!strcasecmp("REVISION_URL",define)) {
		fprintf(fp,"%s",ver.url.c_str());
	}else if(!strcasecmp("REVISION_DATE",define)) {
		fprintf(fp,"%s",ver.date.c_str());
	}else if(!strcasecmp("REVISION_HASH",define)) {
		fprintf(fp,"%s",ver.revhash.c_str());
	}else if(!strcasecmp("REVISION_TAG",define)) {
		fprintf(fp,"v%u.%u.%u#%u",ver.major,ver.minor,ver.build,ver.revision);
		if(STATUS_S[ver.status][0]){
			fputc('-',fp);
			for(const char* c=STATUS_S[ver.status]; *c; ++c){
				if(*c == ' ')
					fputc('_',fp);
				else
					fputc(tolower(*c),fp);
			}
		}
	
	// date / time
	}else if(!strcasecmp("TIMESTAMP",define)) {
		fprintf(fp, "%" PRIi64, (uint64_t)ver.timestamp);
	
	
	}else{
		printf_stderr_line("  unknown define: %s", define);
		return false;
	}
	return true;
}
void WriteDefine(FILE* fp,const char* define,const Version &ver)
{
	fprintf(fp,"#	define VER_%s ",define);
	PrintDefine(fp,define,ver);
	putc('\n',fp);
}
void WriteDefineString(FILE* fp,const char* define,const Version &ver)
{
	fprintf(fp,"#	define VER_%s \"",define);
	PrintDefine(fp,define,ver);
	fputs("\"\n",fp);
}
bool WriteHeader(const char* filepath,Version &ver)
{
	FILE* fheader;
	if(ver.major == ver_.major
	&& ver.minor == ver_.minor
	&& ver.build == ver_.build
	&& ver.status == ver_.status
	&& ver.revision == ver_.revision
	&& ver.timestamp == ver_.timestamp
	&& ver.url == ver_.url
	&& ver.date == ver_.date
	&& ver.revhash == ver_.revhash){
		return false;
	}
	fheader = fopen(filepath,"wb");
	if(!fheader) {
		printf_stderr_line("Error: Couldn't open output file.");
		return false;
	}
	fputs("#ifndef AUTOVERSION_H\n",fheader);
	fputs("#define AUTOVERSION_H\n",fheader);
	fputs("/* Note: to use integer defines as strings, use STR(), eg. STR(VER_REVISION) */\n",fheader);
//	fputs("namespace Version{\n",fheader);
	fputs("/**** Version ****/\n",fheader);
	WriteDefine(fheader,"MAJOR",ver);
	WriteDefine(fheader,"MINOR",ver);
	WriteDefine(fheader,"BUILD",ver);
	fprintf(fheader,"	/** status values: 0=%s(%s)",STATUS_S[0],STATUS_SS2[0]);
	for(int i=1; i<STATUS_NUM_; ++i)
		fprintf(fheader,", %i=%s(%s)",i,STATUS_S[i],STATUS_SS2[i]);
	fputs(" */\n",fheader);
	WriteDefine(fheader,"STATUS",ver);
	WriteDefineString(fheader,"STATUS_FULL",ver);
	WriteDefineString(fheader,"STATUS_SHORT",ver);
	WriteDefineString(fheader,"STATUS_GREEK",ver);
	WriteDefine(fheader,"REVISION",ver);
	
	WriteDefineString(fheader,"FULL",ver);
	WriteDefineString(fheader,"SHORT",ver);
	WriteDefineString(fheader,"SHORT_DOTS",ver);
	WriteDefineString(fheader,"SHORT_GREEK",ver);
	WriteDefine(fheader,"RC_REVISION",ver);
	WriteDefine(fheader,"RC_STATUS",ver);
	fputs("/**** Subversion Information ****/\n",fheader);
	WriteDefineString(fheader,"REVISION_URL",ver);
	WriteDefineString(fheader,"REVISION_DATE",ver);
	WriteDefineString(fheader,"REVISION_HASH",ver);
	WriteDefineString(fheader,"REVISION_TAG",ver);
	char tmp[64];
	if(!(ver.flags & VER_TIME_SET))
		ver.timestamp = timeX(NULL);
	tm ttm = {0};
	gmtimeX_r(&ver.timestamp, &ttm);
	fputs("/**** Date/Time ****/\n",fheader);
	WriteDefine(fheader,"TIMESTAMP",ver);
	fprintf(fheader,"#	define VER_TIME_SEC %i\n",ttm.tm_sec);
	fprintf(fheader,"#	define VER_TIME_MIN %i\n",ttm.tm_min);
	fprintf(fheader,"#	define VER_TIME_HOUR %i\n",ttm.tm_hour);
	fprintf(fheader,"#	define VER_TIME_DAY %i\n",ttm.tm_mday);
	fprintf(fheader,"#	define VER_TIME_MONTH %i\n",ttm.tm_mon+1);
	fprintf(fheader,"#	define VER_TIME_YEAR %i\n",1900+ttm.tm_year);
	fprintf(fheader,"#	define VER_TIME_WDAY %i\n",ttm.tm_wday);
	fprintf(fheader,"#	define VER_TIME_YDAY %i\n",ttm.tm_yday);
	strftime(tmp,64,"%a",&ttm);
	fprintf(fheader,"#	define VER_TIME_WDAY_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%A",&ttm);
	fprintf(fheader,"#	define VER_TIME_WDAY_FULL \"%s\"\n",tmp);
	strftime(tmp,64,"%b",&ttm);
	fprintf(fheader,"#	define VER_TIME_MONTH_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%B",&ttm);
	fprintf(fheader,"#	define VER_TIME_MONTH_FULL \"%s\"\n",tmp);
	strftime(tmp,64,"%H:%M:%S",&ttm);
	fprintf(fheader,"#	define VER_TIME \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%d",&ttm);
	fprintf(fheader,"#	define VER_DATE \"%s\"\n",tmp);
	strftime(tmp,64,"%a, %b %d, %Y %H:%M:%S UTC",&ttm);
	fprintf(fheader,"#	define VER_DATE_LONG \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%d %H:%M:%S UTC",&ttm);
	fprintf(fheader,"#	define VER_DATE_SHORT \"%s\"\n",tmp);
	strftime(tmp,64,"%Y-%m-%dT%H:%M:%SZ",&ttm);
	fprintf(fheader,"#	define VER_DATE_ISO \"%s\"\n",tmp);
	
	fputs("/**** Helper 'functions' ****/\n",fheader);
	fputs("#	define VER_IsReleaseOrHigher() ( VER_STATUS >= 3 )\n", fheader);
	for(int i=0; i<STATUS_NUM_; ++i)
		fprintf(fheader,"#	define VER_Is%s() ( VER_STATUS == %i )\n", STATUS_S[i], i);
	fputs("#ifndef STR\n",fheader);
	fputs("#	define STR_(x) #x\n",fheader);
	fputs("#	define STR(x) STR_(x)\n",fheader);
	fputs("#endif\n",fheader);
	fputs("#ifndef L\n",fheader);
	fputs("#	define L_(x) L##x\n",fheader);
	fputs("#	define L(x) L_(x)\n",fheader);
	fputs("#endif\n",fheader);

	fputs("#endif\n",fheader);
	fclose(fheader);
	return true;
}
