/* $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#ifdef USE_MYSQL
  #include MYSQL_HEADER
#endif

#include "monolith.h"
#include "libmono.h"

#include "sql_user.h"

typedef struct userinfo {
    char username[L_USERNAME + 1];
    unsigned int timescalled;
    unsigned priv;
    long laston;
    unsigned flags;
    char generation[MAXQUADS];
    char forget[MAXQUADS];
} info_t;

/* global variables */
unsigned int usercount = 0;

int NewUser = 0, no_cursed = 0, no_deleted = 0, no_purged = 0, expert_users = 0,
    shix_users = 0;
/* ----------------------------- prototypes ------------------------- */

void read_all_users(void);
int main(int, char **);

int whotest(info_t *, int);
char *add_date(void);

/* ----------------------------------------------------------------- */

int
main(int argc, char *argv[] )
{
    set_invocation_name(argv[0]);
    chdir(BBSDIR);
    mono_sql_connect();

    /* read the necessary data */
    read_all_users();

    /* end */
    mono_sql_detach();
    return EXIT_SUCCESS;
}

/*
 */
void
read_all_users()
{
    char name[L_USERNAME + 1];
    unsigned int i = 0;
    user_t *user;
    DIR *userdir;
    struct dirent *tmpdirent;
    char *profile = NULL;
    char work[200];

    printf("Reading the users.\n");

    userdir = opendir(USERDIR);

    if (userdir == NULL) {
	fprintf(stderr, "opendir(%s) problems!\n", USERDIR);
	perror( get_invocation_short_name);
	exit(2);
    }
    usercount = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {
	if (tmpdirent->d_name[0] != '.')	/* && tmpdirent->d_type == DT_DIR ) peculiar.. */
	    usercount++;
    }
    rewinddir(userdir);

    printf("\nFound %u users\n", usercount);

    i = 0;
    while ((tmpdirent = readdir(userdir)) != NULL) {

#ifdef DT_DIR
	if (tmpdirent->d_type != DT_DIR)	/* ignore normal files */
	    continue;
#endif /* DT_DIR */

	if (tmpdirent->d_name[0] == '.')	/* ignore . files */
	    continue;

	strncpy(name, tmpdirent->d_name, L_USERNAME);

	user = readuser(name);
	if (user == NULL)
	    continue;

        fprintf(stdout, "Processing %s... ", user->username);
        fflush(stdout);

        strcpy(work, "");
        sprintf(work, "%s/profile", getuserdir(user->username) );
        fprintf(stdout, "mmap() ");
        fflush(stdout);
        if( (profile = map_file( work )) == NULL) {
            fprintf(stdout, "Error: map_file(%s)\n", work);
            fflush(stdout);
            xfree(profile);
	    xfree(user);
            continue;
        }
        fprintf(stdout, "save() ");
        fflush(stdout);
        if( (mono_sql_write_profile(user->usernum, profile)) == -1) {
            fprintf(stdout, "Error: write_profile(%u, %X)\n", user->usernum, *profile);
            fflush(stdout);
        }
        fprintf(stdout, "free() ");
        fflush(stdout);
        xfree(profile);
	xfree(user);
        fprintf(stdout, "ok.\n");
        fflush(stdout);

	i++;
	printf("%-5u users left to read.\r", usercount - i);	/* users left to READ */
	fflush(stdout);

    }				/* while */

    closedir(userdir);

    printf("done\n");
    fflush(stdout);

    return;
}
