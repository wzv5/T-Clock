#ifndef GETOPT_TOOLS_H_
#define GETOPT_TOOLS_H_
#ifdef __cplusplus
extern "C" {
#endif

#if !defined( _MSC_VER) && !defined(GETOPT_OVERWRITE)
#	include <getopt.h>
#else // _MSC_VER
#	include <windows.h>
#	define no_argument         no_argument_msvc
#	define required_argument   required_argument_msvc
#	define optional_argument   optional_argument_msvc
#	define option              option_msvc
#	define optind              optind_msvc
#	define optopt              optopt_msvc
#	define opterr              opterr_msvc
#	define optarg              optarg_msvc
#	define getopt_long         getopt_long_msvc
struct option_msvc{
	const char* name;
	int has_arg;
	int* flag;
	int val;
};
extern int optind_msvc; /* index of first non-option in argv      */
extern int optopt_msvc; /* single option character, as parsed     */
extern int opterr_msvc; /* flag to enable built-in diagnostics... */
                     /* (user may set to zero, to suppress)    */
extern char* optarg_msvc; /* pointer to argument of current option  */
enum HAS_ARG{
	no_argument_msvc = 0,   /* option never takes an argument */
	required_argument_msvc, /* option always requires an argument */
	optional_argument_msvc  /* option may take an argument */
};
// basic implementation, doesn't support GNU extensions in `optstring` (+-) or POSIXLY_CORRECT
// http://linux.die.net/man/3/getopt_long
int getopt_long_msvc(int argc, char*const argv[], const char* optstring, const struct option* longopts, int* longindex);
#endif // _MSC_VER

#define DH_ARGV 0 ///< use \c argv0 as is \sa DisplayHelp()
#define DH_ARGV_SHORT (const char*)1 ///< use the file name of argv0 without path \sa DisplayHelp()
/** \sa help::opt, help::params, help::descr */
struct help {
	int opt; ///< option identifier (eg. short option or long option value
	const char* params; ///< \b optional parameter text to display
	const char* descr; ///< \b required description to display
};
/** \brief prints command line help to \c stdout
 * \param argv0 program path ( likely just \c argv[0] )
 * \param short_options same as \c options for \c getopt(_long)
 * \param long_options same as \c longopts for \c getopt_long
 * \param help_info help information structure. Contains descriptions of valid arguments in \p short_options and \p long_options
 * \param max_line max. chars per line (split by word boundary. Recommended: 80)
 * \return the maximum option / indentation length for use by \c PrintIndentedLine() or zero on failure
 * \remark \p help_info [0] configures the behavior of \c DisplayHelp()
 * \remark \c help_info[0].opt is currently unused and should be zero.
 * \remark \c help_info[0].params configures the behavior of \p argv0 or replaces it entirely. Set to any \c DH_ARGV* constant
 * \remark \c help_info[0].descr is the usage description after the program name. Eg. <code>[OPTION]... [-T] SOURCE DEST</code>
 * \remark Note: \p argv0 can be \c NULL when \c help_info[0].params points to a valid string
 * \sa help, DH_ARGV, DH_ARGV_SHORT, PrintIndentedLine(), getopt(), getopt_long() */
int DisplayHelp(const char* argv0, const char* short_options, const struct option* long_options, const struct help* help_info, int max_line);
/** \brief prints a line to \c stdout indented by \p indent with maximum \p max_line characters per line
 * \param str string to output
 * \param max_line max. chars per line (split by word boundary. Recommended: 80)
 * \param indented already indented by this number of characters
 * \param indent requested indentation level
 * \return input string pointing to the next character for output
 * \remark This function must be called repeatedly with the last returned string until it points to a null char */
const char* PrintIndentedLine(const char* str, int max_line, int indented, int indent);

#ifdef __cplusplus
}
#endif
#endif // GETOPT_TOOLS_H_
