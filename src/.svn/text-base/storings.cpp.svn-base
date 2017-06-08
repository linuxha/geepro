/* geepro - Willem eprom programmer for linux
 * Copyright (C) 2011 Krzysztof Komarnicki
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
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
    #include "storings.h"
}

int store_constr(store_str *st, const char *path, const char *fname)
{
    DIR *dir;
    const char *home;
    char *tmp = (char *)path, *x;
    st->file = NULL;
    st->sval = NULL;
    st->ival = 0;
    st->fval = 0;
    // rozwinięcie katalogu domowego jesli jes w sciezce
    home = getenv("HOME"); // scieżka katalogu domowego
    if((x = strchr((char *)path, '~'))){
	tmp = (char *)malloc(strlen(path) + strlen(home) + 1);
	sprintf(tmp,"%s%s", home, x + 1);
    }        
    // test czy istnieje katalog
    if((dir = opendir(tmp)) == NULL){
	if(errno != ENOENT){
	    free(tmp);
	    return errno; // cannot open directory
	}
	if(mkdir(tmp, S_IRWXU) < 0){
	    free(tmp);
	    return errno;	
	}
	printf("[MSG]Creating directory %s\n", tmp);
    } else   
	closedir(dir);    
    x = (char *)malloc(strlen(tmp) + strlen(fname) + 2); // 2 - '/' + \0 
    sprintf(x, "%s/%s", tmp, fname);    
    if(!(st->file = fopen(x, "r+"))){
	if(!(st->file = fopen(x, "w")))
	    printf("[WARN]Open file %s/%s error. ERRNo = %s\n", tmp,fname, strerror(errno));
    }
    free(tmp);
    free(x);
    return 0;
}

void store_destr(store_str *st)
{
    if(st->file) fclose(st->file);
    if(st->sval) free(st->sval);
}

static char *store_buffer(FILE *f)
{
    unsigned int len;
    char *tmp;
    // get file size
    fseek(f, 0L, SEEK_END);
    len = ftell(f);
    fseek(f, 0L, SEEK_SET);
    // memory allocation
    if(!(tmp = (char *)malloc(len + 1))) return NULL;    
    memset(tmp, 0, len + 1);
    if(fread(tmp, 1, len, f) != len) return NULL;
    return tmp;
}

static char *store_lookup(const char *buffer, const char *k)
{
    char tmp[256];
    sprintf(tmp, "$%s=", k);
    return strstr((char *)buffer, tmp);
}

int store_get(store_str *st, const char *key, char **val)
{
    int len;
    char *buffer, *tmp, *x;
    if(!val ) return -3;
    if(!st ) return -3;
    if(!st->file ) return -1;
    if(!key) return -3;
    if(strlen(key) > 250) return -3;
    if(strchr(key, '$')) return -2;
    if(!(buffer = store_buffer(st->file))) return -1;
    tmp = store_lookup(buffer, key);
    if(tmp){
	tmp = strchr(tmp, '=');
	if(*tmp == '=') tmp++;
	    else return -4;
	if(!(x = strchr(tmp, '\n'))) return -4;
	len = x - tmp;
	*val = (char *)malloc(len + 1);	
	strncpy(*val, tmp, len);
	*(*val + len) = '\0';
    }
    free(buffer);
    return 0;    
}

int store_set(store_str *st, const char *key, const char *str)
{
    char *buffer, *tmp, *x;
    if( !st  ) return -3;
    if( !str ) return -3;
    if( !key ) return -3;
    if(!st->file ) return -1;
    if(strlen(key) > 250) return -3;
    if(strchr(str, '=') || strchr(key, '$') || strchr(str, '$')) return -2;
    if(!(buffer = store_buffer(st->file))) return -1;
    tmp = store_lookup(buffer, key);
    if(tmp){
	x = strchr(tmp, '\n');
	if(*x == '\n') x++;
	while(*x) *tmp++ = *x++;
	*tmp = '\0';
    }
    fseek(st->file, 0L, SEEK_SET);
    if(ftruncate(fileno(st->file), 0) < 0) return -4; // wyzerowanie pliku
    fprintf(st->file, "%s$%s=%s\n", buffer, key, str);
    free(buffer);
    return 0;    
}

