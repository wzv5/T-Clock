#include "getopt_tools.h"
#include <stdio.h>
#include <string.h>

const char* PrintIndentedLine(const char* str, int max_line/**< = 80 */, int indented, int indent){
	const char* eol;
	max_line -= indent+2;
	if(!*str){
		putc('\n', stdout);
		return str;
	}
	for(; indented<indent; ++indented) putc(' ', stdout);
	eol = strchr(str, '\n');
	if(!eol)
		eol = strchr(str, '\0');
	if(eol-str > max_line){
		eol = str + max_line;
		for(; *eol > ' ' && eol != str; --eol);
		if(eol == str)
			eol = str + max_line - 1;
	}
	printf("  %.*s\n", eol-str, str);
	if(*eol <= ' ' && *eol)
		return eol + 1;
	return eol;
}
int DisplayHelp(const char* argv0, const char* short_options, const struct option* long_options, const struct help* help_info){
	size_t maxlen = 0;
	size_t len;
	int measure;
	int idx;
	int opt;
	const char* offset;
	// usage
	if(help_info[0].params){
		if(help_info[0].params == DH_ARGV_SHORT){
			const char* short_name;
			short_name = strrchr(argv0, '/');
			if(!short_name)
				short_name = strrchr(argv0, '\\');
			if(!short_name)
				short_name = argv0-1;
			argv0 = short_name+1;
		} else
			argv0 = help_info[0].params;
	}
	printf("Usage:   %s %s\n", argv0, help_info[0].descr);
	// get indent part one
	for(opt=0; long_options[opt].name; ++opt){
		len = 6 + strlen(long_options[opt].name);
		if(len > maxlen)
			maxlen = len;
	}
	// options
	puts("Options:");
	measure = 1;
	do{
		for(idx=1; help_info[idx].descr; ++idx){
			len = 2;
			for(offset=short_options; *offset; ++offset){
				if(help_info[idx].opt == *offset){
					len += 2; // -x
					if(!measure)
						printf("  -%c", help_info[idx].opt);
					break;
				}
			}
			opt = 0;
			if(!*offset){
				for(; long_options[opt].name; ++opt){
					if(long_options[opt].val == help_info[idx].opt){
						len += 2 + strlen(long_options[opt].name); // --x
						if(!measure)
							printf("  --%s", long_options[opt++].name);
						break;
					}
				}
				if(!long_options[opt].name)
					opt = 0;
			}
			if(!len)
				break;
			if(help_info[idx].params){
				len += 1 + strlen(help_info[idx].params);
				if(!measure)
					printf(" %s", help_info[idx].params);
			}
			
			if(measure){
				if(len > maxlen)
					maxlen = len;
				continue;
			}
			
			offset = PrintIndentedLine(help_info[idx].descr, 80, len, maxlen);
			for(; long_options[opt].name; ++opt){
				if(long_options[opt].val == help_info[idx].opt){
					len = printf("    --%s", long_options[opt].name);
					offset = PrintIndentedLine(offset, 80, len, maxlen);
				}
			}
			while(*offset)
				offset = PrintIndentedLine(offset, 80, 0, maxlen);
		}
	}while(measure--);
	return maxlen;
}


#ifdef _MSC_VER
int optind_msvc = 1; /* index of first non-option in argv      */
int optopt_msvc = 0; /* single option character, as parsed     */
int opterr_msvc = 1; /* flag to enable built-in diagnostics... */
                     /* (user may set to zero, to suppress)    */
char* optarg_msvc = NULL; /* pointer to argument of current option  */

