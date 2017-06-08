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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "be_buffer.h"

static inline unsigned int gui_bineditor_buff_str_asm(unsigned char *word, const char *p0, const char *p1, int *error)
{
    char tmp[16], *z;
    const char *x;
    unsigned char *d;
    char bs = 0;
    unsigned int count = 0;
    *error = 0;    
    d = word;
    if( *p0 == '"') { // string copy 
	for( x = p0 + 1; *x && (x != p1); x++){
	    if(( *x == '"' ) && !bs) break;
	    if(( *x == '\\' ) && !bs){
		bs = 1;
		continue;
	    }
	    bs = 0;    
	    *d++ = *x;
	    count++;
	}   
	*d = 0; 
    } else{
	z = tmp;
	for( x = p0; *x && (x != p1); x++) *z++ = *x; *z = 0;
	*d = (unsigned char )strtoul(tmp, NULL, 0); // temporary, should be exchanged by more flexible function
	count = 1;
    }
    return count;
}

unsigned char *gui_bineditor_buff_pattern2data(const char *data, unsigned int *data_size, int *error)
{
    const char *pt0,*pt1;
    unsigned char *word;
    int len;
    char qt, zt;

    len = strlen(data);
    *data_size = 0;
    *error = 0;
    word = (unsigned char *)malloc( len + 1);
    if( word == NULL ){
	printf("ERR: gui_bineditor_buff_pattern2data() -> Memory problem.\n");
	return NULL;
    }
    *word = 0;

    for(pt0 = pt1 = data; *pt1; pt1++){
	qt = zt = 0;
	// clean from leading white characters
	for(;*pt1 && (*pt1 < 33); pt1++);
	// looking for word
	for(pt0 = pt1; *pt1; pt1++){
	    if(*pt1 < 32) break;
	    if( ( *pt1 == ' ' ) && !qt ) break;
	    if( ( *pt1 == ',' ) && !qt ) break;	    
	    if( ( *pt1 == '"' ) && !zt ) qt = !qt;
	    zt = 0;
	    if( *pt1 == '\\' ) zt = 1;
	}
	if( pt0 != pt1 )
	    *data_size += gui_bineditor_buff_str_asm(word + *data_size, pt0, pt1, error);
	if( *pt1 == 0 ) break;
    }
    return word;
}

void gui_bineditor_buff_history_add(gui_be_buffer_str *bf, unsigned int from, unsigned int count)
{
    printf("history not implemented yet\n");
}

void gui_bineditor_buff_history(gui_be_buffer_str *be, int op)
{
    printf("history: %i\n", op);
}

/*******************************************************************************************************************************************/

void gui_bineditor_buff_constr(gui_be_buffer_str **bf)
{
    *bf = (gui_be_buffer_str *)malloc( sizeof( gui_be_buffer_str ) );
    if(*bf == NULL){
	printf("Critical error: memory allocation in gui_bineditor_buff_constr()\n");
	exit(-1);
    }
    (*bf)->ftmp = tmpfile();
    if((*bf)->ftmp == NULL ){
	printf("Error: temporary file creation for history in gui_bineditor_buff_constr() can not be created!\n");
    }
    (*bf)->size = 0;
    (*bf)->data = NULL;
    printf("Temporary file for history created.\n");
}

void gui_bineditor_buff_destroy(gui_be_buffer_str **bf)
{
    if((*bf)->ftmp){
	 fclose((*bf)->ftmp);
	 printf("Temporary file cleanup.\n");
    }
    if(*bf) free(*bf);
    *bf = NULL;
}

char gui_bineditor_buff_edit(gui_be_buffer_str *bf, unsigned int from, unsigned int count, unsigned char *data)
{
    char error;
    
    error = 0;
    
    if(bf == NULL){
	printf("Error: gui_bineditor_buff_edit() buffer == NULL -> ignoring function.\n");
	return 1;
    }        
    if(from >= bf->size){
	printf("Warn: gui_bineditor_buff_edit() insert address is greater than buffor -> ignoring function.\n");    
	return 2;
    }    
    if(from + count >= bf->size){
	count = bf->size - count - 1;
	error = 128;
    }    

    gui_bineditor_buff_history_add(bf, from, from + count - 1);
    memcpy(bf->data + from, data, count);

    return error;
}

char gui_bineditor_buff_clr(gui_be_buffer_str *bf, unsigned int from, unsigned int to, const char *pattern)
{
    unsigned char *data;    
    unsigned int  data_size = 0, i, j ;
    int error = 0;
    
    if( from >= bf->size) error = 1;
    if( to >= bf->size) error = 2;

    if(pattern == NULL) return 0;
    if(*pattern == 0) return 0;

    if( error ){
	printf("Warning: gui_bineditor_buff_clr() -> Address exceed buffer. Ignoring.\n");
	return error;
    }
    
    data = gui_bineditor_buff_pattern2data( pattern, &data_size, &error);
    if( error ){
	printf("Warning: gui_bineditor_buff_clr() -> Pattern error. Ignoring.\n");
	return error;
    }

    // history
    gui_bineditor_buff_history_add(bf, from, to);

    // fill buffer
    for(i = from; i <= to;){
	    for(j = 0;(i <= to) && (j < data_size); i++, j++) bf->data[i] = data[j];
    }    
    
    if( data ) free( data );
    return 0;
}

