/*
 * $Id$
 */
int mono_sql_mes_add(message_t *message);
char * mono_sql_mes_make_file(unsigned int forum, unsigned int num);
int mono_sql_mes_remove(unsigned int id, unsigned int forum);
int mono_sql_mes_fetch_content(unsigned int id, unsigned int forum, char **content);
int mono_sql_mes_retrieve(unsigned int id, unsigned int forum, message_t *message);
int mono_sql_mes_list_forum(unsigned int forum, unsigned int start, mlist_t ** list);
int mono_sql_mes_list_topic(unsigned int topic, unsigned int start, mlist_t ** list);
int mono_sql_mes_search_forum(int forum, const char *needle, sr_list_t **list);
int mono_sql_mes_mark_deleted(unsigned int id, unsigned int forum);
int mono_sql_mes_erase_forum( unsigned int forum );
int mono_sql_mes_count( int user_id );

#define M_TABLE "message"
#define FILEDIR BBSDIR "/save/messages"
