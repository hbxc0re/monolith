/* $Id$ */

/* prototypes */
extern void storeug( int );
extern void killroom( void );
extern void look_into_quickroom( int );
extern void edit_room_field( room_t *, unsigned int, int );
extern void move_rooms( void );
extern void reset_room( void );
extern void whoknows( void );
extern void ungoto( void );
extern void forget( void );
extern void editroom( void );
extern void reset_lastseen(void);
extern void usergoto (int where, int old_rm, int);
extern int jump( int );
extern void gotonext( void );
extern void gotocurr( void );
extern void skiproom( void );
extern int  goto_next_skipped( const int ); 
extern void show_known_rooms( int param );
extern char * known_rooms_list ( const user_t *, int );
extern void do_kickout( void );
extern void do_invite( void );
extern void print_type( void );
extern void room_debug_info( void );
extern void create_room( void );
extern void change_roominfo( void );
extern void show_room_flags( void );
extern int check_messages(int);
extern void mark_as_read(int);
extern void leave_n_unread_posts(int, int); 
extern int is_zapped(int room);
extern void check_generation(void);
extern void zap_all( void );
extern char * unread_rooms_list(const user_t * );
extern void erase_all_messages( long );
extern void low_traffic_quad_list( void );
extern room_t readquad( int ); /*EEK!*/
extern void edithosts_menu(void);
extern void invite_menu(void);
extern void kickout_menu(void);
extern void print_forumlist_list(forumlist_t *);
