/*
 * Generic ftcl command-line parser. 
 * Heavily inspired by the Tk command line parser.
 *
 */

#define UPSTST_PARSE_NOLEFTOVERS	0x00000001
#define UPSTST_PARSE_EXACTMATCH		0x00000002
#ifndef _UPSTST_PARSE_H
#define _UPSTST_PARSE_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
/*
 * Structure to specify how to handle command line options
 */
typedef struct {
    char *key;          /* The key string that flags the option in the
                         * argv array. */
    int type;           /* Indicates option type;  see below. */
    void *src;          /* Value to be used in setting dst;  usage
                         * depends on type. */
    void *dst;          /* Address of value to be modified;  usage
                         * depends on type. */
} upstst_argt;

int upstst_parse(int * const, char ** const , upstst_argt * const, const unsigned int options);
int upstst_split(char *, int * const, char *** const);

void upstst_print_usage (const upstst_argt * const argTable,const char *cmd_name);
void upstst_print_help (upstst_argt *argTable, char *cmd_name);
void upstst_print_arg (upstst_argt *argTable, char *cmd_name);

/* defines for return value of ftcl_ParseArgv */

#define UPSTST_SUCCESS   	0 
#define UPSTST_BADSYNTAX	1
#define UPSTST_USAGE		2

/*
 * Legal values for the type field of a upstst_argt: see the user
 * documentation for details.
 */
 
#define UPSTST_ARGV_CONSTANT                0x00000064
#define UPSTST_ARGV_INT                     0x00000065
#define UPSTST_ARGV_STRING                  0x00000066
#define UPSTST_ARGV_DOUBLE                  0x00000068
#define UPSTST_ARGV_END                     0x0000006B

#define UPSTST_USAGE_BUFSIZE    1025
#define UPSTST_ARGINFO_BUFSIZE  8193

#endif

