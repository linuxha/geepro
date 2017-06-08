/* $Revision: 1.3 $ */
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
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <regex.h>
#include <dirent.h>

#include  "../intl/lang.h"

extern "C" {
    #include "files.h"
}

#define SAVE_HEXRECSIZE 32
#define DEBUG printf

const char *file_err_msg(int err)
{
    if(!err) return NULL;
    if( err == -1) return "Unrecognized file format";
    if( err == -3) return "Data exceed buffer";
    if( err == -4) return "Checksum error";
    if( err == -5) return "Unsupported format type 3 or 5.";
    if( err == -6) return "File save error.";
    if( err == -100) return "!File is shorter than buffer. Rest of the buffer was filled by 0xff.";
    if( err == -101) return "!File is greater than buffer. Rest of the file was cutted.";
    return "some error";
}

int file_load_bin(FILE *f, unsigned int size, char *buffer, long file_offset, long buffer_offset, long bytes_count )
{
    unsigned long sz;
    
    if(file_offset < 0){
	file_offset = 0;
	buffer_offset = 0;
	bytes_count = 0;
    } else {
	fseek( f, file_offset, SEEK_SET);
	size = bytes_count;
    }
    
    sz = fread(buffer + buffer_offset, 1, size, f);
    if( sz != size){
        if(sz < size) return -100;
        return -101;
    }
    return 0;
}

int get_char(FILE *f)
{
    int a;
    if((a = fgetc(f)) == EOF) return -1;
    if((a >= 'A') && ( a <= 'F')) a = a - 'A' + 'a';     // zamiana na małe litery
    if((a >= '0') && (a <= '9')) return a - '0';
    if((a >= 'a') && (a <= 'f')) return a - 'a' + 10; 
    return -2;
}

int get_byte(FILE *f) // gets to hex char from file and assembly to byte, if error return -1
{
    int a, b;
    if( (a = get_char(f)) < 0) return a;
    if( (b = get_char(f)) < 0) return b;    
    return a*16 + b;    
}

int file_load_hex_line(FILE *f, unsigned int size, char *buffer, int *offset)
{
    unsigned int byte_count, byte_addrh, byte_addrl, addr, type, data, i, addrext = 0;
    unsigned char sum = 0;
    // nagłówek
    while(( (fgetc(f)) != ':') && !feof(f)); // start line of hex format
    if((byte_count = get_byte(f)) < 0) return -1;
    sum += byte_count;
    if((byte_addrh = get_byte(f)) < 0) return -1;                
    sum += byte_addrh;
    if((byte_addrl = get_byte(f)) < 0) return -1;
    sum += byte_addrl;
    addr = byte_addrh * 256 + byte_addrl;    
    if((type = get_byte(f)) < 0) return -1;
    sum += type;
    if(type > 5) return -1;
    if((type == 3) || (type ==5)) return -5;
    if(type == 1) return -10; // end of file
    if(type == 2 || type == 4){
	if((byte_addrh = get_byte(f)) < 0) return -1;	    
	if((byte_addrl = get_byte(f)) < 0) return -1;	    
	addrext = byte_addrh * 256 + byte_addrl;    
	sum = byte_addrh + byte_addrl;
	byte_count -= 3;
    }
    if(type == 2)  *offset += addrext << 4;
    if(type == 4)  *offset += addrext << 16;       
    if(addr + *offset >= size) return -3;
    // dane    
    for( i = 0; !feof(f) && byte_count;  byte_count--, i++ ){
	if((data = get_byte(f)) < 0) return -1;	    
	buffer[addr + *offset + i] = (char)data;	
	sum += data;
    }
    sum = ~sum + 1; // sum = -sum   U2
    if((data = get_byte(f)) < 0) return -1; // control sum
    if( data != sum) return -4;
    return 0;
}

int file_load_hex(FILE *f, int size, char *buffer)
{
    int offset = 0, err=0;
    while( (err = file_load_hex_line(f, size, buffer, &offset)) == 0);
    return err;
}

int file_load_srec_line(FILE *f, int size, char *buffer, int *offset)
{
    int data;
    unsigned int byte_count, addr, type, i, addrpole, x;
    unsigned char sum = 0;
    char tmp[520];

    while(( (fgetc(f)) != 'S') && !feof(f)); // start line of hex format
    if((type = fgetc(f)) == EOF) return -1;
    if((type > '9') || (type < '0')) return -1;
    type -= '0'; // ascii to digit
    addrpole = 0;
    if((type > 0) && (type < 4)) addrpole = type + 1;
    addr = 0;
    if((byte_count = get_byte(f)) < 0) return -1;
    sum = byte_count;
    data = 0; x = 0;
    for(i = 0; (i < byte_count) && ((data = get_byte(f)) != EOF); i++){
	sum += data;
	if(addrpole){
	    addrpole--;
	    addr += data << (addrpole * 8);
	    if((addr + byte_count + byte_count - i - 1) < size) continue;
	    return -3;
	}
	if( i == byte_count - 1) break;
	if(type == 0){
	    tmp[x++] = data;
	    continue;
	}	
	if((type > 0) && (type < 4))
	        buffer[ addr++ ] = data;
    }
    if(type == 0){
	tmp[x] = 0;
        printf("S0 HDR = [%s]\n", tmp);
    }
    if(data == EOF) return -1;    
    if(sum != 0xff) return -4;
    if(type > 6) return -10;
    return 0;
}

int file_load_srec(FILE *f, int size, char *buffer)
{
    int offset = 0, err=0;
    while( (err = file_load_srec_line(f, size, buffer, &offset)) == 0);
    return err;
}

int file_test_extension(FILE *f, const char *fname, const char *ext)
{
    int i;
    char *tmp = (char *)fname;
    if(!fname || (*fname == 0)) return 0;
    // test extension
    tmp += strlen(fname); // go to the end of string
    for(;(tmp >= fname) && (*tmp != '.'); tmp--); // looking for '.' from end to begin
    if( *tmp == '.') tmp++;
    for(i = 0; tmp[i]; i++) if((tmp[i] >='A') && (tmp[i] <= 'Z')) tmp[i] = tmp[i] - 'A' + 'a'; // to lower
    if(strcmp(tmp, ext)) return 1; // check extension, if not match, exit with 0
    return 2;
}

const char *file_load(geepro *gep, const char *fname, long file_offset, long buffer_offset, long bytes_count )
{
    FILE *f;
    int err = 0, x=0;

    if(!gep->chp) return "No chip memory size specified.";    
    
    if( file_offset < 0 )
	memset(gep->chp->buffer, 0xff, gep->chp->dev_size); // set buffer by 0xff

    if(!(f = fopen(fname , "r-"))) return "Open file error.";

    if( file_offset >= 0){
        err = file_load_bin(f, gep->chp->dev_size, gep->chp->buffer, file_offset, buffer_offset, bytes_count);
        x = 2;
    } else {
	if((x = file_test_extension(f, fname, "hex")) == 2) 
	    err = file_load_hex(f, gep->chp->dev_size, gep->chp->buffer);
	else
    	    if((x = file_test_extension(f, fname, "srec")) == 2) 
		err = file_load_srec(f, gep->chp->dev_size, gep->chp->buffer);
	else
    	    if((x = file_test_extension(f, fname, "s19")) == 2) 
		err = file_load_srec(f, gep->chp->dev_size, gep->chp->buffer);
	else
    	    if(x == 1) 
    		err = file_load_bin(f, gep->chp->dev_size, gep->chp->buffer, -1, -1, -1);
    }
    if(x == 0) return "Filename error."; // no fname or NULL
    if(err == -10) err = 0; // end of HEX file, normal termination

    fclose(f);    
    return file_err_msg(err);
}

/*************************************************************************************/

int file_save_bin(FILE *f, unsigned int size, char *buffer)
{
    if(fwrite(buffer, 1, size, f) != size) return -1;
    return 0;
}

char file_hex(char c)
{
    c &= 0x0f;
    return c > 9 ? c - 10 + 'A' : c + '0';
}

void file_put_byte(FILE *f, char a)
{
    fputc(file_hex(a >> 4), f);
    fputc(file_hex(a), f);
}

void file_save_srec_line(FILE *f, int size, int addr, char *buffer)
{    
    int n, i, sum, tmp;
    sum = 0;
    n = 0;
    if(size & 0xffff0000) n++;
    if(size & 0xff000000) n++;
    fputc('S', f);
    fputc('1' + n, f);    
    sum = size + n + 3;
    file_put_byte(f, sum); // 3 -> 2 bytes of addres + 1 byte count = 3
    for(i = n + 2; i; i--){
	tmp = addr >> ((i-1) * 8);
	file_put_byte(f, tmp);
	sum += tmp;
    }
    for(i = 0; i < size; i++){
	file_put_byte(f, buffer[addr + i]);
	sum += buffer[addr + i];
    }
    file_put_byte(f, ~sum & 0xff);    
    fputc('\n', f);    
}

int file_save_srec(FILE *f, int size, char *buffer)
{
    int addr = 0, sz;
    while(size){
	sz = size < 16 ? size : 16;
	file_save_srec_line(f, sz, addr, buffer);
	addr += sz;
	size -= sz;
    }
    fputs("S9030000FC\n", f);
    return 0;
}

void file_save_hex_line(FILE *f, int size, int addr, char *buffer)
{    
    int n, i, sum;
    n = 0;
    if(size & 0xffff0000) n = 2;
    if(size & 0xff000000) n = 4;
    fputc(':', f);
    file_put_byte(f, size);    		  // record size
    sum = size;
    file_put_byte(f, i = ((addr >> 8) & 0xff)); // address 16 bit msb
    sum += i;
    file_put_byte(f, i = (addr & 0xff));        // address 16 bit lsb
    sum += i;
    file_put_byte(f, n);        	  // record type
    sum += n;
    if(n){
	i = 0;
	if(n == 2) i = size >> 4;
	if(n == 4) i = size >> 16;
	file_put_byte(f, n = ((i >> 8) & 0xff)); // extended address 16 bit msb
	sum += n;
	file_put_byte(f, n = (i & 0xff));        // extended address 16 bit lsb
	sum += n;
    }

    for(i = 0; i < size; i++){
	file_put_byte(f, buffer[addr + i]);
	sum += buffer[addr + i];
    }
    file_put_byte(f, ~(sum & 0xff) + 1 );
    fputc('\n', f);    
}


int file_save_hex(FILE *f, int size, char *buffer)
{
    int addr = 0, sz;
    while(size){
	sz = size < 16 ? size : 16;
	file_save_hex_line(f, sz, addr, buffer);
	addr += sz;
	size -= sz;
    }
    fputs(":00000001FF\n", f);
    return 0;
}

const char *file_save(geepro *gep, const char *fname)
{
    FILE *f;
    int err = 0, x=0, size = gep->chp->dev_size;

    if(!fname)  return "No file name";
    if(fname[0] == 0)  return "No file name";
    if( !size ) return "Buffer size is 0 bytes.";
    for(; size && (((gep->chp->buffer[size - 1]) & 0xff) == 0xff); size--); // skraca bufor o puste 'FF'
    if(!gep->chp) return "No chip memory size specified.";    
    if(!(f = fopen(fname , "w"))) return "Open file error.";
    if((x = file_test_extension(f, fname, "hex")) == 2) 
	err = file_save_hex(f, size, gep->chp->buffer);
    else
      if((x = file_test_extension(f, fname, "srec")) == 2) 
	err = file_save_srec(f, size, gep->chp->buffer);
    else
      if((x = file_test_extension(f, fname, "s19")) == 2) 
	err = file_save_srec(f, size, gep->chp->buffer);
    else
      if(x == 1) err = file_save_bin(f, size, gep->chp->buffer);
    if(x == 0) return "Filename error."; // no fname or NULL

    fclose(f);    
    return file_err_msg(err);
}

long file_length(const char *fname)
{
    FILE *f;
    long l;
    
    f = fopen( fname , "r");
    if( f == NULL ) return -1;
    fseek(f, 0L, SEEK_END);
    l = ftell( f );
    fclose(f);
    
    return l;
}

//------------------------------------------------------------------------------------------

const char *file_get_time(geepro *gep, long long *time, const char *fname)
{
    struct stat st;

    if( !fname ) return 0;
    if( fname[0] == 0) return 0;
    if(stat(fname, &st)){
	perror(fname);
        return "Cannot stat on file";
    }
    *time = st.st_mtim.tv_sec * 100 + (st.st_mtim.tv_sec / 1000000);
    return 0;
}

boolean file_regex_match(const char *str, regex_t *rgx, char *error)
{
    char tmp[100];
    int  ret;

    error[0] = 0;
    ret = regexec(rgx, str, 0, NULL, 0);
    switch(ret){
	case 0: return true;
	case REG_NOMATCH: return false;
	default: regerror(ret, rgx, tmp, 100);
		 sprintf( error, "Regex match failed: %s\n", tmp);
    }
    return false;
}

boolean file_ls(const char *path, const char *regex, char *error, file_ls_callback fcb, void *arg)
{
    regex_t rgx;
    DIR     *dp;
    struct dirent *ep;
    boolean tmp;

    if( regcomp(&rgx, regex, REG_EXTENDED | REG_NOSUB) ){
	sprintf( error, "regcomp() failed: Could not compile regex. { files.c -> file_regex_match() }\n");
	return false;
    };

    if((dp = opendir( path )) == NULL){
	sprintf(error, "Could not open the directory");
	return false;
    };

    while(( ep = readdir(dp) )){
	tmp = file_regex_match( ep->d_name, &rgx, error );
	if( tmp ){ 
	    if( !fcb( ep->d_name, error, arg ) ) return false;
	}
	if( !tmp && error[0] ) return false;
    }

    closedir( dp );
    regfree( &rgx );
    return true;
}