char gui_bineditor_buff_find(gui_be_buffer_str *bf, const char *find, unsigned int size, unsigned int *from, unsigned int to, char ci)
{
    unsigned int i;

    if( bf->data == NULL ) return 0;
    if( find == NULL ) return 0;

    for(; (*from < bf->size) && (*from <= to); (*from)++){    
	if( *from + size >= bf->size) return 0;
	if(!ci){
	    if(memcmp( bf->data + *from, find, size ) == 0) return 1;
	} else {
	    for(i = 0; i < size; i++){
		if( tolower( bf->data[*from + i] ) == tolower( find[ i ] )) return 1;
	    }
	}
    }
    return 0;
}

static char gui_bineditor_bit_copy(char input, const char *rel, char bits)
{
    int i;
    char tmp, mask;
    
    for(mask = 1, i = 0, tmp = 0; i < bits; i++, mask <<= 1)
	    tmp |= ((1 << rel[i]) & input) ? mask : 0;
    
    return tmp;
}
                          // bit: 0 1 2 3 4 5 6 7
static const char shl_const[8] = {0,0,1,2,3,4,5,6};
static const char shr_const[8] = {1,2,3,4,5,6,7,7};
static const char rol_const[8] = {7,0,1,2,3,4,5,6};
static const char ror_const[8] = {1,2,3,4,5,6,7,0};

static inline char gui_bineditor_func( int arg, char func, char *rel, char input)
{
    switch( func ){
	case GUI_BINEDITOR_BM_FUNC_ADD: return input + arg;
	case GUI_BINEDITOR_BM_FUNC_SUB: return input - arg;
	case GUI_BINEDITOR_BM_FUNC_MUL: return input * arg;
	case GUI_BINEDITOR_BM_FUNC_DIV: if(arg != 0) return input / arg;
	case GUI_BINEDITOR_BM_FUNC_OR:  return input | arg;
	case GUI_BINEDITOR_BM_FUNC_AND: return input & arg;
	case GUI_BINEDITOR_BM_FUNC_XOR: return input ^ arg;
	case GUI_BINEDITOR_BM_FUNC_SHL: return gui_bineditor_bit_copy( input, shl_const, 8) & 0xfe;
	case GUI_BINEDITOR_BM_FUNC_SAL: return gui_bineditor_bit_copy( input, shl_const, 8 );
	case GUI_BINEDITOR_BM_FUNC_SHR: return gui_bineditor_bit_copy( input, shr_const, 8 ) & 0x7f;
	case GUI_BINEDITOR_BM_FUNC_SAR: return gui_bineditor_bit_copy( input, shr_const, 8 );
	case GUI_BINEDITOR_BM_FUNC_ROL: return gui_bineditor_bit_copy( input, rol_const, 8 );
	case GUI_BINEDITOR_BM_FUNC_ROR: return gui_bineditor_bit_copy( input, ror_const, 8 );
	case GUI_BINEDITOR_BM_FUNC_BIT: return gui_bineditor_bit_copy( input, rel, 8 );
    }

    return 0;
}

void gui_bineditor_buff_bman(gui_be_buffer_str *bf, unsigned int start, unsigned int count, int arg, char func, char *rel)
{
    unsigned int i;

    gui_bineditor_buff_history_add(bf, start, start + count - 1);
    for(i = 0; i < count; i++)
	bf->data[ i + start] = (unsigned char )gui_bineditor_func( arg, func, rel, bf->data[ i + start]);
}

// Can be optimalized for lower memory usage, without need to allocate copy of the buffer
void gui_bineditor_buff_reorg(gui_be_buffer_str *bf, unsigned int start, unsigned int count, char op, char *rel, char bits)
{
    unsigned int i, half;
    unsigned char *tmp;

    tmp = (unsigned char *)malloc( count );
    half = count / 2;
    if( tmp == NULL ){
	printf("ERROR: gui_bineditor_buff_reorg() -> memory allocation error.\n");
	return;
    }
    memcpy( tmp, bf->data + start, count );
            
    gui_bineditor_buff_history_add(bf, start, start + count - 1);
    
    switch(op){
	case GUI_BINEDITOR_ORG_SPLIT: for(i = 0; i < half; i++){
					 bf->data[ start + i] = tmp[ i * 2]; // even half
					 bf->data[ start + half + i] = tmp[ i * 2 + 1]; // odd half
				      }
				      break;
	case GUI_BINEDITOR_ORG_MERGE: for(i = 0; i < half; i++){
					 bf->data[ start + i*2] = tmp[ i ]; // even half
					 bf->data[ start + i*2 + 1] = tmp[ half + i]; // odd half
				      }
				      break;
	case GUI_BINEDITOR_ORG_XCHG: for(i = 0; i < count; i++){
					 bf->data[ start + i * 2] = tmp[ i * 2 + 1]; // even half
					 bf->data[ start + i * 2 + 1] = tmp[ i * 2]; // odd half
				      }
				      break;
	case GUI_BINEDITOR_ORG_REORG: for(i = 0; i < count; i++)
				         bf->data[ start + i ] = gui_bineditor_bit_copy( tmp[i], rel, bits );
				      break;
				      
    }
    free( tmp );
}

char gui_bineditor_buff_file_insert(gui_be_buffer_str *bf, FILE *fh, long offs, int start, int count)
{
    fseek(fh, offs, SEEK_SET);
    return fread( bf->data + start, 1, count, fh) != count;
}

char gui_bineditor_buff_file_save(gui_be_buffer_str *bf, int start, int count, const char *fname)
{
    FILE *fh;
    unsigned int err;
    if((fh = fopen(fname, "w")) == NULL) return 1;
    err = fwrite( bf->data + start, 1, count, fh) != count;
    fclose( fh );
    return err != count;
}

void gui_bineditor_buff_asm(gui_be_buffer_str *be, unsigned int start, unsigned int count)
{
    printf("byte asm\n");
}