// basic implementation, doesn't support GNU extensions in `optstring` (+-) or POSIXLY_CORRECT
// http://linux.die.net/man/3/getopt_long
int getopt_long_msvc(int argc, char*const argv[], const char* optstring, const struct option* longopts, int* longindex){
	static int s_idx = 1;
	static const char* s_nextchar = NULL;
	int idx;
	const char* opt;
	int len;
	#define printOptErr(fmt,...) if(opterr && *optstring!=':') fprintf(stderr, "%s: " fmt, argv[0], ##__VA_ARGS__)
	if(optind <= 1){
		optind = 2;
		s_idx = 1;
		s_nextchar = NULL;
	}
	if(longindex)
		*longindex = 0;
	optarg = NULL;
	for(;;){
	if(s_idx >= argc)
		return -1;
	if(!s_nextchar){ // new param
		if(argv[s_idx][0] != '-' || !argv[s_idx][1]){
			// not a parameter, search for next parameter
			for(idx=optind; idx<argc; ++idx){
				if(argv[idx][0] == '-' && argv[idx][1]){
					char** nargv = (char**)argv;
					int oldidx = s_idx;
					int move;
					int result;
					// parse param
					optind = idx;
					s_idx = optind++;
					s_nextchar = NULL;
					result = getopt_long(argc, argv, optstring, longopts, longindex);
					// permutate argv
					if(result == -1) // --
						s_idx = idx+1;
					len = s_idx-idx;
					optind = oldidx + len;
					s_idx = optind++;
					do{
						opt = nargv[idx];
						for(move=idx; move-- > oldidx; ){
							nargv[move+1] = nargv[move];
						}
						nargv[oldidx] = (char*)opt;
						++idx; ++oldidx;
					}while(len-- > 1);
					if(result == -1)
						break;
					return result;
				}
			}
			optind = s_idx;
			s_idx = argc;
			return -1;
		}
		s_nextchar = argv[s_idx]+1;
		if(*s_nextchar == '-'){
			// long option
			if(!*++s_nextchar){
				s_idx = argc;
				return -1;
			}
			opt = s_nextchar;
			s_idx = optind++;
			s_nextchar = strchr(opt, '=');
			if(!s_nextchar)
				s_nextchar = strchr(opt, '\0');
			len = s_nextchar - opt;
			s_nextchar = NULL;
			for(idx=0; longopts[idx].name; ++idx){
				if(!strncmp(opt, longopts[idx].name, len)){
					optopt = longopts[idx].val;
					if(longindex)
						*longindex = idx;
					if(longopts[idx].flag){
						*longopts[idx].flag = longopts[idx].val;
						return 0;
					}
					if(longopts[idx].has_arg != no_argument){
						if(opt[len] == '='){
							optarg = (char*)opt+len+1;
						}else if(s_idx < argc){
							optarg = argv[s_idx];
							s_idx = optind++;
						}else if(longopts[idx].has_arg != optional_argument){
							if(opterr){
								printOptErr("option requires an argument -- %s\n", opt);
								return (*optstring==':'?':':'?');
							}
							optarg = (char*)"-";
						}
					}
					return longopts[idx].val;
				}
			}
			optopt = 0;
			printOptErr("unknown option -- %.*s\n", len, opt);
			return '?';
			// end long option
		}
	}
	// short option
	if(!*s_nextchar){
		s_idx = optind++;
		s_nextchar = NULL;
		continue;
	}
	for(opt=optstring; *opt; ++opt){
		if(*opt == ':' || *opt == '+' || *opt == '-')
			continue;
		if(*opt == *s_nextchar)
			break;
	}
	optopt = *s_nextchar++;
	if(*opt){
		if(opt[1] == ':'){
			optarg = (char*)s_nextchar;
			s_idx = optind++;
			s_nextchar = NULL;
			if(!*optarg){
				optarg = NULL;
				if(s_idx < argc){
					optarg = argv[s_idx];
					s_idx = optind++;
				}else if(opt[2] != ':'){
					printOptErr("option requires an argument -- %c\n", (char)optopt);
					return (*optstring==':'?':':'?');
				}
			}
		}
		return *opt;
	}
	printOptErr("unknown option -- %c\n", (char)optopt);
	return '?';
	// end short option
	}
}
#endif // _MSC_VER
