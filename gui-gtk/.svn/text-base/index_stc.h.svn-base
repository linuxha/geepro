/* simple parser for index file stencil.stc
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

#ifndef __gui_xml_h__
#define __gui_xml_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    FILE *f;
    long length;    
    const char *fname;
    char *aux, *buff;
    char *position;
    char *current;
    char *found;
    char chng;
    const char *fnm, *tr, *dsc; // set after lookup
} index_stc_str;

// open index file, if open file doesn't exist then create it, return NULL if error
// WARN!: fname pointer is copied to inside usage, but not content !
index_stc_str *index_stc_open(const char *fname); 
char index_stc_save(index_stc_str *); // save changes to file
void index_stc_close(index_stc_str *);
char index_stc_add( index_stc_str *, const char *tree, const char *name, const char *desc, const char *fpath);
char index_stc_del( index_stc_str *, const char *tree, const char *name);
void index_stc_rewind(index_stc_str *);	  // set position to buffer begin
char index_stc_get_line(index_stc_str *); // result is in idx->aux, move idx->position to next line
void index_stc_split_line(index_stc_str *, const char **fpath, const char **tree, const char **description);
char index_stc_split_path(const char *tree, const char **name); // split tree path to path and name, if no name then return 0, else 1
char index_stc_path_lookup(index_stc_str *, const char *tree_path, const char *name);

/*
    Example usage:
int main()
{
    const char *pth = NULL, *tr = NULL, *dsc = NULL, *name, *x;
    index_stc_str *idx;
    
    idx = index_stc_open("stencil.idx");
    index_stc_add( idx, "/aa/bb/cc","dd","eee","fff");
    index_stc_add( idx, "/gg/hh/ii","jj","kkk","lll");
    index_stc_del( idx, "/gg/hh/ii","jj");

    index_stc_rewind( idx );
    while(index_stc_get_line( idx )){
	index_stc_split_line(idx, &pth, &tr, &dsc);
	index_stc_split_path( tr, &name);
	printf("-->'%s', '%s', '%s', '%s'\n", pth, tr, dsc, name);
    }    

    index_stc_save( idx );
    index_stc_close( idx );    
    return 0;
}
    
*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif

