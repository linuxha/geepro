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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cfp.h"


static s_cfpq **cfp_add_section( s_cfpq **, const char *key);
static t_bool cfp_add_entry( s_cfpq **, const char *key, const char *value);
static void cfp_free_m( s_cfpq *);
static t_bool cfp_save_file_m(s_cfpq *, const char *file_path);
static t_bool cfp_load_file_m(s_cfpq **, const char *file_path);
static t_bool cfp_cut_m(s_cfpq **, const char *path);
static t_bool cfp_add_m(s_cfpq **, const char *file_path, const char *value); // add/modify
static const s_cfpq *cfp_get_node_m(s_cfpq *, const char *path); // return node given by path or NULL if not existed


static s_cfpq *cfp_new( void )
{
    s_cfpq *p;
    
    p = (s_cfpq *)malloc( sizeof(s_cfpq ) );
    if( !p ) return NULL;
    p->prev = NULL;
    p->next = NULL;
    p->branch = NULL;
    p->value = NULL;
    p->key = NULL;
    return p;
}

static s_cfpq **cfp_add_section( s_cfpq **p, const char *key)
{
    s_cfpq *tmp, *x;
    if( !key ) return NULL;
    if( !p ) return NULL;
    tmp = cfp_new();
    if( !tmp ) return NULL;
    tmp->key = (char *)malloc( strlen( key ) + 1);
    if(!tmp->key){
	free( tmp );
	return NULL;
    }
    strcpy( tmp->key, key);
    if(!*p){
	*p = tmp;
	return &tmp->branch;
    }
    x = *p;
    while( x->next ) x = x->next;
    x->next = tmp;
    tmp->prev = x;
    return &tmp->branch;
}

static t_bool cfp_add_entry( s_cfpq **p, const char *key, const char *value)
{
    s_cfpq *tmp, *x;
    if( !key ) return _F_;
    if( !p ) return _F_;
    tmp = cfp_new();
    if( !tmp ) return _F_;
    tmp->key = (char *)malloc( strlen( key ) + 1);
    if(!tmp->key){
	free( tmp );
	return _F_;
    }
    strcpy( tmp->key, key);
    tmp->value = (char *)malloc( strlen( value ) + 1);
    if(!tmp->value){
	free( tmp->key );
	free( tmp );
	return _F_;        
    }
    strcpy( tmp->value, value);
    if(!*p){
	*p = tmp;
	return _T_;
    }
    x = *p;
    while( x->next ) x = x->next;
    x->next = tmp;
    tmp->prev = x;
    return _T_;
}

static void cfp_free_m( s_cfpq *p)
{
    s_cfpq *x;
    for(;p;){	
	x = p->next;
	if( p->branch ){
	    cfp_free_m( p->branch );
	    free( p->key );
	    free( p );
	} else {
	    free( p->key );
	    free( p->value );
	    free( p );
	}
	p = x;
    }
}

static void cfp_foreach_(s_cfpq *p, int nested, f_cfp_cb cb, void *user_data)
{
    if(!cb) return;
    for( ;p; p = p->next){	
	if( p->branch ){
	    cb(p->key, p->value, nested, P_SECTION_BEGIN, user_data);
	    cfp_foreach_( p->branch, nested + 1, cb, user_data );
	    cb(p->key, p->value, nested, P_SECTION_END, user_data);
	} else {
	    cb(p->key, p->value, nested, P_ENTRY, user_data);
	}
    }
}

static void cfp_foreach_m(s_cfpq *p, f_cfp_cb cb, void *user_data)
{
    cfp_foreach_(p, 0, cb, user_data);
}

