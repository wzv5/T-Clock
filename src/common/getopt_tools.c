#include "getopt_tools.h"
#include <stdio.h>
#include <string.h>

const char* PrintIndentedLine(const char* str, int max_line, int indented, int indent){
	const char* eol;
	max_line -= indent;
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
	printf("%.*s\n", (int)(eol-str), str);
	if(*eol <= ' ' && *eol)
		return eol + 1;
	return eol;
}
int DisplayHelp(const char* argv0, const char* short_options, const struct option* long_options, const struct help* help_info, int max_line){
	enum{
		flags_invalid = 0,
		flag_valid    = 0x01,
		flag_long     = 0x02,
		flag_optional = 0x04,
	};
	int flags;
	int longest_line;
	int len;
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
	longest_line = printf("Usage:   %s ", argv0);
	offset = PrintIndentedLine(help_info[0].descr, max_line, longest_line, longest_line);
	while(*offset) {
		len = 0;
		if(offset[-1] == '\n')
			len = printf("%*s ", longest_line-1, argv0);
		offset = PrintIndentedLine(offset, max_line, len, longest_line);
	}
	// get indent part one
	longest_line = 0;
	for(opt=0; long_options[opt].name; ++opt){
		len = 4 + 2 + (int)strlen(long_options[opt].name) + 2;
		if(len > longest_line)
			longest_line = len;
	}
	// options
	puts("Options:");
	measure = 1;
	do{
		for(idx=1; help_info[idx].descr; ++idx){
			flags = flags_invalid;
			opt = 0;
			len = 2;
			// print option
			for(offset=short_options; *offset; ++offset){
				if(help_info[idx].opt == *offset){
					flags = flag_valid;
					if(!strcmp(offset+1, "::"))
						flags |= flag_optional;
					len += 2; // -x
					if(!measure)
						printf("  -%c", help_info[idx].opt);
					break;
				}
			}
			if(flags == flags_invalid) {
				for(; long_options[opt].name; ++opt){
					if(long_options[opt].val == help_info[idx].opt){
						flags = flag_valid | flag_long;
						if(long_options[opt].has_arg == optional_argument)
							flags |= flag_optional;
						len += 2 + (int)strlen(long_options[opt].name); // --x
						if(!measure)
							printf("  --%s", long_options[opt].name);
						++opt;
						break;
					}
				}
				if(flags == flags_invalid)
					continue;
			}
			// print parameter
			if(help_info[idx].params){
				len += 1 + (int)strlen(help_info[idx].params);
				if(flags & flag_optional) {
					if(flags & flag_long) {
						len += 2;
						if(!measure)
							putc('=', stdout);
					}else
						++len;
					if(!measure)
						printf("[%s]", help_info[idx].params);
				}else if(!measure)
					printf(" %s", help_info[idx].params);
			}
			
			if(measure){
				len += 2; // additional padding
				if(len > longest_line)
					longest_line = len;
				continue;
			}
			// print description & option aliases
			offset = PrintIndentedLine(help_info[idx].descr, max_line, len, longest_line);
			for(; long_options[opt].name; ++opt){
				if(long_options[opt].val == help_info[idx].opt){
					len = printf("    --%s  ", long_options[opt].name);
					offset = PrintIndentedLine(offset, max_line, len, longest_line);
				}
			}
			while(*offset)
				offset = PrintIndentedLine(offset, max_line, 0, longest_line);
		}
	}while(measure--);
	return longest_line;
}


#if defined(_MSC_VER) || defined(GETOPT_OVERWRITE)
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
			len = (int)(s_nextchar - opt);
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
						}else if(longopts[idx].has_arg != optional_argument) {
							if(s_idx < argc){
								optarg = argv[s_idx];
								s_idx = optind++;
							}else{
								if(opterr){
									printOptErr("option requires an argument -- %s\n", opt);
									return (*optstring==':'?':':'?');
								}
								optarg = (char*)"-";
							}
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
				if(opt[2] != ':') {
					if(s_idx < argc){
						optarg = argv[s_idx];
						s_idx = optind++;
					}else{
						printOptErr("option requires an argument -- %c\n", (char)optopt);
						return (*optstring==':'?':':'?');
					}
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
