/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2006 Krzysztof Komarnicki
 * Email: krzkomar@wp.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See the file COPYING. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef __CFP_H__
#define __CFP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

typedef struct
{
    const char *path;
    const char *start;
    const char *end;
    int length;
    int size;
    char *link;
} s_path;

#define ERR_MISSING_BS	"incorrect path format: missing '/' at start"

typedef struct s__cfp_q s_cfpq;

typedef struct
{
    s_cfpq *p;
    s_cfpq *current;
} s_cfp;

struct s__cfp_q
{
    char *key;
    char *value;
    s_cfpq *branch;
    s_cfpq *next;
    s_cfpq *prev;
};

typedef enum
{
    P_ENTRY,
    P_SECTION_BEGIN,
    P_SECTION_END    
} t_cfp_mode;

typedef void (*f_cfp_cb)(const char *key, const char *value, int nested, t_cfp_mode type, void *user_data);
/*
    ///////////////////////// configuration tree set of functions //////////////////////////
    File structure:
    //----> start file <----
    foo = 10;
    section1 = {
	fff = "zzz";
	section2 = {
	    entry8 = 0x1234;
	};
    };
    another_section = {
	aaa = bbb;
    };
    
    // single line comment
    //----> end file <----    

    cft_save() remove all comments. multiline comments are the some as in C

    Example usage:
    s_cfp *p;
    
    p = cfp_init();
    cfp_load( p, "my_config_file.cfg");
    cfp_add( p, "/section1/section2/entry0","\"entry_value0\"");
    cfp_add( p, "/section1/section2/entry1","12345");
    cfp_add( p, "/section1/section2/custom_entry","array[1,2,3,4,5]");
    cfp_cut( p, "/foo/some_existed_entry"); // delete entry
    cfp_save( p, "my_config_file.cfg");
    printf("%s\n",  cfp_get(p, "/section1/section2/entry1")); 
    cfp_free( p );
*/

s_cfp *cfp_init( void );
void cfp_free( s_cfp *);
t_bool cfp_save(s_cfp *, const char *file_path);
t_bool cfp_load(s_cfp *, const char *file_path);
t_bool cfp_cut(s_cfp *, const char *path);
t_bool cfp_add(s_cfp *, const char *path, const char *value); // add/modify
const s_cfpq *cfp_get(s_cfp *, const char *path); // return node given by path or NULL if not existed
const s_cfpq *cfp_find( const  s_cfpq *, const char *key); // search branch s_cfpq to find key, return NULL if fail
const s_cfpq *cfp_next( const  s_cfpq *); // return next node or NULL
const s_cfpq *cfp_prev( const  s_cfpq *); // return previous node or NULL
const s_cfpq *cfp_block( const s_cfpq *); // return branch node or NULL
t_bool cfp_is_section(s_cfpq *);	    // return _T_ if node is section
void cfp_foreach(s_cfp *, f_cfp_cb, void *user_data);
const char *cfp_get_key(const s_cfpq *);

const char *cfp_get_val(const s_cfpq *, const char *key);
char *cfp_get_val_stringp(s_cfp *, const char *path); // "...", allocating memory, must be freed
char *cfp_get_val_string(s_cfpq *, const char *key); // "...", allocating memory, must be freed
char *cfp_get_val_string_lo(const s_cfpq *); // "...", allocating memory, must be freed
long   cfp_get_val_int(const s_cfpq *, const char *key); // 12345, 0x12345
double cfp_get_val_float(const s_cfpq *, const char *key); // 12.345
t_bool cfp_get_val_bool(const s_cfpq *, const char *key);  // true,false,0,1,.t.,.f.,high,low,on,off,enabled,disabled,enable,disable - case insensitive

int cfp_get_val_select(const s_cfpq *, char mode, const char *key, const char *values); // return 0+ position found, or -1: values string: "aa,bb,cc,dd,ee" -> 0,1,2,3,4
/*
    ////////////////// path decomposition set of functions //////////////////////////
    Path format:
    "/", "///"  -empty named paths
    "/my/path"  -path leaf ended
    "/my/path/" -path branch ended
    
    Example usage:
    int main()
    {
	s_path p;
	
	path_start(&p, "/my/path/to/something"); // init structure

	while(path_set_link(&p))	// display each link
	    printf("->%s\n", path_get_link(&p));

	path_reset(&p);			// set to begin

	while(path_set_link(&p))
	    printf("%s\n", path_cmp(&p, "to") ? "hit" : "miss");

	path_end(&p);	// free allocated resources
    
        return 0;
    }    

*/

t_error path_start( s_path *s, const char *path);// initialize path structure, return _F_ if ok
void    path_end( s_path *s);     		// free memory allocated by path structure
t_error path_reset(s_path *s);     		// set pointers to begin of path
t_bool  path_set_link(s_path *s); 		// move to next link
const char  *path_get_link(s_path *s);  	// return pointer to link, doesn't move pointers, first charakter is flag->0-leaf, 1-last link, 2-last link and branch
t_bool  path_is_last(s_path *s); 		// _T_ if last link
char    path_get_type(s_path *s); 		// 0 - path, 1 - key, 2 - last path link
t_bool  path_cmp(s_path *p, const char *str);	// compare link with string

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__CFP_H__

