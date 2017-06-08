/* parser for index file stencil.stc
 * Copyright (C) 2007 Krzysztof Komarnicki
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
#include "index_stc.h"
#include "../../src/error.h"

index_stc_str *index_stc_open(const char *fname)
{
    index_stc_str *idx;

    if(!(idx = (index_stc_str *)malloc(sizeof(index_stc_str)))){
	ERROR(E_ERR, E_T_MALLOC);
	return NULL;
    }
    if(!(idx->f = fopen(fname, "a+"))) return NULL;
    fseek(idx->f, 0L, SEEK_END);
    idx->length = ftell(idx->f);
    fseek(idx->f, 0L, SEEK_SET);    
    if(idx->length == 0)
	fprintf(idx->f, "#Automatically generated - don't edit\n#file path:menu path:\"description\"\n");
    if(!(idx->aux = (char *)malloc( idx->length + 16 ))){
	ERROR(E_ERR, E_T_MALLOC );
	return NULL;
    }
    if(!(idx->buff = (char *)malloc( idx->length + 1 ))){
	ERROR(E_ERR, E_T_MALLOC );
	return NULL;
    }
    memset(idx->buff, 0, idx->length + 1);
    if(fread(idx->buff, 1, idx->length, idx->f) != idx->length){
	ERROR(E_ERR, "Cannot read from file %s", fname);
	return NULL;    
    }
    idx->position = idx->current = idx->buff;
    idx->fname = fname;
    idx->found = NULL;
    idx->chng = 0;
    return idx;    
}

void index_stc_close(index_stc_str *idx)
{
    if(!idx) return;
    if(idx->f) fclose(idx->f);
        
    if(idx->aux) free(idx->aux);
    if(idx->buff) free(idx->buff);
    free( idx );
}

void index_stc_rewind(index_stc_str *idx)
{
    if( !idx ) return;
    idx->position = idx->current = idx->buff;
    idx->found = NULL;
}

char index_stc_get_line(index_stc_str *idx)
{
    char *tmp;
    char sk = 0;
    char cmt = 0;
    char qt = 0;
    if( !idx ) return 0;        
    if( !*idx->position ) return 0;
    idx->current = idx->position;    
    memset(idx->aux, 0, idx->length + 16);
    tmp = idx->aux;
    for( ;*idx->position; idx->position++){
	if( cmt ){
	    if( *idx->position == '\n') cmt = 0;
	    idx->current = idx->position;    
	    continue;
	}
	if(!sk){
	    if( *idx->position == '\\'){
		sk = 1;
		continue;
	    }
	    if(!cmt){
		if((*idx->position == '#') && !qt){
		    cmt = 1;
		    continue;
		}
		if( *idx->position == '"') qt = !qt;	       
	    }
        }
	if(!qt && (*idx->position == '\n')) break;
        sk = 0;
	*tmp = *idx->position;
	tmp++;	    
    }
    idx->position++;    
    return 1;
}

void index_stc_split_line(index_stc_str *idx, const char **fpath, const char **tree, const char **desc)
{
    char *tmp;

    *fpath = NULL;
    *tree  = NULL;
    *desc  = NULL;
    if( !idx->aux ) return;
    if( !*idx->aux ) return;
    if(!(tmp = strchr( idx->aux, ':'))){
	ERROR(E_WRN, "index file '%s' syntax error (missing ':' after path string) in line: '%s'\n", idx->fname, idx->aux);
	return;
    }
    *fpath = idx->aux;
    *tmp = 0; // end string
    tmp++;    // skip '\0'
    *tree = tmp;
    if(!(tmp = strchr( tmp, ':'))){
	ERROR(E_WRN, "index file '%s' syntax error (missing ':' after tree path string) in line: '%s'\n", idx->fname, *tree);
	return;
    }
    *tmp = 0;
    tmp++;
    for(;*tmp && *tmp <= ' '; tmp++); // skip white chars at beginning
    if(*tmp != '"') return;
    tmp++;
    *desc = tmp;        
    do{
	if(!(tmp = strchr( tmp, '"'))){
	    ERROR(E_WRN, "index file '%s' syntax error missing '\"' in line '%s'\n", idx->fname, *desc);
	    return;
	}
    }while( *(tmp-1) == '\\');
    *tmp = 0;
}

char index_stc_split_path(const char *path, const char **name)
{
    char *tmp;

    if(!path) return 0;
    *name = NULL;
    if(!(tmp = strchr( path, '$'))) return 0;    
    *name = tmp + 1;    
    if( tmp > path) *(tmp - 1) = 0;    
    return 1;
}

char index_stc_path_lookup(index_stc_str *idx, const char *tree, const char *name)
{
    const char *pth = NULL, *tr = NULL, *dsc = NULL, *nme;
    char *tmp;
    if( !idx ) return 0;    

    idx->found = NULL;
    idx->fnm = NULL; idx->tr = NULL; idx->dsc = NULL;
    index_stc_rewind( idx );
    while(index_stc_get_line( idx )){
	index_stc_split_line(idx, &pth, &tr, &dsc);
	idx->fnm = pth;
	idx->tr = tr;
	idx->dsc = dsc;
	index_stc_split_path( tr, &nme);
	if( !tr || !nme ) continue;
	if(!strcmp( tr, tree) && !strcmp(nme, name)){
	    tmp = idx->current;
	    for(;*tmp && *tmp <= ' '; tmp++); // skip white chars at beginning
	    idx->found = tmp;
	    return 1;
	}
    }    
    return 0;
}

char index_stc_del( index_stc_str *idx, const char *tree, const char *name)
{
    int len, rr;
    char *from, *to;
    if( !idx ) return 0;
    if(!index_stc_path_lookup( idx, tree, name)){
	ERROR(E_WRN, "Try to delete not existed tree path '%s/$%s' in file '%s'", tree, name, idx->fname);
	return 0;
    }
    from = idx->found;
    to = from;    
    idx->found = NULL;
    to = idx->position;
    if(to <= from) return 1;
    len = strlen( to );
    *from = 0;
    rr  = strlen(idx->buff);
    memcpy( from, to, len);
    idx->buff[len + rr] = 0;
    idx->chng = 1;

    return 1;
}

char index_stc_add( index_stc_str *idx, const char *tree, const char *name, const char *desc, const char *fpath)
{
    char *tmp;
    int x;
    if( !idx ) return 0;
    if(!tree || !name || !desc || !fpath) return 0;
    x = idx->length + strlen( tree ) + strlen( name ) + strlen( desc ) + strlen( fpath ) + 8;
    tmp = (char *)realloc( idx->buff, x);
    if(!tmp){
	ERROR(E_ERR, E_T_MALLOC);
        return 0;
    }
    idx->length = x;
    idx->buff = tmp;
    sprintf(tmp, "%s%s:%s/$%s:\"%s\"\n", idx->buff,fpath, tree, name, desc);
    tmp = (char *)realloc( idx->aux, x + 16);    
    if(!tmp){
	ERROR(E_ERR, E_T_MALLOC);
        return 0;
    }
    idx->aux = tmp;    
    idx->chng = 1;
    return 1;
}

char index_stc_save(index_stc_str *idx)
{
    int x;
    if(!idx) return 0;
    if(!idx->chng || !idx->f || !idx->buff) return 0;
    x = strlen(idx->buff);
    fclose(idx->f);
    if(!(idx->f = fopen(idx->fname, "w"))){
	ERROR(E_WRN, "Cannot reopen file '%s' for writing", idx->fname);
	return 0;
    }
    if(fwrite(idx->buff, 1, x, idx->f) != x){
	ERROR(E_ERR, "save file '%s' error", idx->fname);
	return 0;	
    }    
    return 1;
}

