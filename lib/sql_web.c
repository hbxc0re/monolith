/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "monosql.h"
#include "routines.h"

#include "sql_convert.h"
#include "sql_llist.h"
#include "sql_user.h"
#include "sql_utils.h"
#include "sql_web.h"

/*
 * Adds an entry into the `online' table.
 */
int
mono_sql_web_send_x(unsigned int sender, unsigned int recipient, const char *message, const char *interface)
{

    int ret;
    MYSQL_RES *res;
    char *fmt_message = NULL;

    (void) escape_string(message, &fmt_message );

    ret = mono_sql_query(&res, "INSERT INTO " WEB_X_TABLE 
     " (sender,recipient,message,date,status,i_recipient,i_sender) VALUES (%u,%u,'%s',NOW(),'unread','web','%s')"
     , sender,recipient,fmt_message, interface);

    if (ret == -1) {
        xfree(fmt_message);
	return FALSE;
    }

    mono_sql_u_free_result(res);
    xfree(fmt_message);
    return TRUE;
}

int
mono_sql_web_remove_read(unsigned int user_id)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " WEB_X_TABLE 
     " WHERE sender=%u AND status='read' AND (r_interface='client' OR r_interface='telnet')", user_id );

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_web_cleanup(unsigned int user_id)
{

    int ret;
    MYSQL_RES *res;

    ret = mono_sql_query(&res, "DELETE FROM " WEB_X_TABLE 
     " WHERE sender=%u OR recipient=%u", user_id, user_id);

    if (ret == -1)
	return FALSE;

    mono_sql_u_free_result(res);
    return TRUE;
}

int
mono_sql_web_get_online(wu_list_t ** list)
{

    int ret, i;
    my_ulonglong rows;
    MYSQL_RES *res;
    MYSQL_ROW row;
    wu_list_t entry;

    ret = mono_sql_query(&res, "SELECT u.username AS name,UNIX_TIMESTAMP(o.date) AS online FROM online AS o LEFT JOIN user AS u ON o.user_id=u.id WHERE o.interface='web' ORDER by online ASC");

    if (ret == -1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mono_sql_u_free_result(res);
	return 0;
    }
    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	/*
	 * Get result and add to list.
	 */
	if ((entry.user = mono_sql_convert_row_to_wu(row)) == NULL) {
	    continue;
	}
	if (mono_sql_ll_add_wulist_to_list(entry, list) == -1) {
	    continue;
	}
    }
    (void) mono_sql_u_free_result(res);
    return rows;
}

#ifdef OLD
#define WHOLIST_LINE_LENGTH 200

char *
mono_sql_web_wholist(int level)
{
    char *p, *q, line[WHOLIST_LINE_LENGTH];
    wu_list_t *list = NULL;
    int count = 0, j = 0;

    count = mono_sql_web_get_online(&list);

    /*
     * Query error?
     */
    if(count == -1) {
        mono_sql_ll_free_wulist(list);
        return NULL;
    }

    p = (char *) xmalloc(WHOLIST_LINE_LENGTH * (count+5));
    strcpy(p, "");

    if(count == 0) {
        (void) sprintf(p, "\n\1f\1gThere are no users online via the web.\n");
        mono_sql_ll_free_wulist(list);
        return p;
    } else {
        (void) sprintf(p, "\n\1f\1gThere %s \1y%d\1g user%s online via the web.\n"
            , (count == 1) ? "is" : "are", count, (count == 1) ? "" : "s" );
 
        switch(level) {

            case 3:	/* short */
                while(list != NULL) {
                    j++;
                    if(j == 1)
                        strcpy(line, "\n\1p[web\1p] ");
                    else
                        strcpy(line, "\1p[web\1p] ");
                    q = line + strlen(line);
                    (void) sprintf(q, "\1f\1g%-18s ", list->user->username);
                    if ((j % 3) == 0)
                        strcat(line, "\n");
                    strcat(p, line);
                    list = list->next;
                }
                strcat(p, "\n");
                break;

            default:	/* normal */
	        strcat(p, "\1f\1w-------------------------------------------------------------------------------\1a\n");
                /*
                 * Start printing users.
                 */
                while(list != NULL) {
                    sprintf(line, "\1a\1f\1g%-20s ", list->user->username);
                    strcat(line, "\1p[   \1yW] ");
                    strcat(line, "\1gMonolith Website ");
                    q = line + strlen(line);
                    (void) sprintf(q, "\1f\1p%4s ", list->user->online);
                    q = line + strlen(line);
                    (void) sprintf(q, "\1f\1ySurfing the web in style!\1a\n");
                    strcat(p, line);
                    list = list->next;
                }
                break;
        }
    }
    mono_sql_ll_free_wulist(list);
    return p;

}
#endif

int
mono_sql_web_get_xes(unsigned int user_id, wx_list_t ** list)
{

    int ret, i;
    my_ulonglong rows;
    MYSQL_RES *res;
    MYSQL_ROW row;
    wx_list_t entry;

    ret = mono_sql_query(&res, "SELECT w.id AS id,u.username AS user,w.message AS message,UNIX_TIMESTAMP(w.date) AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.sender WHERE w.recipient=%u AND w.status='unread' AND (w.i_recipient='telnet' OR w.i_recipient='client') ORDER BY w.date ASC", user_id);

    if (ret == -1) {
	(void) mono_sql_u_free_result(res);
	return -1;
    }
    if ((rows = mysql_num_rows(res)) == 0) {
	(void) mono_sql_u_free_result(res);
	return 0;
    }

    for (i = 0; i < rows; i++) {
	row = mysql_fetch_row(res);
	if (row == NULL)
	    break;
	/*
	 * Get result and add to list.
	 */
	if ((entry.x = mono_sql_convert_row_to_wx(row)) == NULL) {
	    continue;
	}
	if (mono_sql_ll_add_wxlist_to_list(entry, list) == -1) {
	    continue;
	}
    }
    (void) mono_sql_u_free_result(res);

    return rows;
}

void
mono_sql_web_mark_wx_read(wx_list_t *list)
{
    MYSQL_RES *res;
    int ret;

    while(list != NULL) {
        ret = mono_sql_query(&res, "UPDATE webx SET status='read' WHERE id=%u", list->x->id);
        if ( ret == -1 ) continue;
        mono_sql_u_free_result(res);
        list = list->next;
    }
    return;

}
/* eof */
