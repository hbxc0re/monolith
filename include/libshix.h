/* $Id$ */

/* defines */
#define DELIM	"|"
#define NSUBEXP  10
#define SHIX_SCOREFILE BBSDIR "etc/shix.scores"

typedef struct {
    char word[100];
    int score;
} shix_t;

typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

/* prototypes */
extern void regerror(char *);
extern int shix( const char *message );
extern regexp * regcomp( char *exp);
extern int regexec( regexp *prog, char *string);
