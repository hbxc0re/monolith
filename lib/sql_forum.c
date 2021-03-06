/* $Id$ */
/* operatoins we can do on forums */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#include "monolith.h"
#include "monosql.h"
#include "routines.h"
#include "sql_utils.h"

#include "btmp.h"
#include "log.h"
#include "sql_forum.h"

/* nasty compatible stuff */
int
mono_sql_f_write_quad(unsigned int num, room_t * const q)
{
    printf("debug: writing quad %u\n", num);
    mono_sql_f_kill_forum(num);
    return mono_sql_f_add_old_forum(num, q);
}

int
mono_sql_f_read_quad(unsigned int num, room_t * room)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    my_ulonglong rows;
    room_t r;

    i = mono_sql_query(&res, "SELECT "
	"name,category_old,flags,highest,lowest,generation,roominfo,maxmsg "
		       "FROM " F_TABLE " WHERE id=%u", num);

    if (i == -1) {
	fprintf(stderr, "Error: no results from query\n");
	mono_sql_u_free_result(res);
	return -1;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);

    /* sscanf error codes need to be checked ! */
    strcpy(r.name, row[0]);
    strcpy(r.category, row[1]);
    sscanf(row[2], "%u", &r.flags);
    sscanf(row[3], "%lu", &r.highest);
    sscanf(row[4], "%lu", &r.lowest);
    sscanf(row[5], "%c", &r.generation);
    sscanf(row[6], "%c", &r.roominfo);
    sscanf(row[7], "%u", &r.maxmsg);

    mono_sql_u_free_result(res);

    *room = r;
    return 0;
}

int
mono_sql_f_get_highest( forum_id_t num)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    my_ulonglong rows;

    i = mono_sql_query(&res, "SELECT MAX(message_id) FROM message WHERE forum_id=%u", num);

    if (i == -1) {
	fprintf(stderr, "Error: no results from query\n");
	mono_sql_u_free_result(res);
	return -1;
    }
    rows = mysql_num_rows(res);
    if (rows != 1) {
	mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);

    /* sscanf error codes need to be checked ! */
    sscanf(row[0], "%d", &i);
    mono_sql_u_free_result(res);

    return i;
}

int
mono_sql_f_add_old_forum(unsigned int forum_id, room_t * const q)
{

    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;

    if (escape_string(q->name, &esc_name) != 0)  {
        printf( "could not escape forumname (#%u).\n", forum_id );
        return -1;
    }

    ret = mono_sql_query(&res, "INSERT INTO " F_TABLE
			 " (id,name,category_old,flags,highest,lowest,generation,roominfo,maxmsg) "
		       "VALUES ( %u, '%s', '%s', %u, %u, %u, %d, %d, %u )\n"
	   ,forum_id, esc_name, q->category, q->flags, q->highest, q->lowest
			 ,q->generation, q->roominfo, q->maxmsg);

    xfree(esc_name);

    return ret;

}

int
mono_sql_f_rename_forum( unsigned int forum_id, char *newname )
{
    int ret;
    MYSQL_RES *res;
    char *esc_name = NULL;

    if (escape_string( newname, &esc_name) != 0)  {
        printf( "could not escape forumname (#%u).\n", forum_id );
        return -1;
    }

    ret = mono_sql_query(&res, "UPDATE " F_TABLE " SET name='%s' WHERE id=%u" ,
           esc_name, forum_id );

    xfree(esc_name);
    mono_sql_u_free_result(res);

    if (ret != 0) {
	return -1;
    }
    return 0;
}

int
mono_sql_f_update_forum(unsigned int forum_id, const room_t * q)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "UPDATE " F_TABLE " SET "
			 " (name='%s',category_old='%s',flags=%u,highest=%u,lowest=%u,generation=%d,roominfo=%d,maxmsg=%u) "
			 "WHERE id=%u\n"
		 ,q->name, q->category, q->highest, q->lowest, q->generation
			 ,q->roominfo, q->maxmsg, forum_id);

    if (ret == -1) {
	fprintf(stderr, "No results from query.\n");
	mono_sql_u_free_result(res);
	return -1;
    }
    mono_sql_u_free_result(res);
    return 0;

}
int
mono_sql_f_kill_forum(unsigned int forum_id)
{

    MYSQL_RES *res;
    int ret;

    ret = mono_sql_query(&res, "DELETE FROM " F_TABLE " WHERE (id='%u')",
			 forum_id);
    mono_sql_u_free_result(res);
    return ret;

    return 0;
}

int
mono_sql_f_name2id(const char *forumname, unsigned int *forumid)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *esc_name;

    escape_string(forumname, &esc_name);

    i = mono_sql_query(&res, "SELECT id FROM " F_TABLE " WHERE name='%s'", esc_name);

    xfree(esc_name);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) != 1) {
	mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", forumid);

    mono_sql_u_free_result(res);
    return 0;
}

int
mono_sql_f_partname2id(const char *forumname, unsigned int *forumid)
{
    int i;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *esc_name;

    escape_string(forumname, &esc_name);

    i = mono_sql_query(&res, "SELECT id FROM " F_TABLE " WHERE name like '%%%s%%'", esc_name);

    xfree(esc_name);

    if (i == -1) {
	fprintf(stderr, "No results from query.\n");
	return -1;
    }
    if (mysql_num_rows(res) < 1) {
	mono_sql_u_free_result(res);
	return -1;
    }
    row = mysql_fetch_row(res);
    sscanf(row[0], "%u", forumid);

    mono_sql_u_free_result(res);
    return 0;
}

void
mono_sql_f_fix_quickroom()
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int i = 0, ret = 0;
    unsigned int highest = 0;
    forum_id_t forum; 
    my_ulonglong rows;

    ret = mono_sql_query(&res, "SELECT forum_id,max(message_id) FROM message group by forum_id");

    if (ret == -1) {
        fprintf(stdout, "Query error\n"); fflush(stdout);
        (void) mono_sql_u_free_result(res);
        return;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
        (void) mono_sql_u_free_result(res);
        return;
    }

    for (i = 0; i < rows; i++) {
        row = mysql_fetch_row(res);
        if (row == NULL)
            break;

        sscanf(row[0], "%u", &forum);
        sscanf(row[1], "%u", &highest);

        if(shm->rooms[forum].highest != highest) {
            log_it("shmlog", "Quadrant %u is broken. SQL says highest=%u and shm says highest=%u", forum, highest, shm->rooms[forum].highest );
            shm->rooms[forum].highest=highest;
            shm->rooms[forum].lowest=(highest-(shm->rooms[forum].maxmsg));
        }
    }

    (void) mono_sql_u_free_result(res);
    return;
}

/* eof */