static void cfp_filter(char *bf, char *key)
{
    char *tmp, *x;
    t_bool skip, qt, lcmt, mcmt;
    // uncomment
    *key = 0;        
    x = key;
    skip = _F_;
    qt = _F_;
    lcmt = _F_;
    mcmt = _F_;
    for(tmp = bf; *tmp; tmp++){
	if( skip ){
	    *x++ = *tmp;
	    skip = _F_;
	    continue;
	}
	if(mcmt || lcmt){
	    if( lcmt && (*tmp == '\n')){
	        lcmt = _F_;
	        *x++ = *tmp;
	        continue;
	    }
	    if( mcmt && (*tmp == '*')){
	         if( *(tmp + 1) == '/'){
	            tmp++;
	            mcmt = _F_;
	            continue;
	         }
	    }
	    continue;
	}
	if(!qt && (*tmp == '/')){
	    if(*(tmp + 1) == '/'){
		lcmt = _T_;
		continue;
	    }
	    if(*(tmp + 1) == '*'){
		mcmt = _T_;
		continue;
	    }
	}
	if( *tmp == '"') qt = !qt;
	if( qt && *tmp == '\\') skip = _T_;
	*x++ = *tmp;
    }
    *x = 0;
    // clean white characters    
    x = bf;
    qt = _F_;
    skip = _F_;
    for(tmp = key; *tmp; tmp++){
	if( skip ){
	    skip = _F_;
	    *x++ = *tmp;
	    continue;
	}	
	if( *tmp == '"') qt = !qt;
	if( qt && *tmp == '\\') skip = _T_;
        if( !qt ){
          if(*tmp < 33) continue;
        }
	*x++ = *tmp;
    }
    *x = 0;
}

static t_bool cfp_is_key_char(char k)
{
    return (( k >= 'a') && ( k <= 'z' )) || 
	    (( k >= 'A') && ( k <= 'Z' )) || 
	     (( k >= '0') && ( k <= '9' )) || 
	      ( k == '=') || ( k == '[') || ( k == ']') || ( k == ',') || ( k == '_') || ( k == ':')|| ( k == '.');
}

static t_bool cfp_unbracket(char *t)
{
    int x = strlen( t );    
    
    for(;x;x--)
	if(t[x] == '}'){
	    t[x] = 0;
	    return _T_;
	}
    return _F_;
}

static t_bool cfp_split(s_cfpq **p,char *bf, char *key, char *val)
{
    int nested;
    s_cfpq **n;
    t_bool qt, skip;
    char *tmp, *x, *z;
    tmp = bf;
    qt = _F_;
    skip = _F_;
    nested = 0;
    while( *tmp ){
	*key = 0; *val = 0;
	for(x = key; *tmp; tmp++){
	    if( !cfp_is_key_char( *tmp ) ){
		ERROR(E_ERR, "illegal character '%c'", *tmp);	    
		return _F_;
	    }
	    if( *tmp == '=' ) break;
	    *x++ = *tmp;
	}   
	*x = 0;
	if( *tmp == '='){
	    tmp++;
	} else {
	    ERROR(E_ERR, "missing '='");
    	    return _F_;
	}
	for(x = val; *tmp; tmp++){
	    if( !skip ){
		if(qt && *tmp == '\\') skip = _T_;
		if((*tmp == '"') && !skip) qt = !qt;
		if(!qt){
		    if( *tmp == '{') nested++;
		    if( *tmp == '}' && nested) nested--;
		    if( (nested == 0) && (*tmp == ';')) break; 
		}
    	    } else
    		skip = _F_;
	    *x++ = *tmp;
	}   
	*x = 0;
	tmp++;
	if( *val == '{'){
	    if(!cfp_unbracket( val )){
		ERROR(E_ERR, "missing '}'");
		return _F_;
	    }
	    n = cfp_add_section( p, key); 
	    z = (char *)malloc( strlen( val ) + 1);
	    if(!z){
		ERROR(E_ERR, "malloc");
		return _F_;		
	    }
	    if(!cfp_split(n, val + 1, key, z)){
		free( z );
	        return _F_; 
	    }
	    free( z );
	} else {
	    cfp_add_entry( p, key, val);
	}	
    }
    return _T_; 
}

static t_bool cfp_load_file_m(s_cfpq **p, const char *path)
{
    t_bool err;
    char *bf;
    char *key, *value;
    long fsize;
    FILE *f;
//    if(cfp_validator(path, &bf)){
//	ERROR(E_WRN,"%s", bf);
//	free( bf );
//	return _F_;    
//    }
    if(!(f = fopen(path, "r"))){
	ERROR(E_ERR,"cannot open file %s", path);
	return _F_;
    }
    fseek(f, 0L, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    if(!(bf = (char *)malloc( fsize + 1))){
	fclose(f);
	ERROR(E_ERR,"malloc()");
	return _F_;
    }
    memset(bf, 0, fsize + 1);
    if(fread(bf, 1, fsize, f) != fsize){
	fclose(f);
	ERROR(E_ERR,"loading file %s", path);
	return _F_;
    }
    fclose(f);
    
    if(!(key = (char *)malloc( fsize + 1))){
	ERROR(E_ERR,"malloc()");
	free( bf );
	return _F_;
    }
    if(!(value = (char *)malloc( fsize + 1))){
	free( bf );
	free( key );
	ERROR(E_ERR,"malloc()");
	return _F_;
    }
    cfp_filter(bf, key); // uncomment, delete white characters
    err = cfp_split(p, bf, key, value);
    free( bf );
    free( key );
    free( value );
    return err;
}

static void cfp_save_cb(const char *key, const char *value, int nested, t_cfp_mode mode, FILE *f)
{
    int i;
    for( i=0; i < nested*3;i++) fputc(' ', f);
    switch( mode ){
	case P_SECTION_BEGIN: fprintf(f,  "%s = {\n", key); break;
	case P_SECTION_END: fprintf(f, "};\n"); break;
	case P_ENTRY: fprintf( f, "%s=%s;\n", key, value ? value : "{}"); break;
    }    
}

static t_bool cfp_save_lo(s_cfpq *p, const char *path)
{
    FILE *f = NULL;
    if(!(f = fopen( path, "w"))){
	ERROR(E_ERR, "cannot open file %s to write", path);
	return _F_;
    }
    cfp_foreach_m( p, (f_cfp_cb)cfp_save_cb, (void *)f);
    fclose( f );
    return _T_;
}

static t_bool cfp_save_file_m(s_cfpq *p, const char *path)
{
    FILE *f;
    t_bool ren = _F_;
    if((f = fopen(path, "r"))){ // just test if file exist
	fclose(f);
	if( rename(path,"_$temporary$_._$_") ){
	    ERROR(E_ERR, "cannot change file name %s", path);
	    return _F_;
	}
	ren = _T_;
    }
    if(!cfp_save_lo(p, path)){
	ERROR(E_ERR, "create file error");
	if(ren) rename("_$temporary$_._$_", path);
	return _F_;
    }
    if(ren) remove("_$temporary$_._$_");
    return _T_;    
}

const s_cfpq *cfp_find(const s_cfpq *p, const char *key)
{
    while( p ){
	if(!p->key) return NULL;
	if(!strcmp(p->key, key)) return p;
	p = p->next;
    }    
    return NULL;
}

const s_cfpq *cfp_block(const s_cfpq *p)
{
    if( !p ) return NULL;
    return p->branch;
}

static t_bool cfp_cut_m(s_cfpq **ps, const char *path)
{
    s_path p;
    s_cfpq *pz, *gz, *last_branch = NULL;
    const char *link;
    
    if(path_start( &p, path)) return _F_;
    pz = *ps;
    while(path_set_link( &p )){
	link = path_get_link( &p );
	if(!(pz = (s_cfpq*)cfp_find(pz, link))){
	    ERROR(E_ERR,"link '%s' is not a proper link", link);
	    break;	
	}
	gz = pz->branch;
	if(path_is_last(&p)){
	    if( pz->value ){ 
		free( pz->value );
		pz->value = NULL;
	    }
	    if( pz->branch ){
		cfp_free_m( pz->branch );
		pz->branch = NULL;
	    }
	    if( pz->key ){
		free( pz->key );
		pz->key = NULL;
	    }
	    if( pz->next ){
		pz->next->prev = pz->prev;
	    }
	    if( pz->prev ){
		pz->prev->next = pz->next;
	    } else {
		if( last_branch ){
		    last_branch->branch = pz->next;
		} else {
		    *ps = pz->next;
		}
	    }
	    free( pz );
	    return _T_;
	}
	if(pz->branch){
	    last_branch = pz;
	    pz = gz;
	}
    }
    path_end( &p );
    return _T_;
}

static t_bool cfp_add_m(s_cfpq **ps, const char *path, const char *value)
{
    s_path p;
    s_cfpq **pz = ps, *pk;
    const char *link;
    char *pr;
    
    if(path_start( &p, path)) return _F_;
    while(path_set_link( &p )){
	link = path_get_link( &p );
	if(strlen(link) == 0){
	    ERROR(E_ERR,"empty link name in path '%s'", path);
	    return _F_;
	}
	if((pk = (s_cfpq*)cfp_find( *pz, link))){ // link exist
	    if(path_is_last( &p )){ // modify
		if( pk->branch ){ // modify section name
		    if(!(pr = realloc( pk->key, strlen(value) + 1))){
			ERROR(E_ERR, "realloc");
			return _F_;
		    }
		    pk->key = pr;
		    strcpy(pk->key, value);
		} else {
		   if(!(pr = realloc( pk->value, strlen(value) + 1))){
			ERROR(E_ERR, "realloc");
			return _F_;
		   }
		   pk->value = pr;
		   strcpy(pk->value, value);
		}
		return _T_;	    
	    } else { // follow link
		pz = &pk->branch;                
	    }
	} else { // create link
	    if(path_is_last( &p )){	
		if( !value )
		    cfp_add_section( pz, link);
		else
		    cfp_add_entry( pz, link, value);
	    } else { // create section
		pz = cfp_add_section( pz, link);
	    }
	}
    }
    path_end( &p );
    return _T_;
}

static const s_cfpq *cfp_get_node_m(s_cfpq *ps, const char *path)
{
    s_path p;
    const char *link;
    s_cfpq *pk;
        
    if(path_start( &p, path)) return _F_;
    while(path_set_link( &p )){
	link = path_get_link( &p );
	if((pk = (s_cfpq*)cfp_find( ps, link))){ // link exist
	    if(path_is_last( &p ))
		return pk;
	    else
    		ps = pk->branch;	
	} else
	    return NULL;
    }
    path_end( &p );
    return NULL;
}

void cfp_free(s_cfp *p)
{
    if( !p ) return;
    cfp_free_m(p->p);
    free( p );
}

t_bool cfp_save(s_cfp *p, const char *fpath)
{
    if( !p || !fpath) return _F_;
    return cfp_save_file_m(p->p, fpath);
}

t_bool cfp_load(s_cfp *p, const char *fpath)
{
    if( !p || !fpath) return _F_;
    return cfp_load_file_m(&p->p, fpath);
}

t_bool cfp_cut(s_cfp *p, const char *path)
{
    if( !p || !path) return _F_;    
    return cfp_cut_m( &p->p, path);
}

t_bool cfp_add(s_cfp *p, const char *path, const char *val)
{
    if( !p || !path || !val) return _F_;    
    return cfp_add_m(&p->p, path, val);    
}

const s_cfpq *cfp_get(s_cfp *p, const char *path)
{
    if( !p || !path) return NULL;
    return cfp_get_node_m(p->p, path);
}

s_cfp *cfp_init(void)
{
    s_cfp *p;
    p = ( s_cfp *)malloc( sizeof(s_cfp) );
    if( !p ){
	ERROR(E_ERR, "malloc");
    }
    p->p = NULL;
    p->current = NULL;
    return p;
}

const s_cfpq *cfp_next(const s_cfpq *p)
{
    if( !p ) return NULL;    
    return p->next;
}

const s_cfpq *cfp_prev(const s_cfpq *p)
{
    if( !p ) return NULL;    
    return p->prev;
}

t_bool cfp_is_section(s_cfpq *p)
{
    if( !p ) return _F_;
    return p->branch && 1;
}

void cfp_foreach(s_cfp *p, f_cfp_cb cb, void *user_data)
{
    if( !p ) return;
    cfp_foreach_m(p->p, cb, user_data);
}

const char *cfp_get_key(const s_cfpq *p)
{
    if( !p ) return NULL;
    return p->key;
}

const char *cfp_get_val(const s_cfpq *p, const char *key)
{
    if(!(p = cfp_find( p, key))) return 0;
    if( !p ) return NULL;
    return p->value;
}

char *cfp_get_val_stringp(s_cfp *p, const char *key)
{
    return cfp_get_val_string_lo( cfp_get( p, key) );
}

char *cfp_get_val_string(s_cfpq *p, const char *key)
{
    return cfp_get_val_string_lo( cfp_find( p, key) );
}

char *cfp_get_val_string_lo(const s_cfpq *p)
{
    char *tmp;
    char *z1, *z2, *t;
    int x;
    if( !p ) return NULL;
    if( !p->value) return NULL;
    x = strlen(p->value);
    tmp = (char *)malloc(x + 1);
    if(!tmp ){
	ERROR(E_ERR, "malloc");
	return NULL;
    }
    memset(tmp, 0, x + 1);
    z1 = strchr(p->value, '"');
    if(!z1 ) return NULL; // did not opening quotation "...
    z2 = z1;
    do {
	z2++;
	if(! *z2 ) return NULL;
	z2 = strchr(z2, '"');
	if(!z2 ) return NULL;
    } while( *(z2 - 1) == '\\');
    t = tmp;
    z1++; // skip '"'
    while( z1 != z2) *t++ = *z1++;
    *z1 = 0; // cut '"'
    return tmp;
}

long cfp_get_val_int(const s_cfpq *p, const char *key)
{
    if(!(p = cfp_find( p, key))) return 0;
    if( !p->value) return 0;
    return strtol( p->value, NULL, 0);
}

double cfp_get_val_float(const s_cfpq *p, const char *key)
{
    if(!(p = cfp_find( p, key))) return 0.0;
    if( !p ) return 0.0;
    if( !p->value) return 0.0;
    return strtod( p->value, NULL);
}

t_bool cfp_get_val_bool(const s_cfpq *p, const char *key)
{
    int i;
    const char *comp[] = {"true","1",".t.","high","on","enabled","enable"};
    char tmp[16], *t;
    if(!(p = cfp_find( p, key))) return _F_;
    if( !p ) return _F_;
    if( !p->value) return _F_;
    strncpy( tmp, p->value, 15);
    t = tmp;
    while(*t) *t = tolower( *t );
    for(i=0; i < sizeof(comp); i++)
	if(!strcmp(tmp, comp[i] )) return _T_;
    return _F_;
}

t_error path_start( s_path *s, const char *path)
{
    s->path = path;
    s->start = path;
    s->end = path;
    s->length = 0;
    s->link = NULL;
    if( !path || !s ){
	ERROR(E_ERR, "(path && (struct s_path)) == NULL");
	return E_PAR;
    }
    if(*path != '/'){
	ERROR(E_ERR, ERR_MISSING_BS);
	return E_PAR;
    }
    if(!(s->link = (char *)malloc( strlen( path ) + 2))){
	ERROR(E_ERR, E_T_MALLOC);
	return E_MEM;
    };
    s->size = strlen(path);    
    return E_OK;
}

void path_end( s_path *s)
{
    if(!s) return;
    if(s->link) free(s->link);
}

t_error path_reset(s_path *s)
{
    if( !s ) return E_OK;
    if( !s->path ){
	ERROR(E_ERR, "path == NULL");
	return E_PAR;
    }
    s->start = s->path;
    s->end = s->path;
    s->size = strlen(s->path);    
    s->length = 0;
    return E_OK;
}

t_bool path_set_link(s_path *s)
{
    char *link, *tmp;
    if( !s ) return _F_;
    if( !s->path ) return _F_;
    if( !*s->path ) return _F_;
    if( !*s->start ) return _F_;
    strcpy(s->link,"0");
    s->start = s->end;
    // skip first '/'
    if( *s->start != '/' ) return _F_; // invalid path
    if( *(s->start + 1) == 0){
	if( s->start == s->path ){
	    s->end = s->start + 1;
	    if(*(s->end) == 0) (*s->link)++;
	    return _T_;
	}
	return _F_;
    }
    s->start++;
    if( !*s->start ) return _F_; 
    s->end = s->start;
    s->length = 0;
    for(; *s->end && (*s->end != '/'); s->length++, s->end++);
    if(*s->end == '/'){	// 'link/'
	if( *(s->end + 1 ) == 0){
	    (*s->link)++;
	    s->end++;
	}
    }
    for(link = s->link + 1, tmp = (char *)s->start; tmp != s->end && *tmp != '/'; tmp++, link++) *link = *tmp;
    *link = 0;
    if(*(s->end) == 0) (*s->link)++;
    return _T_;    
}

t_bool path_is_last(s_path *s)
{
    if( !s ) return _F_;
    return *(s->end) == 0;
}

t_bool path_cmp(s_path *p, const char *str)
{
    const char *tmp;
    int i;
    if( str == NULL ) return 0;
    for(i=0, tmp = p->start; tmp != p->end; tmp++, str++) if( *tmp != *str) i++;
    return i == strlen( str );
}

const char *path_get_link(s_path *s)
{
    if( !s ) return NULL;    
    return s->link + 1;
}

char path_get_type(s_path *s)
{
    if( !s ) return 0;    
    return *s->link - '0';
}

int cfp_get_val_select(const s_cfpq *p, char mode, const char *key, const char *values)
{
    return -1;
}


